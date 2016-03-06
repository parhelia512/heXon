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

#include "door.h"
#include "player.h"

Door::Door(Context* context, MasterControl* masterControl, bool right) :
    Object(context),
    masterControl_{masterControl},
    right_{right}
{
    player_ = right ? masterControl_->GetPlayer(2) : masterControl_->GetPlayer(1);
    rootNode_ = masterControl_->lobbyNode_->CreateChild("Door");
    rootNode_->SetPosition(Vector3(right_? 2.26494f : -2.26494f, 0.0f, 5.21843f));
    door_ = rootNode_->CreateComponent<AnimatedModel>();
    door_->SetModel(masterControl_->cache_->GetResource<Model>("Models/Door.mdl"));
    door_->SetMaterial(0, masterControl_->resources.materials.basic);

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Door, HandleSceneUpdate));
}

void Door::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    door_->SetMorphWeight(0, Lerp(door_->GetMorphWeight(0),
                                  LucKey::Distance(rootNode_->GetWorldPosition(), player_->GetWorldPosition()) < 0.42f,
                                  eventData[SceneUpdate::P_TIMESTEP].GetFloat() *23.0f));
}
