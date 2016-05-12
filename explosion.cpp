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

#include "explosion.h"

#include "spawnmaster.h"

Explosion::Explosion():
    Effect(),
    playerID_{0},
    initialMass_{3.0f},
    initialBrightness_{8.0f}
{
    rootNode_->SetName("Explosion");

    rigidBody_ = rootNode_->CreateComponent<RigidBody>();
    rigidBody_->SetMass(initialMass_);
    rigidBody_->SetLinearFactor(Vector3::ONE - Vector3::UP);

    light_ = rootNode_->CreateComponent<Light>();
    light_->SetRange(13.0f);
    light_->SetBrightness(initialBrightness_);

//    particleEmitter_ = rootNode_->CreateComponent<ParticleEmitter>();
    particleEmitter_->SetEffect(MC->cache_->GetResource<ParticleEffect>("Particles/Explosion.xml"));

    sample_ = MC->cache_->GetResource<Sound>("Samples/Explode.ogg");
    sample_->SetLooped(false);
    sampleSource_ = rootNode_->CreateComponent<SoundSource>();
    sampleSource_->SetSoundType(SOUND_EFFECT);
}

void Explosion::UpdateExplosion(StringHash eventType, VariantMap& eventData)
{
    float timeStep{eventData[Update::P_TIMESTEP].GetFloat()};

    rigidBody_->SetMass(Max(initialMass_*((0.1f - age_)/0.1f),0.01f));
    light_->SetBrightness(Max(initialBrightness_*(0.32f - age_)/0.32f,0.0f));

    if (rootNode_->IsEnabled() && MC->world.scene->IsUpdateEnabled()) {
        PODVector<RigidBody* > hitResults{};
        float radius{2.0f * initialMass_ + age_ * 7.0f};
        if (MC->PhysicsSphereCast(hitResults,rootNode_->GetPosition(), radius, M_MAX_UNSIGNED)) {
            for (int i = 0; i < hitResults.Size(); i++){
                Vector3 hitNodeWorldPos{hitResults[i]->GetNode()->GetWorldPosition()};
                if (!hitResults[i]->IsTrigger() && hitResults[i]->GetPosition().y_ > -0.1f) {
                    //positionDelta is used for force calculation
                    Vector3 positionDelta{hitNodeWorldPos - rootNode_->GetWorldPosition()};
                    float distance{positionDelta.Length()};
                    Vector3 force{positionDelta.Normalized() * Max(radius-distance, 0.0f)
                                * timeStep * 2342.0f * rigidBody_->GetMass()};
                    hitResults[i]->ApplyForce(force);
                    //Deal damage
                    unsigned hitID{hitResults[i]->GetNode()->GetID()};
                    float damage{rigidBody_->GetMass()*timeStep};
                    if (MC->spawnMaster_->spires_.Keys().Contains(hitID)) {
                        MC->spawnMaster_->spires_[hitID]->Hit(damage, playerID_);
                    } else if (MC->spawnMaster_->razors_.Keys().Contains(hitID)) {
                        MC->spawnMaster_->razors_[hitID]->Hit(damage, playerID_);
                    }
                }
            }
        }
    }
}

void Explosion::Set(const Vector3 position, const Color color, const float size, int playerID)
{
    playerID_ = playerID;
    Effect::Set(position);
    rootNode_->SetScale(size);
    initialMass_ = 3.0f * size;
    rigidBody_->SetMass(initialMass_);
    light_->SetColor(color);
    light_->SetBrightness(initialBrightness_);

    ParticleEffect* particleEffect{particleEmitter_->GetEffect()};
    Vector<ColorFrame> colorFrames{};
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.0f));
    Color mixColor{0.5f * (color + particleEffect->GetColorFrame(1)->color_)};
    colorFrames.Push(ColorFrame(mixColor, 0.1f));
    colorFrames.Push(ColorFrame(mixColor*0.1f, 0.35f));
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.0f));
    particleEffect->SetColorFrames(colorFrames);

    sampleSource_->SetGain(Min(0.5f*size, 1.0f));
    sampleSource_->Play(sample_);

    MC->tileMaster_->AddToAffectors(WeakPtr<Node>(rootNode_), WeakPtr<RigidBody>(rigidBody_));

    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Explosion, UpdateExplosion));
}

void Explosion::Disable()
{
    UnsubscribeFromEvent(E_POSTUPDATE);
    Effect::Disable();
}
