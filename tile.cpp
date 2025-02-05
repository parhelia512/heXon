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

#include "tile.h"

void Tile::RegisterObject(Context *context)
{
    context->RegisterFactory<Tile>();
}

Tile::Tile(Context* context):
    LogicComponent(context),
    lastOffsetY_{ 0.666f },
    flipped_{ static_cast<bool>(Random(2)) }
{
}

void Tile::OnNodeSet(Node *node)
{ (void)node;

    node_->SetScale(1.1f);
    model_ = node_->CreateComponent<StaticModel>();
    model_->SetModel(MC->GetModel("Hexagon"));
    model_->SetMaterial(MC->GetMaterial("BackgroundTile")->Clone());
    model_->SetCastShadows(false);

    referencePosition_ = node_->GetPosition();
    centerDistExp_ = static_cast<float>(exp2(static_cast<double>(0.75f*LucKey::Distance(Vector3::ZERO, referencePosition_))));

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Tile, HandleUpdate));
}

void Tile::HandleUpdate(StringHash eventType, VariantMap &eventData)
{ (void)eventType; (void)eventData;

    float elapsedTime{ MC->scene_->GetElapsedTime() };
    float offsetY{ 0.0f };

    //Switch curcuit
    if (Random(23) == 5)
        node_->SetRotation(Quaternion(Random(3)*120.0f + 60.0f*flipped_, Vector3::UP));

    //Calculate periodic tile movement
    wave_ = 6.0f * pow(LucKey::Sine(Abs(centerDistExp_ - elapsedTime * 5.2625f)), 4.0f);

    Arena* arena{ node_->GetParentComponent<Arena>() };
    unsigned nHexAffectors{ arena->hexAffectors_.Size()};
    if (nHexAffectors) {
        for (unsigned i{0}; i < nHexAffectors; ++i) {
            Node* hexAffector = arena->hexAffectors_.Keys()[i];
            float hexAffectorMass = arena->hexAffectors_[hexAffector]->GetMass();
            if (hexAffector->IsEnabled()) {
                float offsetYPart = sqrt(hexAffectorMass) - (0.1f * LucKey::Distance(referencePosition_, hexAffector->GetPosition()));
                if (offsetYPart > 0.0f) {
                    offsetYPart = pow(offsetYPart, 4);
                    offsetY += offsetYPart;
                }
            }
        }
        offsetY = sqrt(offsetY*0.666f);
    }

    offsetY += 0.023f * wave_;

    offsetY = (offsetY + lastOffsetY_) * 0.5f;
    lastOffsetY_ = offsetY;

    Vector3 lastPos{ node_->GetPosition() };
    Vector3 newPos{ lastPos.x_, referencePosition_.y_ - Min(offsetY, 4.0f), lastPos.z_ };
    node_->SetPosition(newPos);

    bool lobby{ MC->GetGameState() == GS_LOBBY };
    float brightness{ Clamp((0.23f * offsetY) + 0.25f, 0.0f, 1.0f) + 0.42f*static_cast<float>(lobby) };
    Color color{ brightness +  offsetY * lobby, brightness + offsetY * 0.00042f * (MC->Sine(23.0f, -23.0f - 1000.0f * lobby, 23.0f + 1000.0f * lobby, 23.0f) * wave_), brightness - Random(0.23f) * lobby, brightness + (0.023f * wave_) };
    model_->GetMaterial(0)->SetShaderParameter("MatDiffColor", color);
}
