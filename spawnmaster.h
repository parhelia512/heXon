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

#include "mastercontrol.h"
#include "razor.h"
#include "spire.h"
#include "chaozap.h"
#include "chaomine.h"
#include "seeker.h"
#include "flash.h"
#include "explosion.h"

class SpawnMaster : public Object
{
    friend class MasterControl;
    OBJECT(SpawnMaster);
public:
    SpawnMaster(Context *context, MasterControl *masterControl);

    HashMap<unsigned, SharedPtr<Razor> > razors_;
    HashMap<unsigned, SharedPtr<Spire> > spires_;
    HashMap<unsigned, SharedPtr<Seeker> > seekers_;
    HashMap<unsigned, SharedPtr<ChaoMine> > chaoMines_;
    Vector<SharedPtr<HitFX> > hitFXs_;
    Vector<SharedPtr<ChaoZap> > chaoZaps_;
    Vector<SharedPtr<Flash> > flashes_;
    Vector<SharedPtr<Explosion> > explosions_;

    void Clear();
    Vector3 SpawnPoint();

    int CountActiveRazors();
    int CountActiveSpires();
    void SpawnChaoZap(const Vector3 &position);
    void SpawnChaoMine(const Vector3 &position);
    void SpawnSeeker(const Vector3& position);
    void SpawnHitFX(const Vector3& position, bool sound = true);
    void SpawnFlash(const Vector3& position);
    bool SpawnExplosion(const Vector3& position, const Color &color, float size);
private:
    MasterControl* masterControl_;
    void HandleSceneUpdate(StringHash eventType, VariantMap &eventData);

    bool spawning_;

    float razorInterval_;
    float sinceRazorSpawn_;

    float spireInterval_;
    float sinceSpireSpawn_;

    void SpawnRazor(const Vector3& position);
    bool RespawnRazor(const Vector3& position);
    void SpawnSpire(const Vector3& position);
    bool RespawnSpire(const Vector3& position);

    bool RespawnChaoZap(const Vector3& position);
    bool RespawnChaoMine(const Vector3& position);
    bool RespawnSeeker(const Vector3& position);
    bool RespawnFlash(const Vector3& position);
    bool RespawnExplosion(const Vector3& position, const Color& color, float size);
    bool RespawnHitFX(const Vector3& position, bool sound = true);

    void Activate();
    void Deactivate();
    void Restart();
};

#endif // SPAWNMASTER_H
