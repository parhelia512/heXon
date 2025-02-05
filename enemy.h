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

#ifndef ENEMY_H
#define ENEMY_H

#include <Urho3D/Urho3D.h>

#include "explosion.h"
#include "arena.h"

#include "sceneobject.h"

namespace Urho3D {
class Drawable;
class Node;
class Scene;
class Sprite;
}

using namespace Urho3D;

class Enemy : public SceneObject
{
    friend class ChaoFlash;
    friend class SpawnMaster;
    URHO3D_OBJECT(Enemy, SceneObject);
public:
    Enemy(Context* context);
    virtual void OnNodeSet(Node* node);

    float GetHealth() const { return health_; }
    int GetWorth() const { return worth_; }
    float GetPanic() const { return panic_; }
    Vector3 GetLinearVelocity() const { return rigidBody_->GetLinearVelocity(); }
    virtual void Hit(const float damage, const int colorSet);
    virtual void Set(const Vector3 position);
    virtual void Update(float timeStep);
protected:
    float panicTime_;
    float health_;
    float initialHealth_;
    float panic_;
    int worth_;
    int lastHitBy_;

    float sinceLastWhack_;
    const float whackInterval_;
    float meleeDamage_;

    SharedPtr<Node> centerNode_;
    SharedPtr<ParticleEmitter> particleEmitter_;
    SharedPtr<ParticleEffect> particleEffect_;
    SharedPtr<RigidBody> rigidBody_;
    StaticModel* centerModel_;
    Color color_;

    HashMap<int, float> damagePerColorSet_;

    virtual void HandleNodeCollision(StringHash eventType, VariantMap &eventData);
    void SetHealth(const float health);
    virtual void CheckHealth();
    Color GetGlowColor() const;
    void Disable();
private:
    SharedPtr<SoundSource> soundSource_;
};

#endif // ENEMY_H
