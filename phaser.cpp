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

#include "phaser.h"
#include "player.h"

void Phaser::RegisterObject(Context *context)
{
    context->RegisterFactory<Phaser>();
}

Phaser::Phaser(Context* context) : Effect(context),
    phaseMaterial_{MC->GetMaterial("Phase")->Clone()},
    staticModels_{}
//    velocity_{vel}
{
}

void Phaser::OnNodeSet(Node *node)
{/*
    Effect::OnNodeSet(node);

    node_->SetName("Phaser");
    Set(pos);
    node_->LookAt(pos+vel);

    staticModels_.Push(SharedPtr<StaticModel>(node_->CreateComponent<StaticModel>()));
    staticModels_[0]->SetModel(model);

    SetMaterial();

    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Phaser, HandlePostUpdate));
*/}

void Phaser::SetMaterial()
{
    for (StaticModel* m : staticModels_)
        m->SetMaterial(phaseMaterial_);
}

void Phaser::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{/*
    float timeStep{eventData[PostUpdate::P_TIMESTEP].GetFloat()};
    node_->Translate(velocity_ * timeStep, TS_WORLD);
    phaseMaterial_->SetShaderParameter("Dissolve", age_ * 2.3f);
    if (age_ > 2.0f){
        Disable();
        if (!MC->GetPlayer(1)->IsEnabled() &&
            !MC->GetPlayer(2)->IsEnabled() &&
            MC->GetGameState() != GS_LOBBY)
        {
            MC->SetGameState(GS_LOBBY);
        }
        node_->Remove();
        delete this;
    }
*/}
