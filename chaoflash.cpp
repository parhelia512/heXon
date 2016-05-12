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
#include "spawnmaster.h"

ChaoFlash::ChaoFlash(int playerID):
    SceneObject(),
    playerID_{playerID},
    age_{0.0f}
{
    rootNode_->SetName("ChaoFlash");
    rootNode_->SetScale(7.0f);
    chaoModel_ = rootNode_->CreateComponent<StaticModel>();
    chaoModel_->SetModel(MC->cache_->GetResource<Model>("Models/ChaoFlash.mdl"));
    chaoMaterial_ = MC->cache_->GetResource<Material>("Materials/ChaoFlash.xml");
    chaoModel_->SetMaterial(chaoMaterial_);

    Node* sunNode{MC->world.scene->CreateChild("SunDisk")};
    sunNode->SetTransform(Vector3::UP, Quaternion::IDENTITY, 42.0f);
    StaticModel* sunPlane{sunNode->CreateComponent<StaticModel>()};
    sunPlane->SetModel(MC->cache_->GetResource<Model>("Models/Plane.mdl"));;
    sunMaterial_ = MC->cache_->GetResource<Material>("Materials/SunDisc.xml");
    sunPlane->SetMaterial(sunMaterial_);

    rootNode_->SetEnabled(false);
    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(ChaoFlash, HandleSceneUpdate));

    sample_ = MC->cache_->GetResource<Sound>("Samples/Chaos.ogg");

    rigidBody_ = rootNode_->CreateComponent<RigidBody>();
    rigidBody_->SetMass(5.0f);
}

void ChaoFlash::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    if (!IsPlayingSound()) Disable();

    if (!IsEnabled()) return;

    float timeStep{eventData[Update::P_TIMESTEP].GetFloat()};
    age_ += timeStep;

    Color chaoColor{chaoMaterial_->GetShaderParameter("MatDiffColor").GetColor()};
    rigidBody_->SetMass(chaoColor.a_);
    Color newDiffColor{chaoColor.r_ * Random(0.1f , 1.23f),
                       chaoColor.g_ * Random(0.23f, 0.9f),
                       chaoColor.b_ * Random(0.16f, 0.5f),
                       chaoColor.a_ * Random(0.42f, 0.9f)};
    chaoMaterial_->SetShaderParameter("MatDiffColor", chaoColor.Lerp(newDiffColor, Clamp(23.0f*timeStep, 0.0f, 1.0f)));
    Color newSpecColor{Random(0.3f, 1.5f),
                       Random(0.5f, 1.8f),
                       Random(0.4f, 1.4f),
                       Random(4.0f, 64.0f)};
    chaoMaterial_->SetShaderParameter("MatSpecColor", newSpecColor);
    rootNode_->SetRotation(Quaternion(Random(360.0f), Random(360.0f), Random(360.0f)));

    if (age_ > 0.16f)
        sunMaterial_->SetShaderParameter("MatDiffColor", Color(Random(1.0f), Random(1.0f), Random(1.0f), Max(0.23f - age_, 0.0f)));
}

void ChaoFlash::Set(const Vector3 position)
{
    SceneObject::Set(position);
    age_ = 0.0f;
    PlaySample(sample_, 0.69f);
    PODVector<RigidBody* > hitResults{};
    float radius{7.666f};
    rootNode_->SetEnabled(true);
    chaoMaterial_->SetShaderParameter("MatDiffColor", Color(0.1f, 0.5f, 0.2f, 0.5f));
    if (MC->PhysicsSphereCast(hitResults,rootNode_->GetPosition(), radius, M_MAX_UNSIGNED)){
        for (int i = 0; i < hitResults.Size(); i++){
            unsigned hitID = hitResults[i]->GetNode()->GetID();
            if(MC->spawnMaster_->spires_.Contains(hitID)){
                WeakPtr<Spire> spire = MC->spawnMaster_->spires_[hitID];
                spire->Disable();
                MC->spawnMaster_->SpawnChaoMine(spire->GetPosition(), playerID_);
                MC->GetPlayer(playerID_)->AddScore(Random(42, 100));
            }
            else if(MC->spawnMaster_->razors_.Contains(hitID)){
                WeakPtr<Razor> razor = MC->spawnMaster_->razors_[hitID];
                razor->Disable();
                MC->spawnMaster_->SpawnChaoMine(razor->GetPosition(), playerID_);
                MC->GetPlayer(playerID_)->AddScore(Random(23, 42));
            }
            else if(MC->spawnMaster_->seekers_.Contains(hitID)){
                WeakPtr<Seeker> seeker = MC->spawnMaster_->seekers_[hitID];
                seeker->Disable();
                MC->GetPlayer(playerID_)->AddScore(Random(5, 23));
            }
        }
    }
    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(ChaoFlash, HandleSceneUpdate));
}

void ChaoFlash::Disable()
{
    UnsubscribeFromEvent(E_SCENEUPDATE);
    SceneObject::Disable();
}
