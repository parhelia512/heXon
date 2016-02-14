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

Tile::Tile(Context *context, TileMaster* tileMaster, Vector3 position):
    Object(context),
    tileMaster_{tileMaster},
    lastOffsetY_{0.666f},
    flipped_{Random(2)}
{
    masterControl_ = tileMaster->masterControl_;
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Tile, HandleUpdate));
    rootNode_ = tileMaster_->rootNode_->CreateChild("Tile");
    rootNode_->SetPosition(position);
    rootNode_->SetScale(1.1f);
    model_ = rootNode_->CreateComponent<StaticModel>();
    model_->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Hexagon.mdl"));
    model_->SetMaterial(masterControl_->cache_->GetTempResource<Material>("Resources/Materials/BackgroundTile.xml"));
    model_->SetCastShadows(false);

    referencePosition_ = rootNode_->GetPosition();
    centerDistExp_ = exp2(0.75*LucKey::Distance(Vector3::ZERO, referencePosition_));
}

void Tile::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
    float elapsedTime = masterControl_->world.scene->GetElapsedTime();
    float offsetY = 0.0;

    //Switch curcuit
    if (Random(23)==5)
        rootNode_->SetRotation(Quaternion(Random(3)*120.0f + 60.0f*flipped_, Vector3::UP));

    wave_ = 6.0*pow(masterControl_->Sine(Abs(centerDistExp_ - elapsedTime * 5.2625f)), 4.0f);

    unsigned nHexAffectors = tileMaster_->hexAffectors_.Size();
    if (nHexAffectors) {
        for (unsigned i = 0; i < nHexAffectors; i++) {
            WeakPtr<Node> hexAffector = tileMaster_->hexAffectors_.Keys()[i];
            float hexAffectorMass = tileMaster_->hexAffectors_[hexAffector]->GetMass();
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

    Vector3 lastPos = rootNode_->GetPosition();
    Vector3 newPos = Vector3(lastPos.x_, referencePosition_.y_ - Min(offsetY, 4.0f), lastPos.z_);
    rootNode_->SetPosition(newPos);

    bool lobby = masterControl_->GetGameState() == GS_LOBBY;
    float brightness = Clamp((0.23f * offsetY) + 0.25f, 0.0f, 1.0f) + 0.42f*(float)(lobby);
    Color color = Color(brightness +  offsetY * lobby, brightness + offsetY * 0.00042f * (masterControl_->Sine(23.0f, -23.0f - 1000.0f * lobby, 23.0f + 1000.0f * lobby, 23.0f) * wave_), brightness - Random(0.23f) * lobby, brightness + (0.023f * wave_));
    model_->GetMaterial(0)->SetShaderParameter("MatDiffColor", color);
}
