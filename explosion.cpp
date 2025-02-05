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
#include "chaomine.h"

void Explosion::RegisterObject(Context *context)
{
    context->RegisterFactory<Explosion>();
}

Explosion::Explosion(Context* context):
    Effect(context),
    playerID_{0},
    initialMass_{3.0f},
    initialBrightness_{8.0f}
{
}

void Explosion::OnNodeSet(Node *node)
{
    Effect::OnNodeSet(node);

    node_->SetName("Explosion");

    rigidBody_ = node_->CreateComponent<RigidBody>();
    rigidBody_->SetMass(initialMass_);
    rigidBody_->SetLinearFactor(Vector3::ONE - Vector3::UP);

    light_ = node_->CreateComponent<Light>();
    light_->SetRange(13.0f);
    light_->SetBrightness(initialBrightness_);

    particleEmitter_->SetEffect(CACHE->GetResource<ParticleEffect>("Particles/Explosion.xml"));

    sample_ = CACHE->GetResource<Sound>("Samples/Explode.ogg");
    sample_->SetLooped(false);
    sampleSource_ = node_->CreateComponent<SoundSource>();
    sampleSource_->SetSoundType(SOUND_EFFECT);
}

void Explosion::Update(float timeStep)
{
    Effect::Update(timeStep);

    rigidBody_->SetMass(Max(initialMass_ * ((0.1f - age_) / 0.1f), 0.01f));
    light_->SetBrightness(Max(initialBrightness_ * (0.32f - age_) / 0.32f, 0.0f));

    if (node_->IsEnabled() && MC->scene_->IsUpdateEnabled()) {
        PODVector<RigidBody*> hitResults{};
        float radius{ 2.0f * initialMass_ + age_ * 7.0f };

        if (MC->PhysicsSphereCast(hitResults, node_->GetPosition(), radius, M_MAX_UNSIGNED)) {

            for (RigidBody* h : hitResults){
                if (h->GetNode()->GetName() == "PickupTrigger")
                    h = h->GetNode()->GetParent()->GetComponent<RigidBody>();

                Vector3 hitNodeWorldPos{h->GetNode()->GetWorldPosition()};
                if (!h->IsTrigger() && h->GetPosition().y_ > -0.1f) {
                    //positionDelta is used for force calculation
                    Vector3 positionDelta{ hitNodeWorldPos - node_->GetWorldPosition() };
                    float distance{ positionDelta.Length() };
                    Vector3 force{ positionDelta.Normalized() * Max(radius-distance, 0.0f)
                                 * timeStep * 2342.0f * rigidBody_->GetMass() };
                    h->ApplyForce(force);

                    //Deal damage
                    float damage{rigidBody_->GetMass() * timeStep};

                    for (Component* c : h->GetNode()->GetComponents()){
                        if (c->IsInstanceOf<Enemy>() && !c->IsInstanceOf<ChaoMine>()){
                            Enemy* e{ static_cast<Enemy*>(c) };
                                e->Hit(damage, playerID_);
                            }
                    }
                }
            }
        }
    }
}

void Explosion::Set(const Vector3 position, const Color color, const float size, int colorSet)
{
    playerID_ = colorSet;
    Effect::Set(position);
    node_->SetScale(size);
    initialMass_ = 3.0f * size;
    rigidBody_->SetMass(initialMass_);
    light_->SetColor(color);
    light_->SetBrightness(initialBrightness_);

    ParticleEffect* particleEffect{ particleEmitter_->GetEffect() };
    Vector<ColorFrame> colorFrames{};
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.0f));
//    Color mixColor{0.5f * (color + particleEffect->GetColorFrame(1)->color_)};
    colorFrames.Push(ColorFrame(color, 0.1f));
    colorFrames.Push(ColorFrame(color * 0.1f, 0.35f));
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.0f));
    particleEffect->SetColorFrames(colorFrames);

    sampleSource_->SetGain(Min(0.5f * size, 1.0f));
    sampleSource_->Play(sample_);

    MC->arena_->AddToAffectors(WeakPtr<Node>(node_), WeakPtr<RigidBody>(rigidBody_));

}

void Explosion::Disable()
{
//    UnsubscribeFromEvent(E_POSTUPDATE);
    Effect::Disable();
}
