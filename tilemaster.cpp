/* heXon
// Copyright (C) 2015 LucKey Productions (luckeyproductions.nl)
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

#include "mastercontrol.h"
#include "tilemaster.h"
#include "tile.h"

namespace Urho3D {
template <> unsigned MakeHash(const IntVector2& value)
  {
    return LucKey::IntVector2ToHash(value);
  }
}

TileMaster::TileMaster(Context *context, MasterControl* masterControl):
Object(context),
  masterControl_{masterControl},
  targetPosition_{Vector3::UP * 0.666f},
  targetScale_{Vector3::ONE * 0.05f}
{
    rootNode_ = masterControl_->world.scene->CreateChild("TileMaster");
    rootNode_->SetPosition(targetPosition_);
    rootNode_->SetScale(targetScale_);

    //Create hexagonal field
    //Lays a field of hexagons at the origin
    int bigHexSize = 23;
    for (int i = 0; i < bigHexSize; i++) {
        for (int j = 0; j < bigHexSize; j++) {
            if (    i < (bigHexSize - bigHexSize / 4) + j / 2 &&                            //Exclude bottom right
                    i > (bigHexSize / 4) - (j + 1) / 2 &&                                   //Exclude bottom left
                    i + 1 < (bigHexSize - bigHexSize / 4) + ((bigHexSize - j + 1)) / 2 &&   //Exclude top right
                    i - 1 > (bigHexSize / 4) - ((bigHexSize - j + 2) / 2)) {                //Exclude top left
                Vector3 tilePos = Vector3((-bigHexSize / 2.0f + i) * 2.0f + j % 2, -0.1f, (-bigHexSize / 2.0f + j + 0.5f) * 1.8f);
                tileMap_[IntVector2(i, j)] = new Tile(context_, this, tilePos);
            }
        }
    }
    //Add a directional light to the arena.
    Node* lightNode = rootNode_->CreateChild("Sun");
    lightNode->SetPosition(Vector3::UP*5.0f);
    lightNode->SetRotation(Quaternion(90.0f, 0.0f, 0.0f));
    playLight_ = lightNode->CreateComponent<Light>();
    playLight_->SetLightType(LIGHT_DIRECTIONAL);
    playLight_->SetBrightness(0.0f);
    playLight_->SetRange(10.0f);
    playLight_->SetColor(Color(1.0f, 0.9f, 0.95f));
    playLight_->SetCastShadows(false);

    //Create heXon logo
    logoNode_ = rootNode_->CreateChild("heXon");
    logoNode_->SetPosition(Vector3(0.0f, -4.0f, 0.0f));
    logoNode_->SetRotation(Quaternion(0.0f, 180.0f, 0.0f));
    logoNode_->SetScale(16.0f);
    StaticModel* logoModel = logoNode_->CreateComponent<StaticModel>();
    logoModel->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/heXon.mdl"));
    logoMaterial_ = masterControl_->cache_->GetResource<Material>("Resources/Materials/Loglow.xml");
    logoModel->SetMaterial(logoMaterial_);

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(TileMaster, HandleUpdate));
}

void TileMaster::EnterPlayState()
{
    targetPosition_ = Vector3::DOWN * 0.23f;
    targetScale_ = Vector3::ONE;
    Vector<SharedPtr<Tile> > tiles = tileMap_.Values();
    for (unsigned t = 0; t < tiles.Size(); t++){
        tiles[t]->lastOffsetY_ = 2.3f;
    }
}
void TileMaster::EnterLobbyState()
{
    targetPosition_ = Vector3::UP * 0.45f;
    targetScale_ = Vector3::ONE * 0.05f;
}

void TileMaster::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    float timestep = eventData[Update::P_TIMESTEP].GetFloat();
    float lerpFactor = masterControl_->GetGameState() == GS_LOBBY ? 13.0f : 2.3f ;
    float t = Min(1.0f, timestep * lerpFactor);
    rootNode_->SetPosition(rootNode_->GetPosition().Lerp(targetPosition_, t));
    rootNode_->SetScale(rootNode_->GetScale().Lerp(targetScale_, pow(t, 0.88f) ));

    logoNode_->SetPosition(logoNode_->GetPosition().Lerp(masterControl_->GetGameState() == GS_LOBBY
                                                         ? Vector3::UP * 4.0f * masterControl_->Sine(5.0f, -0.1f, 1.23f)
                                                         : Vector3::UP * -4.0f, t));
    logoMaterial_->SetShaderParameter("MatDiffColor", logoMaterial_->GetShaderParameter("MatDiffColor").GetColor().Lerp(
                                          masterControl_->GetGameState() == GS_LOBBY
                                          ? Color(0.42f, 0.666f, 0.666f, 1.0) * masterControl_->Sine(5.0f, 0.42f, 1.0f, 0.23f)
                                          : Color(0.0666f, 0.16f, 0.16f, 0.23f), t));
    playLight_->SetBrightness(masterControl_->GetGameState() == GS_PLAY? 0.8f : 0.0f);
}

Tile* TileMaster::GetRandomTile()
{
    Vector<SharedPtr<Tile> > tiles = tileMap_.Values();
    if (tiles.Size()){
        SharedPtr<Tile> tile;
        while (!tile){
            SharedPtr<Tile> tryTile = tiles[Random((int)tiles.Size())];
            PODVector<PhysicsRaycastResult> hitResults;
            Ray spawnRay(tryTile->rootNode_->GetPosition()-Vector3::UP, Vector3::UP*10.0f);
            if (!masterControl_->PhysicsRayCast(hitResults, spawnRay, 23.0f, M_MAX_UNSIGNED)){
                tile = tryTile;
            }
        }
        return tile.Get();
    }
}
