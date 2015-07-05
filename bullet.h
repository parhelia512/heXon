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

#ifndef BULLET_H
#define BULLET_H


#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Resource/ResourceCache.h>

#include "sceneobject.h"

namespace Urho3D {
class Node;
class Scene;
class Sprite;
}

class Bullet : public SceneObject
{
    friend class Player;
    friend class SpawnMaster;
    OBJECT(Bullet);
public:
    Bullet(Context *context, MasterControl* masterControl);
protected:
    SharedPtr<RigidBody> rigidBody_;
    SharedPtr<StaticModel> model_;

    void Set(Vector3 position);
    void HandleSceneUpdate(StringHash eventType, VariantMap &eventData);
private:
    float age_ = 0.0f;
    float timeSinceHit_ = 0.0f;
    float lifeTime_;
    bool fading_ = false;
    float damage_;
    void HitCheck(float timeStep);
    void Disable();
};

#endif
