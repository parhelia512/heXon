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

#ifndef BULLET_H
#define BULLET_H

#include "sceneobject.h"

#include "spawnmaster.h"
#include "razor.h"
#include "spire.h"
#include "arena.h"
#include "hitfx.h"

#include <Urho3D/Urho3D.h>

namespace Urho3D {
class Node;
class Scene;
class Sprite;
}

using namespace Urho3D;

class Bullet : public SceneObject
{
    friend class Ship;
    friend class SpawnMaster;
    URHO3D_OBJECT(Bullet, SceneObject);
public:
    Bullet(Context* context);
    static void RegisterObject(Context* context);
    virtual void OnNodeSet(Node* node);
    void Set(const Vector3 position, const int playerId, const Vector3 direction, Vector3 force, const float damage);
    int GetPlayerID() const noexcept { return colorSet_; }
protected:
    SharedPtr<RigidBody> rigidBody_;
    SharedPtr<StaticModel> model_;

    void Update(float timeStep);
private:
    int colorSet_;
    float age_ = 0.0f;
    float timeSinceHit_ = 0.0f;
    float lifeTime_;
    bool fading_ = false;
    float damage_;
    void HitCheck(const float timeStep);
    void Disable();
};

#endif // BULLET_H
