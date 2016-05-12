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

#ifndef SPLATTERPILLAR_H
#define SPLATTERPILLAR_H


#include "mastercontrol.h"

#include <Urho3D/Urho3D.h>

class Player;

class SplatterPillar : public Object
{
    URHO3D_OBJECT(SplatterPillar, Object);
public:
    SplatterPillar(bool right);
    Vector3 GetPosition() const { return rootNode_->GetPosition(); }
    bool IsIdle() const;
private:
    Player* player_;
    Node* rootNode_;
    Node* pillarNode_;
    Node* bloodNode_;
    Node* particleNode_;
    AnimatedModel* pillar_;
    AnimatedModel* blood_;
    SoundSource* soundSource_;
    ParticleEmitter* splatEmitter_;
    ParticleEmitter* dripEmitter_;

    bool right_;
    bool spun_;
    bool reset_;
    float lastTriggered_;
    float delay_;
    float delayed_;
    float sequenceLength_;
    float rotationSpeed_;

    void HandleSceneUpdate(StringHash eventType, VariantMap &eventData);
    void Trigger();
};

#endif // SPLATTERPILLAR_H
