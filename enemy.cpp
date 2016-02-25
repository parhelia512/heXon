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
    centerModel_->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Core.mdl"));
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
    collider->SetPosition(Vector3::UP * 0.23f);

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

void Enemy::Set(const Vector3 position)
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
    SubscribeToEvent(rootNode_, E_NODECOLLISION, URHO3D_HANDLER(Enemy, HandleCollision));
}

// Takes care of dealing damage and keeps track of who deserves how many points.
void Enemy::Hit(const float damage, const int playerID) {
    if (firstHitBy_ == 0) firstHitBy_ = playerID;
    else if (firstHitBy_ != playerID && playerID != 0) bonus_ = false;

    lastHitBy_ = playerID;

    SetHealth(health_ - damage);
}

void Enemy::SetHealth(const float health)
{
    health_ = health;
    panic_ = (initialHealth_ - health_) / initialHealth_;
    panic_ < 0.0f ? panic_ = 0.0f : panic_ = panic_;
    particleEffect_->SetMinEmissionRate(7.0f+23.0f*panic_);

    CheckHealth();
}

void Enemy::CheckHealth()
{
    //Die
    if (rootNode_->IsEnabled() && health_ <= 0.0f) {
        if (lastHitBy_ != 0) masterControl_->GetPlayer(lastHitBy_)->AddScore(bonus_ ? worth_ : 2 * worth_ / 3);

        masterControl_->spawnMaster_->SpawnExplosion(rootNode_->GetPosition(),
                                                     Color(color_.r_*color_.r_, color_.g_*color_.g_, color_.b_*color_.b_),
                                                     0.5f*rigidBody_->GetMass(),
                                                     lastHitBy_);
        Disable();
    }
}

void Enemy::Disable()
{
    SceneObject::Disable();
}

Color Enemy::GetGlowColor() const
{
    float factor = (Sin(200.0f*(masterControl_->world.scene->GetElapsedTime()+panicTime_))*(0.25f+panic_*0.25f)+(panic_*0.5f));
    factor *= factor * 2.0f;
    return color_*factor;
}

void Enemy::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    float time = masterControl_->world.scene->GetElapsedTime() + rootNode_->GetID() * 0.023f;
    float timeStep = eventData[SceneUpdate::P_TIMESTEP].GetFloat();
    panicTime_ += 3.0f * panic_ * timeStep;
    sinceLastWhack_ += timeStep;

    Emerge(timeStep);

    //Animate core
    centerModel_->GetMaterial()->SetShaderParameter("VOffset",
                Vector4(0.0f, LucKey::Cycle(time * 3.0f, 0.0f, 1.0f), 0.0f, 0.0f));
    centerNode_->Rotate(Quaternion((1.0f + panic_) * timeStep * 333.0f, Vector3::UP));
}

void Enemy::HandleCollision(StringHash eventType, VariantMap &eventData)
{
    PODVector<RigidBody*> collidingBodies;
    rigidBody_->GetCollidingBodies(collidingBodies);

    if (sinceLastWhack_ > whackInterval_){
        for (unsigned b = 0; b < collidingBodies.Size(); ++b) {
            RigidBody* collider = collidingBodies[b];
            StringHash otherNameHash = collider->GetNode()->GetNameHash();
            if (otherNameHash == N_PLAYER) {
                PlaySample(samples_[Random((int)samples_.Size())], 0.16f);

                Player* hitPlayer = masterControl_->players_[collider->GetNode()->GetID()];

                hitPlayer->Hit(meleeDamage_ + meleeDamage_*panic_);
                masterControl_->spawnMaster_->SpawnHitFX(
                            (hitPlayer->GetPosition() + GetPosition()) * 0.5f, 0, false);
            }
            // Vector3 hitPos = eventData[NodeCollision::P_CONTACTS].GetBuffer().At(0);
            sinceLastWhack_ = 0.0f;
        }
    }
}

