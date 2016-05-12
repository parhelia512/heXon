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

#ifndef DOOR_H
#define DOOR_H

#include "mastercontrol.h"

#include <Urho3D/Urho3D.h>

class Player;

class Door : public Object
{
    URHO3D_OBJECT(Door, Object);
public:
    Door(bool right);
    float HidesPlayer() const;
    Vector3 GetPosition() const { return rootNode_->GetPosition(); }
private:
    Player* player_;
    Node* rootNode_;
    AnimatedModel* door_;
    SharedPtr<Sound> doorSample_;

    bool right_;
    bool wasNear_;
    float hiding_;

    void HandleSceneUpdate(StringHash eventType, VariantMap &eventData);
};

#endif // DOOR_H
