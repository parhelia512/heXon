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

#include "seeker.h"
#include "hitfx.h"
#include "tilemaster.h"
#include "player.h"

Seeker::Seeker(Context *context, MasterControl *masterControl):
    SceneObject(context, masterControl),
    age_{0.0f},
    lifeTime_{7.5f},
    damage_{2.3f}
{
    rootNode_->SetName("Seeker");

    rigidBody_ = rootNode_->CreateComponent<RigidBody>();
    rigidBody_->SetMass(2.3f);
    rigidBody_->SetLinearDamping(0.23f);
    rigidBody_->SetTrigger(true);

    CollisionShape* trigger = rootNode_->CreateComponent<CollisionShape>();
    trigger->SetSphere(1.0f);

    ParticleEmitter* particleEmitter = rootNode_->CreateComponent<ParticleEmitter>();
    particleEmitter->SetEffect(masterControl_->cache_->GetResource<ParticleEffect>("Resources/Particles/Seeker.xml"));

    AddTail();

    Light* light = rootNode_->CreateComponent<Light>();
    light->SetRange(6.66f);
    light->SetBrightness(2.3f);
    light->SetColor(Color(1.0f, 1.0f, 1.0f));

    sample_ = masterControl_->cache_->GetResource<Sound>("Resources/Samples/Seeker.ogg");
    sample_->SetLooped(false);
}

void Seeker::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    if (!IsEnabled()) return;

    float timeStep = eventData[SceneUpdate::P_TIMESTEP].GetFloat();

    age_ += timeStep;
    if (age_ > lifeTime_ && rootNode_->IsEnabled()) {
        masterControl_->spawnMaster_->SpawnHitFX(GetPosition(), false);
        Disable();
    }

    target_ = LucKey::Distance(rootNode_->GetPosition(), masterControl_->player1_->rootNode_->GetPosition()) <
            LucKey::Distance(rootNode_->GetPosition(), masterControl_->player2_->rootNode_->GetPosition())
            ? masterControl_->player1_->rootNode_
            : masterControl_->player2_->rootNode_;
    rigidBody_->ApplyForce((target_->GetPosition() - rootNode_->GetPosition()).Normalized() * timeStep * 666.0f);
}

void Seeker::HandleTriggerStart(StringHash eventType, VariantMap &eventData)
{
    PODVector<RigidBody*> collidingBodies{};
    rigidBody_->GetCollidingBodies(collidingBodies);

    for (int i = 0; i < collidingBodies.Size(); i++) {
        RigidBody* collider = collidingBodies[i];
        if (collider->GetNode()->GetNameHash() == N_PLAYER) {
            masterControl_->player1_->Hit(2.3f, false);
            masterControl_->spawnMaster_->SpawnHitFX(rootNode_->GetPosition(), true);
            collider->ApplyImpulse(rigidBody_->GetLinearVelocity()*0.5f);
            Disable();
        }
        else if (collider->GetNode()->GetNameHash() == N_CHAOMINE){
            masterControl_->spawnMaster_->chaoMines_[collider->GetNode()->GetID()]->Hit(damage_, 0);
        }
        else if (collider->GetNode()->GetNameHash() == N_SEEKER){
            masterControl_->spawnMaster_->SpawnHitFX(rootNode_->GetPosition(), false);
            Disable();
        }
    }
}

void Seeker::Set(Vector3 position)
{
    age_= 0.0f;
    SceneObject::Set(position);
    rigidBody_->ResetForces();
    rigidBody_->SetLinearVelocity(Vector3::ZERO);
    masterControl_->tileMaster_->AddToAffectors(WeakPtr<Node>(rootNode_), WeakPtr<RigidBody>(rigidBody_));
    AddTail();
    PlaySample(sample_, 0.666f);

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Seeker, HandleSceneUpdate));
    SubscribeToEvent(rootNode_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Seeker, HandleTriggerStart));
}
void Seeker::Disable()
{
    RemoveTail();
    SceneObject::Disable();
}

void Seeker::AddTail()
{
    tailGen_ = rootNode_->CreateComponent<TailGenerator>();
    tailGen_->SetDrawHorizontal(true);
    tailGen_->SetDrawVertical(false);
    tailGen_->SetTailLength(0.23f);
    tailGen_->SetNumTails(9);
    tailGen_->SetWidthScale(0.666f);
    tailGen_->SetColorForHead(Color(0.5f, 0.23f, 0.666f));
    tailGen_->SetColorForTip(Color(0.0f, 0.1f, 0.23f));
}
void Seeker::RemoveTail()
{
    tailGen_->Remove();
}
