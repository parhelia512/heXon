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

#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <Urho3D/Urho3D.h>

#include "hitfx.h"
#include "arena.h"

#include "effect.h"

namespace Urho3D {
class Drawable;
class Node;
class Scene;
class Sprite;
}

using namespace Urho3D;

class Explosion : public Effect
{
    friend class Enemy;
    URHO3D_OBJECT(Explosion, Effect);
public:
    Explosion(Context* context);
    static void RegisterObject(Context* context);
    virtual void OnNodeSet(Node* node);
    virtual void Update(float timeStep);

    void Set(const Vector3 position, const Color color, const float size, int colorSet);
    void Disable();
protected:
    SharedPtr<RigidBody> rigidBody_;
    SharedPtr<Light> light_;
private:
    int playerID_;
    SharedPtr<Sound> sample_;
    SharedPtr<SoundSource> sampleSource_;
    float initialMass_;
    float initialBrightness_;
};

#endif // EXPLOSION_H
