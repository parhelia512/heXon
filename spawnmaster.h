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

#ifndef SPAWNMASTER_H
#define SPAWNMASTER_H

#include <Urho3D/Urho3D.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>

#include "mastercontrol.h"
#include "razor.h"
#include "spire.h"
#include "seeker.h"

class SpawnMaster : public Object
{
    friend class MasterControl;
    OBJECT(SpawnMaster);
public:
    SpawnMaster(Context *context, MasterControl *masterControl);

    HashMap<unsigned, SharedPtr<Razor> > razors_;
    HashMap<unsigned, SharedPtr<Spire> > spires_;
    Vector<SharedPtr<Seeker> > seekers_;

    Vector3 CreateSpawnPoint();

    int CountActiveRazors();
    int CountActiveSpires();
    void SpawnSeeker(Vector3 position);
    void Clear();
private:
    MasterControl* masterControl_;
    void HandleSceneUpdate(StringHash eventType, VariantMap &eventData);

    bool spawning_;

    float razorInterval_;
    float sinceRazorSpawn_;

    float spireInterval_;
    float sinceSpireSpawn_;

    void SpawnRazor(Vector3 position);
    bool RespawnRazor(Vector3 position);
    void SpawnSpire(Vector3 position);
    bool RespawnSpire(Vector3 position);
    bool RespawnSeeker(Vector3 position);
    void Activate();
    void Deactivate();
    void Restart();
};

#endif // SPAWNMASTER_H
