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
#include "pilot.h"

void Door::RegisterObject(Context *context)
{
    context->RegisterFactory<Door>();
}

Door::Door(Context* context) :
    LogicComponent(context),
    open_{false},
    hiding_{0.0f}
{
}

void Door::OnNodeSet(Node *node)
{ (void)node;

    model_ = node_->CreateComponent<AnimatedModel>();
    model_->SetModel(MC->GetModel("Door"));
    model_->SetMaterial(0, MC->GetMaterial("Basic"));
    model_->SetCastShadows(true);

    Node* lightNode{ node_->CreateChild("DoorLight") };
    lightNode->SetPosition(Vector3(0.0f, 0.666f, 2.3f));
    Light* doorLight{ lightNode->CreateComponent<Light>() };
    doorLight->SetRange(10.0f);
    doorLight->SetBrightness(5.0f);
    doorLight->SetCastShadows(true);
    doorLight->SetShadowBias(BiasParameters(0.00008f, 0.042f));

    doorSample_ = MC->GetSample("Door");
    doorSample_->SetLooped(false);
    node_->CreateComponent<SoundSource>();

    Node* triggerNode{ node_->CreateChild("Trigger") };
    RigidBody* triggerBody{ triggerNode->CreateComponent<RigidBody>() };
    triggerBody->SetTrigger(true);
    CollisionShape* trigger{ triggerNode->CreateComponent<CollisionShape>() };
    trigger->SetBox(Vector3(2.0f, 3.0f, 1.0f), Vector3::BACK * 0.34f);

    node_->CreateComponent<RigidBody>();
    for (float x : { -0.5f, 0.5f }){

        CollisionShape* collider{ node_->CreateComponent<CollisionShape>() };
        collider->SetCapsule(0.2f, 3.0f, Vector3( x, 0.0f, -0.23f));
    }

    SubscribeToEvent(triggerNode, E_NODECOLLISIONSTART, URHO3D_HANDLER(Door, Open));
    SubscribeToEvent(triggerNode, E_NODECOLLISIONEND, URHO3D_HANDLER(Door, Close));

}

void Door::Open(StringHash eventType, VariantMap& eventData)
{ (void)eventType; (void)eventData;

    node_->GetComponent<SoundSource>()->Play(doorSample_);
    open_ = true;
}
void Door::Close(StringHash eventType, VariantMap& eventData)
{ (void)eventType; (void)eventData;

    node_->GetComponent<SoundSource>()->Play(doorSample_);
    open_ = false;
}

float Door::HidesPilot() const
{
    return hiding_;
}

void Door::Update(float timeStep)
{

    model_->SetMorphWeight(0, Lerp( model_->GetMorphWeight(0),
                                  static_cast<float>(open_),
                                  timeStep * 23.0f) );
}
