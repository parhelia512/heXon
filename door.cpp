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

Door::Door(MasterControl* masterControl, bool right) :
    Object(masterControl->GetContext()),
    masterControl_{masterControl},
    right_{right},
    wasNear_{false}
{
    player_ = right ? masterControl_->GetPlayer(2) : masterControl_->GetPlayer(1);
    rootNode_ = masterControl_->lobbyNode_->CreateChild("Door");
    rootNode_->SetPosition(Vector3(right_? 2.26494f : -2.26494f, 0.0f, 5.21843f));
    door_ = rootNode_->CreateComponent<AnimatedModel>();
    door_->SetModel(masterControl_->cache_->GetResource<Model>("Models/Door.mdl"));
    door_->SetMaterial(0, masterControl_->resources.materials.basic);
    door_->SetCastShadows(true);

    Node* lightNode = rootNode_->CreateChild("DoorLight");
    lightNode->SetPosition(Vector3(0.0f, 0.666f, 2.3f));
    Light* doorLight = lightNode->CreateComponent<Light>();
    doorLight->SetBrightness(5.0f);
    doorLight->SetCastShadows(true);
    doorLight->SetShadowBias(BiasParameters(0.000023, 0.042f));

    doorSample_ = masterControl_->cache_->GetResource<Sound>("Samples/Door.ogg");
    doorSample_->SetLooped(false);

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Door, HandleSceneUpdate));
}

void Door::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    bool playerNear = LucKey::Distance(rootNode_->GetWorldPosition(), player_->GetWorldPosition()) < 0.666f;
    if (playerNear != wasNear_){
        player_->PlaySample(doorSample_);
    }
    wasNear_ = playerNear;

    door_->SetMorphWeight(0, Lerp(door_->GetMorphWeight(0),
                                  static_cast<float>(playerNear),
                                  eventData[SceneUpdate::P_TIMESTEP].GetFloat() *23.0f));
}
