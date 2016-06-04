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

#include "chaomine.h"

#include "spawnmaster.h"
#include "player.h"

ChaoMine::ChaoMine() : Enemy(),
    playerID_{0}
{
    rootNode_->SetName("ChaoMine");
    big_ = false;

    rigidBody_->SetMass(0.5f);
    //Overrides Enemy values
    meleeDamage_ = 0.1f;
    initialHealth_ = 0.05f;
    worth_ = 1;

    countDown_ = Random(1.0f, 5.0f);
    innerNode_ = rootNode_->CreateChild();
    innerModel_ = innerNode_->CreateComponent<StaticModel>();
    innerModel_->SetModel(MC->cache_->GetResource<Model>("Models/MineInner.mdl"));
    innerModel_->SetMaterial(0, MC->resources.materials.ship1Primary);

    outerNode_ = rootNode_->CreateChild();
    outerModel_ = outerNode_->CreateComponent<StaticModel>();
    outerModel_->SetModel(MC->cache_->GetResource<Model>("Models/MineOuter.mdl"));
    outerModel_->SetMaterial(0, MC->resources.materials.ship1Secondary);
    outerModel_->SetMaterial(1, MC->resources.materials.ship1Primary);

}

void ChaoMine::Set(const Vector3 position, int playerID)
{
    playerID_ = playerID;

    if (playerID_ == 1) {
        innerModel_->SetMaterial(0, MC->resources.materials.ship1Primary);
        outerModel_->SetMaterial(0, MC->resources.materials.ship1Secondary);
        outerModel_->SetMaterial(1, MC->resources.materials.ship1Primary);
    } else if (playerID_ == 2) {
        innerModel_->SetMaterial(0, MC->resources.materials.ship2Primary);
        outerModel_->SetMaterial(0, MC->resources.materials.ship2Secondary);
        outerModel_->SetMaterial(1, MC->resources.materials.ship2Primary);
    }


    Enemy::Set(position);
    SubscribeToEvent(E_SCENEPOSTUPDATE, URHO3D_HANDLER(ChaoMine, HandleMineUpdate));
}

void ChaoMine::HandleMineUpdate(StringHash eventType, VariantMap &eventData)
{
    float timeStep{eventData[ScenePostUpdate::P_TIMESTEP].GetFloat()};

    //Spin
    innerNode_->Rotate(Quaternion(50.0f*timeStep, 80.0f*timeStep, 92.0f*timeStep));
    outerNode_->Rotate(Quaternion(-60.0f*timeStep,-101.0f*timeStep, -95.0f*timeStep));
}

void ChaoMine::CheckHealth()
{
    if (rootNode_->IsEnabled() && health_ <= 0 || panicTime_ > 23.0f) {
        MC->spawnMaster_->SpawnChaoZap(GetPosition(), playerID_);
        Disable();
    }
}

void ChaoMine::HandleCollision(StringHash eventType, VariantMap &eventData)
{
    PODVector<RigidBody*> collidingBodies{};
    rigidBody_->GetCollidingBodies(collidingBodies);

    for (RigidBody* b : collidingBodies) {
        StringHash colliderNodeNameHash = b->GetNode()->GetNameHash();
        if (    colliderNodeNameHash == N_RAZOR ||
                colliderNodeNameHash == N_SPIRE   ) {
            SetHealth(0.0f);
        }
    }
}
