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

#ifndef TILE_H
#define TILE_H

#include <Urho3D/Urho3D.h>

#include "tilemaster.h"

namespace Urho3D {
class Node;
}

using namespace Urho3D;

class Tile : public Object
{
    URHO3D_OBJECT(Tile, Object);
    friend class TileMaster;
    friend class InputMaster;
    friend class SpawnMaster;
public:
    IntVector2 coords_;
    TileType tileType_;
    Tile(Context *context, TileMaster* tileMaster, Vector3 position);
private:
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    MasterControl* masterControl_;
    TileMaster* tileMaster_;

    Node* rootNode_;
    Vector3 referencePosition_;
    float centerDistExp_;
    float wave_;
    float lastOffsetY_;
    StaticModel* model_;
    bool flipped_;

    Node* cursor_;
    void FixFringe();
    void SetTileType(TileType type);
};

#endif // TILE_H
