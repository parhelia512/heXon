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
    masterControl_{masterControl}
{
    rootNode_ = masterControl_->world.scene->CreateChild("Bubble");
    StaticModel* model = rootNode_->CreateComponent<StaticModel>();
    model->SetModel(masterControl_->cache_->GetResource<Model>("Models/Box.mdl"));
    model->SetMaterial(masterControl_->cache_->GetResource<Material>("Resources/Materials/Loglo.xml"));
}

void Bubble::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

    rootNode_->Translate(Vector3::UP * timeStep * 23.5f);
    if (!rootNode_->GetComponent<StaticModel>()->IsInView())
        Disable();
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
