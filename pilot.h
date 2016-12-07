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

#ifndef PILOT_H
#define PILOT_H

#include <Urho3D/Urho3D.h>

#include "controllable.h"

enum PilotColor { PC_SKIN, PC_SHIRT, PC_PANTS, PC_SHOES, PC_HAIR, PC_ALL };
enum Hair{HAIR_BALD, HAIR_SHORT, HAIR_MOHAWK, HAIR_SEAGULL, HAIR_MUSTAIN, HAIR_FROTOAD, HAIR_FLATTOP, HAIR_ALL};


class Pilot : public Controllable
{
#define SPAWNPOS Vector3(playerId_ * 0.88f - 2.3f - Random(0.23f), 0.0f, 5.666f - Random(0.42f))

    URHO3D_OBJECT(Pilot, Controllable);
    friend class Player;
    friend class MasterControl;
public:
    Pilot(Context* context);
    static void RegisterObject(Context* context);
    virtual void OnNodeSet(Node* node);
    virtual void Update(float timeStep);

    void Randomize();
    void Initialize(int playerId);
    int GetPlayerId() { return playerId_; }
    unsigned GetScore() const { return score_; }
    void Upload();
    void Trip(bool rightFoot);
    virtual void ClearControl();
    void HandleNodeCollisionStart(StringHash eventType, VariantMap& eventData);
    void EnterLobbyFromShip();
    virtual void Think();
private:
    unsigned score_;
//    unsigned flightScore_;
//    int multiplier_;

    int playerId_;
    bool male_;
    bool alive_;
    float deceased_;
    bool autoPilot_;
    int hairStyle_;
    HashMap<int, Color> pilotColors_;
    AnimatedModel* hairModel_;

    void Load();
    void UpdateModel();
    void Save(int playerID, unsigned score);
    void Die();
    void Revive();
    void EnterLobbyThroughDoor();
};

#endif // PILOT_H
