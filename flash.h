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

#ifndef FLASH_H
#define FLASH_H

#include <Urho3D/Urho3D.h>

#include "effect.h"

namespace Urho3D {
class Drawable;
class Node;
class Scene;
class Sprite;
}

using namespace Urho3D;

class Flash : public Effect
{
    URHO3D_OBJECT(Flash, Effect);
public:
    Flash(Context* context);
    static void RegisterObject(Context* context);
    virtual void OnNodeSet(Node* node);

    SharedPtr<Light> light_;

    virtual void Update(float timeStep);
    void Set(const Vector3 position, bool big);
    void Disable();
private:
    ParticleEffect* bigFlash_;
    ParticleEffect* smallFlash_;
    float initialBrightness_;
};

#endif // FLASH_H
