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

#include "lobby.h"
#include "door.h"
#include "ship.h"
#include "splatterpillar.h"
#include "highest.h"
#include "effectmaster.h"

void Lobby::RegisterObject(Context *context)
{
    context->RegisterFactory<Lobby>();
}

Lobby::Lobby(Context* context) : LogicComponent(context)
{
}

void Lobby::OnNodeSet(Node *node)
{ (void)node;

    node_->Rotate(Quaternion(0.0f, 0.0f, 0.0f));
    Node* chamberNode{ node_->CreateChild("Chamber") };
    StaticModel* chamberModel{ chamberNode->CreateComponent<StaticModel>() };
    chamberModel->SetModel(MC->GetModel("Chamber"));
    chamberModel->SetMaterial(0, MC->GetMaterial("Marble"));
    chamberModel->SetMaterial(1, MC->GetMaterial("PitchBlack"));
    chamberModel->SetMaterial(2, MC->GetMaterial("BlueGlowEnvmap"));
    chamberModel->SetMaterial(3, MC->GetMaterial("Drain"));
    chamberModel->SetCastShadows(true);

    //Create highest
    Node* highestNode{ node_->CreateChild("Highest") };
    highest_ = highestNode->CreateComponent<Highest>();

    //Create coliders
    node_->CreateComponent<RigidBody>();
    node_->CreateComponent<CollisionShape>()->SetTriangleMesh(MC->GetModel("Chamber_COLLISION"));
    node_->CreateComponent<CollisionShape>()->SetBox(Vector3(5.5f, 1.0f, 1.0f));
    node_->CreateComponent<Navigable>()->SetRecursive(false);

    //Create lights
    for (int l{0}; l < 7; ++l){

        Node* pointLightNode{ node_->CreateChild("PointLight") };
        pointLightNode->LookAt(Vector3::DOWN);
        pointLightNode->SetPosition(Vector3::RIGHT * 5.0f * (l != 0) + Vector3::UP * 2.3f);
        pointLightNode->RotateAround(Vector3::ZERO, Quaternion(60.0f * l, Vector3::UP), TS_WORLD);
        Light* spotLight{ pointLightNode->CreateComponent<Light>() };
        spotLight->SetLightType(LIGHT_SPOT);
        spotLight->SetRange(5.0f);
        spotLight->SetFov(120.0f + ((l == 0) * 40.0f));
        spotLight->SetCastShadows(true);
        spotLight->SetShadowBias(BiasParameters(0.0001f, 0.001f));
    }
    //Create doors and splatterpillars
    Node* doorNode{ node_->CreateChild("Door") };
    doorNode->SetPosition(Vector3( 0.0f, 0.0f, 5.21843f));
    doorNode->CreateComponent<Door>();

    Node* splatterPillarNode{ node_->CreateChild("SplatterPillar") };
    splatterPillarNode->SetPosition(Vector3( 0.0f, 0.0f, -4.36142));
    splatterPillarNode->CreateComponent<SplatterPillar>();

    SubscribeToEvent(E_ENTERLOBBY, URHO3D_HANDLER(Lobby, EnterLobby));
    SubscribeToEvent(E_ENTERPLAY,  URHO3D_HANDLER(Lobby, EnterPlay));
}

void Lobby::Update(float timeStep)
{ (void)timeStep;

    PODVector<Node*> lightNodes{};
    node_->GetChildrenWithComponent<Light>(lightNodes);
    for (Node* lightNode : lightNodes){
        lightNode->GetComponent<Light>()->SetBrightness(
                    MC->Sine(2.3f, 0.666f, 1.0f,
                             M_PI_4 * M_DEGTORAD * (lightNode->GetPosition().Angle(Vector3::BACK) )));
    }
}

void Lobby::EnterLobby(StringHash eventType, VariantMap &eventData)
{
    node_->SetEnabledRecursive(true);
}
void Lobby::EnterPlay(StringHash eventType, VariantMap &eventData)
{
    node_->SetEnabledRecursive(false);
}
