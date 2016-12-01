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

#ifndef PLAYER_H
#define PLAYER_H

#include <Urho3D/Urho3D.h>
#include "luckey.h"

class GUI3D;
class Ship;

class Player : public Object
{
    URHO3D_OBJECT(Player, Object);
public:
    Player(int playerId, Context* context);
//    static void RegisterObject(Context* context);
//    virtual void OnNodeSet(Node* node);

    Vector3 GetPosition();
    Ship *GetShip();

    int GetPlayerId() const { return playerId_; }
    void AddScore(int points);
    unsigned GetScore() const { return score_; }
    unsigned GetFlightScore() const { return flightScore_; }
    void Die();
    void Respawn();
    void ResetScore();

    bool IsAlive() const noexcept { return alive_; }
    bool IsHuman() const noexcept { return !autoPilot_; }
    void EnterLobby();
    void EnterPlay();

    GUI3D* gui3d_;
private:
    int playerId_;
    bool autoPilot_;
    bool alive_;

    unsigned score_;
    unsigned flightScore_;
    int multiplier_;


    void SetScore(int points);
    Vector3 Sniff(float playerFactor, bool taste);

    /*Node* shieldNode_;
    StaticModel* shieldModel_;
    SharedPtr<Material> shieldMaterial_;
    ChaoFlash* chaoFlash_;
    Vector3 lastHitDirection_;
    RigidBody* rigidBody_;
    CollisionShape* collisionShape_;
    AnimationController* animCtrl_;

    Node* guiNode_;
    Node* scoreNode_;
    Node* healthBarNode_;
    StaticModel* healthBarModel_;
    Node* shieldBarNode_;
    StaticModel* shieldBarModel_;

    Node* appleCounterRoot_;
    Node* appleCounter_[4];
    Node* heartCounterRoot_;
    Node* heartCounter_[4];
    Node* scoreDigits_[10];

    Vector<SharedPtr<TailGenerator> > tailGens_;

    SharedPtr<Muzzle> muzzle_;

    SharedPtr<SoundSource> deathSource_;
    SharedPtr<Sound> shot_s;
    SharedPtr<Sound> shieldHit_s;
    SharedPtr<Sound> death_s;
    Vector<SharedPtr<Sound>> pickup_s;
    SharedPtr<Sound> powerup_s;
    SharedPtr<Sound> multix_s;
    SharedPtr<Sound> chaoball_s;
    Vector<SharedPtr<Sound> > seekerHits_s;

    void HandleSceneUpdate(StringHash eventType, VariantMap &eventData);
    void Shoot(Vector3 fire);
    void FireBullet(Vector3 direction);
    Color HealthToColor(float health);
    void CreateGUI();
    void SetPilotMode(bool pilotMode);
    void MoveMuzzle();
    void LoadPilot();
    void Think();
    Vector3 Sniff(float playerFactor, bool taste = false);
    void CountScore();*/
};

#endif // PLAYER_H
