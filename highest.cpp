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

#include "highest.h"


void Highest::RegisterObject(Context *context)
{
    context->RegisterFactory<Highest>();
}

Highest::Highest(Context* context) : LogicComponent(context)
{

}

void Highest::OnNodeSet(Node *node)
{ (void)node;

    node_->Translate(Vector3(0.0f, 3.2f, -5.0f));
    node_->Rotate(Quaternion(45.0f, Vector3::RIGHT));
    node_->Rotate(Quaternion(180.0f, Vector3::UP));
    Node* podiumNode = node_->CreateChild("Podium");
    podiumNode->SetScale(0.5f);
    StaticModel* hexPodium{podiumNode->CreateComponent<StaticModel>()};
    hexPodium->SetModel(MC->GetModel("Hexagon"));
    hexPodium->SetMaterial(MC->GetMaterial("BackgroundTile")->Clone());
    Node* highestPilotNode{podiumNode->CreateChild("HighestPilot")};
    highestPilotNode->SetScale(2.0f);


    Node* spotNode{node_->CreateChild("HighestSpot")};
    spotNode->Translate(Vector3(0.0f, -2.0f, 3.0f));
    spotNode->LookAt(node_->GetWorldPosition());
    Light* highestSpot{spotNode->CreateComponent<Light>()};
    highestSpot->SetLightType(LIGHT_SPOT);
    highestSpot->SetRange(5.0f);
    highestSpot->SetFov(23.5f);
    highestSpot->SetColor(Color(0.6f, 0.7f, 1.0f));
    highestSpot->SetBrightness(5.0f);
    highestSpot->SetSpecularIntensity(0.23f);
}



