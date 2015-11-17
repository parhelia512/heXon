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

#include "hitfx.h"

HitFX::HitFX(Context *context, MasterControl *masterControl):
    Effect(context, masterControl)
{
    rootNode_->SetName("HitFX");
    particleEmitter_ = rootNode_->CreateComponent<ParticleEmitter>();
    ParticleEffect* particleEffect = masterControl_->cache_->GetResource<ParticleEffect>("Resources/Particles/HitFX.xml");
    particleEmitter_->SetEffect(particleEffect);

    sample_ = masterControl_->cache_->GetResource<Sound>("Resources/Samples/Hit.ogg");
    sample_->SetLooped(false);
    sampleSource_ = rootNode_->CreateComponent<SoundSource>();
    sampleSource_->SetGain(0.23f);
    sampleSource_->SetSoundType(SOUND_EFFECT);
}

void HitFX::Set(Vector3 position, bool sound)
{
    if (sound) sampleSource_->Play(sample_);
    Effect::Set(position);
}
