/* heXon
// Copyright (C) 2015 LucKey Productions (luckeyproductions.nl)
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

#include "tilemaster.h"
#include "pickup.h"
#include "spawnmaster.h"
#include "player.h"

Pickup::Pickup(Context *context, MasterControl *masterControl):
    SceneObject(context, masterControl),
    sinceLastPickup_{0.0f},
    chaoInterval_{Random(23.0f, 100.0f)}
{
    rootNode_->SetName("Pickup");
    graphicsNode_ = rootNode_->CreateChild("Graphics");

    model_ = graphicsNode_->CreateComponent<StaticModel>();
    rootNode_->SetScale(0.8f);
    rootNode_->SetEnabledRecursive(false);

    rigidBody_ = rootNode_->CreateComponent<RigidBody>();
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

    CollisionShape* collisionShape = rootNode_->CreateComponent<CollisionShape>();
    collisionShape->SetSphere(1.5f);

    masterControl_->tileMaster_->AddToAffectors(WeakPtr<Node>(rootNode_), WeakPtr<RigidBody>(rigidBody_));

    Node* triggerNode = rootNode_->CreateChild("PickupTrigger");
    triggerBody_ = triggerNode->CreateComponent<RigidBody>();
    triggerBody_->SetTrigger(true);
    triggerBody_->SetMass(0.0f);
    triggerBody_->SetLinearFactor(Vector3::ZERO);
    CollisionShape* triggerShape = triggerNode->CreateComponent<CollisionShape>();
    triggerShape->SetSphere(2.5f);

    particleEmitter_ = graphicsNode_->CreateComponent<ParticleEmitter>();

    particleEmitter_->SetEffect(masterControl_->cache_->GetTempResource<ParticleEffect>("Resources/Particles/Shine.xml"));

    sample_ = masterControl_->cache_->GetResource<Sound>("Resources/Samples/Pickup.ogg");
    sample_->SetLooped(false);
    soundSource_ = rootNode_->CreateComponent<SoundSource>();
    soundSource_->SetGain(0.6f);
    soundSource_->SetSoundType(SOUND_EFFECT);

    SubscribeToEvent(triggerNode, E_NODECOLLISIONSTART, HANDLER(Pickup, HandleTriggerStart));
    SubscribeToEvent(E_SCENEUPDATE, HANDLER(Pickup, HandleSceneUpdate));
}

void Pickup::HandleTriggerStart(StringHash eventType, VariantMap &eventData)
{
    using namespace NodeCollisionStart;

    PODVector<RigidBody*> collidingBodies;
    triggerBody_->GetCollidingBodies(collidingBodies);

    for (int i = 0; i < collidingBodies.Size(); i++) {
        RigidBody* collider = collidingBodies[i];
        if (collider->GetNode()->GetNameHash() == N_PLAYER) {
            soundSource_->Play(sample_);
            masterControl_->player_->Pickup(pickupType_);
            masterControl_->spawnMaster_->SpawnHitFX(GetPosition(), false);
            switch (pickupType_){
            case PT_MULTIX: case PT_CHAOBALL: Deactivate(); break;
            case PT_APPLE: case PT_HEART: Respawn(); break;
            default: break;
            }
        }
    }
}

void Pickup::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    //Move trigger along
    triggerBody_->SetPosition(rootNode_->GetPosition());
    //Emerge
    if (!IsEmerged()) rootNode_->Translate(Vector3::UP * timeStep * (0.23f - rootNode_->GetPosition().y_), TS_WORLD);

    float xSpin = 0.0f;
    float ySpin = 100.0f;
    float zSpin = 0.0f;
    float frequency = 2.5f;
    float shift = 0.5f;

    switch (pickupType_){
    case PT_APPLE: shift = 0.23f; break;
    case PT_HEART: break;
    case PT_MULTIX:
        xSpin = 64.0f; zSpin = 10.0f; frequency = 5.0f;
        if (IsEmerged() && masterControl_->GetGameState() == GS_PLAY)
            rigidBody_->ApplyForce(2.0f*masterControl_->player_->GetPosition()); break;
    case PT_CHAOBALL: {
        xSpin = 23.0f; zSpin = 42.0f; frequency = 5.0f; shift = 0.23f;
        if (!rootNode_->IsEnabled() && masterControl_->GetGameState() == GS_PLAY) {
            if (sinceLastPickup_ > chaoInterval_) Respawn();
            else sinceLastPickup_ += timeStep;
        }
        else if (IsEmerged() && masterControl_->GetGameState() == GS_PLAY)
            rigidBody_->ApplyForce(-3.0f*masterControl_->player_->GetPosition()); break;
    } break;
    default: break;
    }
    //Spin
    rootNode_->Rotate(Quaternion(xSpin * timeStep, ySpin * timeStep, zSpin * timeStep));
    //Float
    float floatFactor = 0.5f - Min(0.5f, 0.5f * Abs(rootNode_->GetPosition().y_));
    graphicsNode_->SetPosition(Vector3::UP * masterControl_->Sine(frequency, -floatFactor, floatFactor, shift));
}

void Pickup::Respawn(bool restart)
{
    rigidBody_->SetLinearVelocity(Vector3::ZERO);
    rigidBody_->ResetForces();

    Set(restart ? initialPosition_
                : masterControl_->spawnMaster_->SpawnPoint());
    masterControl_->tileMaster_->AddToAffectors(WeakPtr<Node>(rootNode_), WeakPtr<RigidBody>(rigidBody_));
}
void Pickup::Deactivate()
{
    sinceLastPickup_ = 0.0f; chaoInterval_ = Random(23.0f, 100.0f);

    soundSource_->Stop();
    SceneObject::Disable();
}
