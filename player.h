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

#ifndef PLAYER_H
#define PLAYER_H

#include <Urho3D/Urho3D.h>

#include "sceneobject.h"
#include "bullet.h"
#include "muzzle.h"
#include "chaoflash.h"
#include "tailgenerator.h"

namespace Urho3D {
class Drawable;
class Node;
class Scene;
class Sprite;
}

using namespace Urho3D;

typedef struct Ship
{
    Node* node_;
    StaticModel* model_;
} Ship;
typedef struct Pilot
{
    Node* node_;
    bool male_;
    Vector<Color> colors_;
    AnimatedModel* model_;
} Pilot;

class Player : public SceneObject
{
    friend class ChaoMine;
    OBJECT(Player);
public:
    Player(Context* context, MasterControl* masterControl);

    Vector3 GetPosition() { return rootNode_->GetPosition(); }
    double GetHealth(){return health_;}
    void Hit(float damage, bool melee = true);

    void AddScore(int points);
    void Die();
    void ResetScore();
    unsigned GetScore() { return score_; }
    unsigned GetFlightScore() { return flightScore_; }
    void Pickup(PickupType pickup);
    void EnterLobby();
    void EnterPlay();
    void CreateNewPilot();
    void UpdateGUI(float timeStep);
    void PickupChaoBall();
private:
    bool pilotMode_;
    float initialHealth_;
    float health_;
    unsigned score_;
    unsigned flightScore_;
    int multiplier_;
    int weaponLevel_;
    int bulletAmount_;

    int appleCount_;
    int heartCount_;

    float initialShotInterval_;
    float shotInterval_;
    float sinceLastShot_;

    Ship ship_;
    Pilot pilot_;
    Node* shieldNode_;
    StaticModel* shieldModel_;
    SharedPtr<Material> shieldMaterial_;
    ChaoFlash* chaoFlash_;
    Vector3 lastHitDirection_;
    RigidBody* rigidBody_;
    CollisionShape* collisionShape_;
    AnimationController* animCtrl_;

    Node* guiNode_;
    Node* healthBarNode_;
    StaticModel* healthBarModel_;
    Node* shieldBarNode_;
    StaticModel* shieldBarModel_;

    Node* appleCounterRoot_;
    Node* appleCounter_[5];
    Node* heartCounterRoot_;
    Node* heartCounter_[5];

    Vector<SharedPtr<TailGenerator> > tailGens_;
    Vector<SharedPtr<Bullet> > bullets_;
    SharedPtr<Muzzle> muzzle_;
    SharedPtr<Sound> shot_;
    Vector<SharedPtr<SoundSource> > sampleSources_;

    String scoreTextName_;

    void HandleSceneUpdate(StringHash eventType, VariantMap &eventData);
    void PlaySample(Sound *sample);
    void Shoot(Vector3 fire);
    void FireBullet(Vector3 direction);
    void UpgradeWeapons();
    void SetHealth(float health);
    Color HealthToColor(float health);
    void SetScore(int points);
    void SetupShip();
    void CreateTails();
    void RemoveTails();
    void CreateGUI();
    void SetPilotMode(bool pilotMode);
    void MoveMuzzle();
};

#endif // PLAYER_H
