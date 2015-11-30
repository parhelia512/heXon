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

#include "enemy.h"
#include "player.h"

Enemy::Enemy(Context *context, MasterControl *masterControl):
    SceneObject(context, masterControl),
    initialHealth_{1.0f},
    whackInterval_{0.5f},
    sinceLastWhack_{0.0f},
    meleeDamage_{0.44f}
{
    rootNode_->SetName("Enemy");

    health_ = initialHealth_;

    //Generate random color
    int randomizer = Random(6);
    color_ = Color(0.5f + (randomizer * 0.075f), 0.9f - (randomizer * 0.075f), 0.5f+Max(randomizer-3.0f, 3.0f)/6.0f, 1.0f);

    centerNode_ = rootNode_->CreateChild("SmokeTrail");
    particleEmitter_ = centerNode_->CreateComponent<ParticleEmitter>();
    particleEffect_ = masterControl_->cache_->GetTempResource<ParticleEffect>("Resources/Particles/Enemy.xml");
    Vector<ColorFrame> colorFrames;
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.0f));
    colorFrames.Push(ColorFrame(Color(color_.r_*0.666f, color_.g_*0.666f, color_.b_*0.666f, 0.5f), 0.1f));
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f));
    particleEffect_->SetColorFrames(colorFrames);
    particleEmitter_->SetEffect(particleEffect_);

    centerModel_ = centerNode_->CreateComponent<StaticModel>();
    centerModel_->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/RazorCenter.mdl"));
    centerModel_->SetMaterial(masterControl_->cache_->GetTempResource<Material>("Resources/Materials/CoreGlow.xml"));
    centerModel_->GetMaterial(0)->SetShaderParameter("MatDiffColor", color_);
    centerModel_->GetMaterial(0)->SetShaderParameter("MatEmissiveColor", color_);

    rigidBody_ = rootNode_->CreateComponent<RigidBody>();
    rigidBody_->SetRestitution(0.666f);
    rigidBody_->SetLinearDamping(0.1f);
    rigidBody_->SetMass(2.0f);
    rigidBody_->SetLinearFactor(Vector3::ONE - Vector3::UP);
    rigidBody_->SetAngularFactor(Vector3::ZERO);
    CollisionShape* collider = rootNode_->CreateComponent<CollisionShape>();
    collider->SetSphere(2.0f);

    for (int s = 1; s <= 5; ++s){
        samples_.Push(SharedPtr<Sound>(masterControl_->cache_->GetResource<Sound>("Resources/Samples/Melee"+String(s)+".ogg")));
    }
    for (unsigned s = 0; s < samples_.Size(); s++){
        samples_[s]->SetLooped(false);
    }

    Node* soundNode = masterControl_->world.scene->CreateChild("SoundSource");
    soundSource_ = soundNode->CreateComponent<SoundSource>();
    soundSource_->SetGain(0.1f);
    soundSource_->SetSoundType(SOUND_EFFECT);
}

void Enemy::Set(Vector3 position)
{
    soundSource_->Stop();

    rigidBody_->SetLinearVelocity(Vector3::ZERO);
    rigidBody_->ResetForces();

    firstHitBy_ = lastHitBy_ = 0;
    bonus_ = true;
    health_ = initialHealth_;
    panic_ = 0.0f;

    particleEmitter_->RemoveAllParticles();
    SceneObject::Set(position);
    masterControl_->tileMaster_->AddToAffectors(WeakPtr<Node>(rootNode_), WeakPtr<RigidBody>(rigidBody_));
    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Enemy, HandleSceneUpdate));
    SubscribeToEvent(rootNode_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Enemy, HandleCollisionStart));
}

// Takes care of dealing damage and keeps track of who deserves how many points.
void Enemy::Hit(float damage, int ownerID) {
    if (firstHitBy_ == 0) firstHitBy_ = ownerID;
    else if (firstHitBy_ != ownerID) bonus_ = false;
    lastHitBy_ = ownerID;

    SetHealth(health_-damage);
}

void Enemy::SetHealth(float health)
{
    health_ = health;
    panic_ = (initialHealth_-health_)/initialHealth_;
    panic_ < 0.0f ? panic_ = 0.0f : panic_ = panic_;
    particleEffect_->SetMinEmissionRate(7.0f+23.0f*panic_);

    CheckHealth();
}

void Enemy::CheckHealth()
{
    //Die
    if (rootNode_->IsEnabled() && health_ <= 0.0) {
        masterControl_->player_->AddScore(bonus_ ? worth_ : 2 * worth_ / 3);
        masterControl_->spawnMaster_->SpawnExplosion(rootNode_->GetPosition(), Color(color_.r_*color_.r_, color_.g_*color_.g_, color_.b_*color_.b_), 0.5f*rigidBody_->GetMass());
        Disable();
    }
}

void Enemy::Disable()
{
    SceneObject::Disable();
}

Color Enemy::GetGlowColor()
{
    return color_*(Sin(200.0f*(masterControl_->world.scene->GetElapsedTime()+panicTime_))*(0.25f+panic_*0.25f)+(panic_*0.5f));
}

void Enemy::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    using namespace SceneUpdate;
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    panicTime_ += 3.0f * panic_ * timeStep;
    sinceLastWhack_ += timeStep;

    Emerge(timeStep);
}

void Enemy::HandleCollisionStart(StringHash eventType, VariantMap &eventData)
{
    using namespace NodeCollisionStart;

    PODVector<RigidBody*> collidingBodies;
    rigidBody_->GetCollidingBodies(collidingBodies);

    if (sinceLastWhack_ > whackInterval_){
        for (unsigned b = 0; b < collidingBodies.Size(); ++b) {
            RigidBody* collider = collidingBodies[b];
            StringHash otherNameHash = collider->GetNode()->GetNameHash();
            if (otherNameHash == N_PLAYER) {
                PlaySample(samples_[Random((int)samples_.Size())], 0.16f);
                masterControl_->player_->Hit(meleeDamage_ + meleeDamage_*panic_);
                sinceLastWhack_ = 0.0f;
            }
        }
    }
}

void Enemy::Emerge(float timeStep)
{
    if (!IsEmerged()) {
        rootNode_->Translate(Vector3::UP * timeStep * (0.25f - rootNode_->GetPosition().y_));
    }
}
