/* heXon
// Copyright (C) 2016 LucKey Productions (luckeyproductions.nl)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "chaoflash.h"

#include "player.h"
#include "apple.h"
#include "heart.h"
#include "ship.h"
#include "chaomine.h"
#include "seeker.h"
#include "spawnmaster.h"

void ChaoFlash::RegisterObject(Context *context)
{
    context->RegisterFactory<ChaoFlash>();
}

ChaoFlash::ChaoFlash(Context* context):
    SceneObject(context),
    age_{0.0f}
{
}

void ChaoFlash::OnNodeSet(Node *node)
{
    SceneObject::OnNodeSet(node);

    node_->SetName("ChaoFlash");
    node_->SetScale(7.0f);
    chaoModel_ = node_->CreateComponent<StaticModel>();
    chaoModel_->SetModel(MC->GetModel("ChaoFlash"));
    chaoMaterial_ = MC->GetMaterial("ChaoFlash");
    chaoModel_->SetMaterial(chaoMaterial_);

    Node* sunNode{MC->scene_->CreateChild("SunDisk")};
    sunNode->SetTransform(Vector3::UP, Quaternion::IDENTITY, 42.0f);
    StaticModel* sunPlane{sunNode->CreateComponent<StaticModel>()};
    sunPlane->SetModel(MC->GetModel("Plane"));;
    sunMaterial_ = MC->GetMaterial("SunDisc");
    sunPlane->SetMaterial(sunMaterial_);

    node_->SetEnabled(false);

    rigidBody_ = node_->CreateComponent<RigidBody>();
    rigidBody_->SetMass(5.0f);

}

void ChaoFlash::Update(float timeStep)
{
    if (!IsEnabled()) return;

    age_ += timeStep;

    Color chaoColor{chaoMaterial_->GetShaderParameter("MatDiffColor").GetColor()};
    rigidBody_->SetMass(chaoColor.a_);
    Color newDiffColor{chaoColor.r_ * Random(0.1f , 1.23f),
                       chaoColor.g_ * Random(0.23f, 0.9f),
                       chaoColor.b_ * Random(0.16f, 0.5f),
                       chaoColor.a_ * Random(0.42f, 0.9f)};
    chaoMaterial_->SetShaderParameter("MatDiffColor", chaoColor.Lerp(newDiffColor, Clamp(23.0f * timeStep, 0.0f, 1.0f)));
    Color newSpecColor{Random(0.3f, 1.5f),
                       Random(0.5f, 1.8f),
                       Random(0.4f, 1.4f),
                       Random(4.0f, 64.0f)};
    chaoMaterial_->SetShaderParameter("MatSpecColor", newSpecColor);
    node_->SetRotation(Quaternion(Random(360.0f), Random(360.0f), Random(360.0f)));

    if (age_ > 0.13f)
        sunMaterial_->SetShaderParameter("MatDiffColor", Color(Random(1.0f),
                                                               Random(1.0f),
                                                               Random(1.0f),
                                                               Max(0.23f - age_, 0.0f)));

    if (age_ > 0.42f)
        Disable();
}

void ChaoFlash::Set(const Vector3 position, int colorSet)
{
    Player* owner{ MC->GetPlayerByColorSet(colorSet) };
    Vector<Ship*> ships{};
    bool caughtApple{ false };
    bool caughtHeart{ false };

    age_ = 0.0f;
    SceneObject::Set(position);

    PODVector<RigidBody* > hitResults{};
    float radius{7.666f};
    chaoMaterial_->SetShaderParameter("MatDiffColor", Color(0.1f, 0.5f, 0.2f, 0.5f));

    if (MC->PhysicsSphereCast(hitResults, node_->GetPosition(), radius, M_MAX_UNSIGNED)){
        for (RigidBody* hitResult : hitResults){
            Node* hitNode{ hitResult->GetNode() };
            if (hitNode->GetName() == "PickupTrigger")
                hitNode = hitNode->GetParent();

            if (hitNode->HasComponent<Ship>()) {
                ships.Push(hitNode->GetComponent<Ship>());

            } else if (hitNode->HasComponent<Apple>()) {

                caughtApple = true;
                hitNode->GetComponent<Apple>()->Respawn();

            } else if (hitNode->HasComponent<Heart>()) {
                caughtHeart = true;
                hitNode->GetComponent<Heart>()->Respawn();
            //Destroy Seekers
            } else if (hitNode->HasComponent<Seeker>()){
                owner->AddScore(Random(2, 3));
                hitNode->GetComponent<Seeker>()->Disable();
            //Turn enemies into mines
            } else for (Component* c : hitNode->GetComponents()) {
                if (c->IsInstanceOf<Enemy>() && !c->IsInstanceOf<ChaoMine>()){

                    Enemy* e{ static_cast<Enemy*>(c) };
                    ChaoMine* chaoMine{ GetSubsystem<SpawnMaster>()->Create<ChaoMine>() };
                    chaoMine->Set(e->GetPosition(), colorSet);
                    MC->GetPlayerByColorSet(colorSet)->AddScore(Random(2, 3) * e->GetWorth());
                    e->Disable();
                }
            }
        }
    }

    //Hand out upgrades
    for (Ship* s : ships){
        if (caughtApple)
            s->UpgradeWeapons();
        if (caughtHeart)
            s->ChargeShield();
    }

    //Swap ship positions
    if (ships.Size() > 1) {
        Vector3 firstPos{ ships[0]->GetPosition() };
        for (unsigned s{0}; s < ships.Size(); ++s){

            if (s == ships.Size() - 1) {
                ships[s]->GetNode()->SetPosition(firstPos);
            } else
                ships[s]->GetNode()->SetPosition(ships[s+1]->GetPosition());
        }
    } else if (ships.Size()) {
        ships[0]->GetNode()->SetPosition(Quaternion(Random(360.0f), Vector3::UP) * (Vector3::FORWARD * Random(5.0f)) +
                                       node_->GetPosition() * Vector3(1.0f, 0.0f, 1.0f));
    }
}

void ChaoFlash::Disable()
{
    SceneObject::Disable();
}
