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
#include "inputmaster.h"
#include "player.h"
#include "ship.h"
#include "hitfx.h"

Pickup::Pickup(Context* context) : SceneObject(context)
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
    rigidBody_->SetCollisionLayerAndMask(4, M_MAX_UNSIGNED - 1);

    CollisionShape* collisionShape = node_->CreateComponent<CollisionShape>();
    collisionShape->SetSphere(2.3f);

    MC->arena_->AddToAffectors(WeakPtr<Node>(node_), WeakPtr<RigidBody>(rigidBody_));

    triggerNode_ = node_->CreateChild("PickupTrigger");
    triggerBody_ = triggerNode_->CreateComponent<RigidBody>();
    triggerBody_->SetTrigger(true);
    triggerBody_->SetKinematic(true);
    triggerBody_->SetCollisionLayerAndMask(1, 1);

    CollisionShape* triggerShape = triggerNode_->CreateComponent<CollisionShape>();
    triggerShape->SetSphere(2.5f);

    particleEmitter_ = graphicsNode_->CreateComponent<ParticleEmitter>();

    particleEmitter_->SetEffect(CACHE->GetTempResource<ParticleEffect>("Particles/Shine.xml"));

    SubscribeToEvent(triggerNode_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Pickup, HandleTriggerStart));
}

void Pickup::Update(float timeStep)
{
    //Emerge
    Emerge(timeStep);
    if (!IsEmerged()) {
        rigidBody_->ResetForces();
        rigidBody_->SetLinearVelocity(Vector3::ZERO);
    }
}

void Pickup::HandleTriggerStart(StringHash eventType, VariantMap &eventData)
{ (void)eventType;

    PODVector<RigidBody*> collidingBodies;
    triggerBody_->GetCollidingBodies(collidingBodies);
    Node* otherNode = static_cast<Node*>(eventData[NodeCollisionStart::P_OTHERNODE].GetPtr());

    if (otherNode->HasComponent<Ship>()) {

        Ship* ship{ otherNode->GetComponent<Ship>() };
        ship->Pickup(pickupType_);
        int hitPlayer{ ship->GetPlayer()->GetPlayerId() };
        GetSubsystem<SpawnMaster>()->Create<HitFX>()
                ->Set(GetPosition(), ship->GetColorSet(), false);
        switch (pickupType_){
        case PT_CHAOBALL: GetSubsystem<SpawnMaster>()->ChaoPickup(); Deactivate(); break;
        case PT_APPLE: case PT_HEART: Respawn(); break;
        default: break;
        }
    }
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

    Set(restart ? initialPosition_ : GetSubsystem<SpawnMaster>()->SpawnPoint());
    MC->arena_->AddToAffectors(WeakPtr<Node>(node_), WeakPtr<RigidBody>(rigidBody_));
}
void Pickup::Deactivate()
{
    SceneObject::Disable();
}
