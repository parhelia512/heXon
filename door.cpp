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

#include "inputmaster.h"
#include "pilot.h"

#include "door.h"

void Door::RegisterObject(Context *context)
{
    context->RegisterFactory<Door>();
}

Door::Door(Context* context) :
    LogicComponent(context),
    open_{false}
{
}

void Door::OnNodeSet(Node *node)
{ (void)node;

    model_ = node_->CreateComponent<AnimatedModel>();
    model_->SetModel(MC->GetModel("Door"));
    model_->SetMaterial(0, MC->GetMaterial("Basic"));
    model_->SetCastShadows(true);

    Node* lightNode{ node_->CreateChild("DoorLight") };
    lightNode->LookAt(Vector3::BACK);
    lightNode->SetPosition(Vector3(0.0f, 1.0, 3.4f));
    Light* doorLight{ lightNode->CreateComponent<Light>() };
    doorLight->SetLightType(LIGHT_SPOT);
    doorLight->SetFov(160.0f);
    doorLight->SetRange(11.0f);
    doorLight->SetBrightness(6.66f);
    doorLight->SetSpecularIntensity(0.05f);
    doorLight->SetCastShadows(true);
    doorLight->SetShadowBias(BiasParameters(0.000000023f, 0.42f));
//    doorLight->SetShadowResolution(0.5f);

    node_->CreateComponent<SoundSource>();

    RigidBody* triggerBody{ node_->CreateComponent<RigidBody>() };
    triggerBody->SetTrigger(true);
    CollisionShape* trigger{ node_->CreateComponent<CollisionShape>() };
    trigger->SetBox(Vector3(4.23f, 3.0f, 1.0f), Vector3::BACK * 0.34f);

    /*node_->CreateComponent<RigidBody>();
    for (float x : { -2.05f, 2.05f }){

        CollisionShape* collider{ node_->CreateComponent<CollisionShape>() };
        collider->SetCapsule(0.2f, 3.0f, Vector3( x, 0.0f, -0.23f));
    }*/

    SubscribeToEvent(node_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Door, Open));
    SubscribeToEvent(node_, E_NODECOLLISIONEND, URHO3D_HANDLER(Door, Close));

}

void Door::Open(StringHash eventType, VariantMap& eventData)
{ (void)eventType; (void)eventData;

    node_->GetComponent<SoundSource>()->Play(MC->GetSample("Door"));
    open_ = true;
}
void Door::Close(StringHash eventType, VariantMap& eventData)
{ (void)eventType; (void)eventData;

    PODVector<RigidBody*> colliders{};
    node_->GetComponent<RigidBody>()->GetCollidingBodies(colliders);
    if (!colliders.Size()) {

        node_->GetComponent<SoundSource>()->Play(MC->GetSample("Door"));
        open_ = false;
    }
}

bool Door::HidesAllPilots(bool onlyHuman) const
{
    Vector<Controllable*> controllables{ GetSubsystem<InputMaster>()->GetControlled() };

    for (Controllable* c : controllables) {

            if (c->IsInstanceOf<Pilot>()) {

                if (c->GetPosition().z_ < node_->GetPosition().z_) {

                    if (c->GetPlayer()->IsHuman() || !onlyHuman)
                    return false;
                }

            } else return false;
    }
    return model_->GetMorphWeight(0) < 0.0023f || onlyHuman;
}

void Door::Update(float timeStep)
{

    model_->SetMorphWeight(0, Lerp( model_->GetMorphWeight(0),
                                  static_cast<float>(open_),
                                  timeStep * 7.0f) );
}
