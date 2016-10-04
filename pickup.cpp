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

#include "pickup.h"

#include "arena.h"
#include "spawnmaster.h"
#include "player.h"

Pickup::Pickup(Context* context) : SceneObject(context),
    chaoInterval_{CHAOINTERVAL},
    sinceLastPickup_{0.0f}
{
}

void Pickup::OnNodeSet(Node *node)
{
    SceneObject::OnNodeSet(node);

    node_->SetName("Pickup");
    graphicsNode_ = node_->CreateChild("Graphics");

    model_ = graphicsNode_->CreateComponent<StaticModel>();
    node_->SetScale(0.8f);
    node_->SetEnabledRecursive(false);

    rigidBody_ = node_->CreateComponent<RigidBody>();
    rigidBody_->SetRestitution(0.666f);
    rigidBody_->SetMass(0.666f);
    rigidBody_->SetLinearFactor(Vector3::ONE - Vector3::UP);
    rigidBody_->SetLinearDamping(0.5f);
    rigidBody_->SetFriction(0.0f);
    rigidBody_->SetAngularFactor(Vector3::UP);
    rigidBody_->SetAngularDamping(0.0f);
    rigidBody_->ApplyTorque(Vector3::UP * 32.0f);
    rigidBody_->SetLinearRestThreshold(0.0f);
    rigidBody_->SetAngularRestThreshold(0.0f);

    CollisionShape* collisionShape = node_->CreateComponent<CollisionShape>();
    collisionShape->SetSphere(2.3f);

    MC->arena_->AddToAffectors(WeakPtr<Node>(node_), WeakPtr<RigidBody>(rigidBody_));

    triggerNode_ = MC->scene_->CreateChild("PickupTrigger");
    triggerBody_ = triggerNode_->CreateComponent<RigidBody>();
    triggerBody_->SetTrigger(true);
    triggerBody_->SetKinematic(true);
    triggerBody_->SetLinearFactor(Vector3::ZERO);
    CollisionShape* triggerShape = triggerNode_->CreateComponent<CollisionShape>();
    triggerShape->SetSphere(2.5f);

    particleEmitter_ = graphicsNode_->CreateComponent<ParticleEmitter>();

    particleEmitter_->SetEffect(CACHE->GetTempResource<ParticleEffect>("Particles/Shine.xml"));

    SubscribeToEvent(triggerNode_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Pickup, HandleTriggerStart));
    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Pickup, HandleSceneUpdate));
}

void Pickup::HandleTriggerStart(StringHash eventType, VariantMap &eventData)
{

    PODVector<RigidBody*> collidingBodies;
    triggerBody_->GetCollidingBodies(collidingBodies);

    for (int i = 0; i < collidingBodies.Size(); i++) {
        RigidBody* collider = collidingBodies[i];
        if (collider->GetNode()->GetNameHash() == N_PLAYER) {
            Player* hitPlayer = MC->players_[collider->GetNode()->GetID()];
//            hitPlayer->Pickup(pickupType_);
            GetSubsystem<SpawnMaster>()->SpawnHitFX(GetPosition(), hitPlayer->GetPlayerID(), false);
            switch (pickupType_){
            case PT_CHAOBALL: Deactivate(); break;
            case PT_APPLE: case PT_HEART: Respawn(); break;
            default: break;
            }
            return;
        }
    }
}

void Pickup::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    float timeStep = eventData[SceneUpdate::P_TIMESTEP].GetFloat();
    Player* player1 = MC->GetPlayer(1);
    Player* player2 = MC->GetPlayer(2);

    //Move trigger along
    triggerNode_->SetWorldPosition(node_->GetWorldPosition());
    //Emerge
    Emerge(timeStep);
    if (!IsEmerged()) {
        rigidBody_->ResetForces();
        rigidBody_->SetLinearVelocity(Vector3::ZERO);
    }

    float xSpin = 0.0f;
    float ySpin = 100.0f;
    float zSpin = 0.0f;
    float frequency = 2.5f;
    float shift = 0.5f;

    switch (pickupType_){
    case PT_APPLE: shift = 0.23f; break;
    case PT_HEART: break;
    case PT_CHAOBALL: {
        xSpin = 23.0f; zSpin = 42.0f; frequency = 5.0f; shift = 0.23f;
        if (!node_->IsEnabled() && MC->GetGameState() == GS_PLAY) {
            if (sinceLastPickup_ > chaoInterval_) Respawn();
            else sinceLastPickup_ += timeStep;
        }
        else if (IsEmerged() && MC->GetGameState() == GS_PLAY){
            Vector3 force{};
//            force += player1->IsAlive() * -3.0f*player1->GetPosition() - rigidBody_->GetLinearVelocity();
//            force += player2->IsAlive() * -3.0f*player2->GetPosition() - rigidBody_->GetLinearVelocity();
            rigidBody_->ApplyForce(force);
        } break;
    } break;
    default: break;
    }
    //Spin
    node_->Rotate(Quaternion(xSpin * timeStep, ySpin * timeStep, zSpin * timeStep));
    //Float like a float
    float floatFactor = 0.5f - Min(0.5f, 0.5f * Abs(node_->GetPosition().y_));
    graphicsNode_->SetPosition(Vector3::UP * MC->Sine(frequency, -floatFactor, floatFactor, shift));
}

void Pickup::Set(Vector3 position)
{
    SceneObject::Set(position);
    SubscribeToEvent(triggerNode_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Pickup, HandleTriggerStart));
}

void Pickup::Respawn(bool restart)
{
    rigidBody_->SetLinearVelocity(Vector3::ZERO);
    rigidBody_->ResetForces();

    Set(restart ? initialPosition_
                : GetSubsystem<SpawnMaster>()->SpawnPoint());
    MC->arena_->AddToAffectors(WeakPtr<Node>(node_), WeakPtr<RigidBody>(rigidBody_));
}
void Pickup::Deactivate()
{
    sinceLastPickup_ = 0.0f; chaoInterval_ = CHAOINTERVAL;

    SceneObject::Disable();
}
