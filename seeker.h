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

#ifndef SEEKER_H
#define SEEKER_H

#include <Urho3D/Urho3D.h>

#include "sceneobject.h"
#include "TailGenerator.h"

using namespace Urho3D;

class Seeker : public SceneObject
{
    friend class ChaoFlash;
    friend class ChaoZap;
    URHO3D_OBJECT(Seeker, SceneObject);
public:
    Seeker(Context* context, MasterControl* masterControl);
    void HandleSceneUpdate(StringHash eventType, VariantMap &eventData);
    void HandleTriggerStart(StringHash eventType, VariantMap &eventData);
    void Set(Vector3 position);
    void Disable();
protected:
    RigidBody* rigidBody_;
    SharedPtr<Sound> sample_;
    SharedPtr<Node> target_;
    SharedPtr<TailGenerator> tailGen_;

    float age_;
    float lifeTime_;
    float damage_;
private:
    void AddTail();
    void RemoveTail();
};

#endif // SEEKER_H
