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

#ifndef BUBBLE_H
#define BUBBLE_H

#include <Urho3D/Urho3D.h>
#include "sceneobject.h"

using namespace Urho3D;

class Bubble : public Object
{
    URHO3D_OBJECT(Bubble, Object);
public:
    Bubble(MasterControl* masterControl);
    void HandleSceneUpdate(StringHash eventType, VariantMap &eventData);
    void Set(const Vector3 position);
    void Disable();
    bool IsEnabled() { return rootNode_->IsEnabled(); }
private:
    MasterControl* masterControl_;
    Node* rootNode_;
    Vector3 spinAxis_;
    float spinVelocity_;
    float baseScale_;
};

#endif // BUBBLE_H
