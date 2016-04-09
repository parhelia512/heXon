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

#include "line.h"

Line::Line(Context* context, MasterControl *masterControl) :
    Object(context),
    masterControl_{masterControl},
    baseScale_{Random(0.42f, 0.666f)}
{
    rootNode_ = masterControl_->world.scene->CreateChild("Line");
    rootNode_->SetScale(baseScale_);
    model_ = rootNode_->CreateComponent<StaticModel>();
    model_->SetModel(masterControl_->cache_->GetResource<Model>("Models/Line.mdl"));
}

void Line::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

    if (!rootNode_->GetComponent<StaticModel>()->IsInView() && rootNode_->GetPosition().y_ > 5.0f)
        Disable();

    rootNode_->Translate(Vector3::UP * timeStep * (42.23f + baseScale_ * 34.0f), TS_WORLD);
}

void Line::Set(const Vector3 position, int playerID)
{
    model_->SetMaterial(playerID == 2
                        ? masterControl_->cache_->GetResource<Material>("Materials/PurpleBullet.xml")
                        : masterControl_->cache_->GetResource<Material>("Materials/GreenBullet.xml"));
    rootNode_->SetEnabledRecursive(true);
    rootNode_->SetPosition(position);
    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Line, HandleSceneUpdate));
}

void Line::Disable()
{
    rootNode_->SetEnabledRecursive(false);
    UnsubscribeFromEvent(E_SCENEUPDATE);
}



