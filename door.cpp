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

Door::Door(bool right) :
    Object(MC->GetContext()),
    right_{right},
    wasNear_{true},
    hiding_{0.0f}
{
    player_ = right ? MC->GetPlayer(2) : MC->GetPlayer(1);
    rootNode_ = MC->lobbyNode_->CreateChild("Door");
    rootNode_->SetPosition(Vector3(right_? 2.26494f : -2.26494f, 0.0f, 5.21843f));
    door_ = rootNode_->CreateComponent<AnimatedModel>();
    door_->SetModel(MC->GetModel("Door"));
    door_->SetMaterial(0, MC->GetMaterial("Basic"));
    door_->SetCastShadows(true);

    Node* lightNode{rootNode_->CreateChild("DoorLight")};
    lightNode->SetPosition(Vector3(0.0f, 0.666f, 2.3f));
    Light* doorLight{lightNode->CreateComponent<Light>()};
    doorLight->SetRange(10.0f);
    doorLight->SetBrightness(5.0f);
    doorLight->SetCastShadows(true);
    doorLight->SetShadowBias(BiasParameters(0.000023, 0.042f));

    doorSample_ = CACHE->GetResource<Sound>("Samples/Door.ogg");
    doorSample_->SetLooped(false);

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Door, HandleSceneUpdate));
}

float Door::HidesPlayer() const
{
    return hiding_;
}

void Door::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    bool playerNear{LucKey::Distance(rootNode_->GetWorldPosition(), player_->GetPosition()) < 0.666f};
    if (playerNear != wasNear_) {
        player_->PlaySample(doorSample_);
    }
    wasNear_ = playerNear;

    if (door_->GetMorphWeight(0) < 0.023f && player_->GetPosition().z_ > GetPosition().z_)
        hiding_ += eventData[SceneUpdate::P_TIMESTEP].GetFloat();
    else hiding_ = 0.0f;

    door_->SetMorphWeight(0, Lerp(door_->GetMorphWeight(0),
                                  static_cast<float>(playerNear),
                                  eventData[SceneUpdate::P_TIMESTEP].GetFloat() *23.0f));
}
