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

#include "seeker.h"

#include "hitfx.h"
#include "arena.h"
#include "player.h"
#include "spawnmaster.h"

void Seeker::RegisterObject(Context *context)
{
    context->RegisterFactory<Seeker>();
}

Seeker::Seeker(Context* context):
    SceneObject(context),
    age_{0.0f},
    lifeTime_{7.5f},
    damage_{2.3f}
{

}

void Seeker::OnNodeSet(Node *node)
{
    SceneObject::OnNodeSet(node);

    node_->SetName("Seeker");
    big_ = false;

    rigidBody_ = node_->CreateComponent<RigidBody>();
    rigidBody_->SetMass(2.3f);
    rigidBody_->SetLinearDamping(0.23f);
    rigidBody_->SetTrigger(true);

    CollisionShape* trigger = node_->CreateComponent<CollisionShape>();
    trigger->SetSphere(1.0f);

    ParticleEmitter* particleEmitter = node_->CreateComponent<ParticleEmitter>();
    particleEmitter->SetEffect(CACHE->GetResource<ParticleEffect>("Particles/Seeker.xml"));

    AddTail();

    Light* light{ node_->CreateComponent<Light>() };
    light->SetRange(6.66f);
    light->SetBrightness(2.3f);
    light->SetColor(Color(1.0f, 1.0f, 1.0f));

    sample_ = CACHE->GetResource<Sound>("Samples/Seeker.ogg");
    sample_->SetLooped(false);
}

void Seeker::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    if (!IsEnabled()) return;

    float timeStep{ eventData[SceneUpdate::P_TIMESTEP].GetFloat() };

    age_ += timeStep;
    if (age_ > lifeTime_ && node_->IsEnabled()) {
        GetSubsystem<SpawnMaster>()->SpawnHitFX(GetPosition(), 0, false);
        Disable();
    }

    Vector3 targetPosition = Vector3::ZERO;
    Player* player1 = MC->GetPlayer(1);
    Player* player2 = MC->GetPlayer(2);
    if (player1->IsActive() && player2->IsActive())
        targetPosition =
                LucKey::Distance(node_->GetPosition(), player1->GetPosition()) <
                LucKey::Distance(node_->GetPosition(), player2->GetPosition())
                ? player1->GetPosition()
                : player2->GetPosition();
    else if (player1->IsActive()) targetPosition = player1->GetPosition();
    else if (player2->IsActive()) targetPosition = player2->GetPosition();
    rigidBody_->ApplyForce((targetPosition - node_->GetPosition()).Normalized() * timeStep * 666.0f);
}

void Seeker::HandleTriggerStart(StringHash eventType, VariantMap &eventData)
{
    PODVector<RigidBody*> collidingBodies{};
    rigidBody_->GetCollidingBodies(collidingBodies);

    for (int i = 0; i < collidingBodies.Size(); i++) {
        RigidBody* collider = collidingBodies[i];
        if (collider->GetNode()->GetNameHash() == N_PLAYER) {
            Player* hitPlayer = MC->players_[collider->GetNode()->GetID()];

            hitPlayer->Hit(2.3f, false);
            GetSubsystem<SpawnMaster>()->SpawnHitFX(node_->GetPosition(), 0, false);
            collider->ApplyImpulse(rigidBody_->GetLinearVelocity()*0.5f);
            Disable();
        }
        else if (collider->GetNode()->GetNameHash() == N_CHAOMINE){
            GetSubsystem<SpawnMaster>()->chaoMines_[collider->GetNode()->GetID()]->Hit(damage_, 0);
        }
        else if (collider->GetNode()->GetNameHash() == N_SEEKER){
            GetSubsystem<SpawnMaster>()->SpawnHitFX(node_->GetPosition(), false);
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
    MC->arena_->AddToAffectors(WeakPtr<Node>(node_), WeakPtr<RigidBody>(rigidBody_));
    AddTail();
    PlaySample(sample_, 0.666f);

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Seeker, HandleSceneUpdate));
    SubscribeToEvent(node_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Seeker, HandleTriggerStart));
}
void Seeker::Disable()
{
    RemoveTail();
    SceneObject::Disable();
}

void Seeker::AddTail()
{
    tailGen_ = node_->CreateComponent<RibbonTrail>();
    tailGen_->SetStartScale(0.666f);
    tailGen_->SetEndScale(0.0f);
    tailGen_->SetLifetime(0.23f);
    tailGen_->SetVertexDistance(0.5f);
    tailGen_->SetTailColumn(3);
    tailGen_->SetStartColor(Color(0.5f, 0.23f, 0.666f));
    tailGen_->SetEndColor(Color(0.0f, 0.1f, 0.23f));
}
void Seeker::RemoveTail()
{
    tailGen_->Remove();
}
