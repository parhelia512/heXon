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

void Line::RegisterObject(Context *context)
{
    context->RegisterFactory<Line>();
}

Line::Line(Context* context) :
    Effect(context),
    baseScale_{Random(1.0f, 2.3f)}
{
}

void Line::OnNodeSet(Node *node)
{ (void)node;

    Effect::OnNodeSet(node_);

    node_->SetName("Line");
    node_->SetScale(baseScale_);
    model_ = node_->CreateComponent<StaticModel>();
    model_->SetModel(MC->GetModel("Line"));

}

void Line::Update(float timeStep)
{
    Effect::Update(timeStep);

    if (((!node_->GetComponent<StaticModel>()->IsInView() && node_->GetPosition().y_ > 5.0f)
        || node_->GetScale().x_ <= 0.0f))
        Disable();

    node_->Translate(Vector3::UP * timeStep * (42.23f + baseScale_ * 23.5f), TS_WORLD);
    node_->SetScale(Max(node_->GetScale().x_ - timeStep, 0.0f));
}

void Line::Set(int colorSet)
{
    float angle{};
    switch(colorSet){
    case 1: angle =  -60.0f; break;
    case 2: angle =   60.0f; break;
    case 3: angle = -120.0f; break;
    case 4: angle =  120.0f; break;
    }

    Vector3 position{ Quaternion(angle, Vector3::UP) *
            (Vector3::FORWARD * Random(32.0f, 42.0f) + Vector3::RIGHT * Random(-13.0f, 13.0f))
            + Vector3::DOWN * (23.0f + Random(46.0f)) };

    Effect::Set(position);
    model_->SetMaterial(MC->colorSets_[colorSet].bulletMaterial_);
    node_->SetScale(baseScale_);
}

void Line::Disable()
{
    node_->SetEnabledRecursive(false);
    UnsubscribeFromEvent(E_SCENEUPDATE);
}



