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

#include "bubble.h"

Bubble::Bubble(Context *context, MasterControl *masterControl):
    Object(context),
    masterControl_{masterControl},
    spinAxis_{Vector3(Random(), Random(), Random()).Normalized()},
    spinVelocity_{LucKey::RandomSign() * Random(23.0f, 42.0f)},
    baseScale_{pow(Random(0.23f, 0.88f), 2.3f)}
{
    rootNode_ = masterControl_->world.scene->CreateChild("Bubble");
    StaticModel* model = rootNode_->CreateComponent<StaticModel>();
    model->SetModel(masterControl_->cache_->GetResource<Model>("Models/Box.mdl"));
    model->SetMaterial(masterControl_->cache_->GetResource<Material>("Resources/Materials/Bubble.xml"));
}

void Bubble::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

    if (!rootNode_->GetComponent<StaticModel>()->IsInView() && rootNode_->GetPosition().y_ > 5.0f)
        Disable();

    rootNode_->Translate(Vector3::UP * timeStep * (6.66f + masterControl_->SinceLastReset() * 0.0023f), TS_WORLD);
    rootNode_->Rotate(Quaternion(timeStep * spinVelocity_, spinAxis_));
    rootNode_->SetWorldScale(Vector3(masterControl_->Sine(4.0f - baseScale_, baseScale_*0.88f, baseScale_*1.23f, spinVelocity_),
                                     masterControl_->Sine(5.0f - baseScale_, baseScale_*0.88f, baseScale_*1.23f, spinVelocity_ + 2.0f),
                                     masterControl_->Sine(4.2f - baseScale_, baseScale_*0.88f, baseScale_*1.23f, spinVelocity_ + 3.0f)));
}

void Bubble::Set(const Vector3 position)
{
    rootNode_->SetEnabledRecursive(true);
    rootNode_->SetPosition(position);
    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Bubble, HandleSceneUpdate));
}

void Bubble::Disable()
{
    rootNode_->SetEnabledRecursive(false);
    UnsubscribeFromEvent(E_SCENEUPDATE);
}
