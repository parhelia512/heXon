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

#ifndef LINE_H
#define LINE_H

#include <Urho3D/Urho3D.h>
#include "mastercontrol.h"

class Line : public Object
{
    URHO3D_OBJECT(Line, Object);
public:
    Line(MasterControl *masterControl);
    void HandleSceneUpdate(StringHash eventType, VariantMap &eventData);
    void Set(const Vector3 position, int playerID);
    void Disable();
    bool IsEnabled() { return rootNode_->IsEnabled(); }
private:
    MasterControl* masterControl_;
    Node* rootNode_;
    float baseScale_;
    StaticModel* model_;
};

#endif // LINE_H
