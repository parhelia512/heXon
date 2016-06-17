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

#include "hitfx.h"

void HitFX::RegisterObject(Context *context)
{
    context->RegisterFactory<HitFX>();
}

HitFX::HitFX(Context* context):
    Effect(context)
{
}

void HitFX::OnNodeSet(Node *node)
{
    Effect::OnNodeSet(node);

    node_->SetName("HitFX");
    particleEmitter_ = node_->CreateComponent<ParticleEmitter>();

    sample_ = MC->GetSample("Hit");
    sample_->SetLooped(false);
}

void HitFX::Set(const Vector3 position, int playerID, const bool sound)
{
    ParticleEffect* effect{nullptr};
    switch (playerID) {
    case 1: effect = CACHE->GetResource<ParticleEffect>("Particles/GreenHitFX.xml");
        break;
    case 2: effect = CACHE->GetResource<ParticleEffect>("Particles/PurpleHitFX.xml");
        break;
    default: effect = CACHE->GetResource<ParticleEffect>("Particles/HitFX.xml");
        break;
    }

    particleEmitter_->SetEffect(effect);

    if (sound) PlaySample(sample_, 0.23f);
    Effect::Set(position);
}
