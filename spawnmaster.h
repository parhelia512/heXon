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

#ifndef SPAWNMASTER_H
#define SPAWNMASTER_H

#include <algorithm>

#include <Urho3D/Urho3D.h>

#include "mastercontrol.h"
#include "razor.h"
#include "spire.h"
#include "chaozap.h"
#include "chaomine.h"
#include "seeker.h"
#include "flash.h"
#include "explosion.h"
#include "bubble.h"
#include "line.h"


class SpawnMaster : public Object
{
    friend class MasterControl;
    URHO3D_OBJECT(SpawnMaster, Object);
public:
    SpawnMaster(Context* context);

    HashMap<unsigned, SharedPtr<Razor> > razors_;
    HashMap<unsigned, SharedPtr<Spire> > spires_;
    HashMap<unsigned, SharedPtr<Seeker> > seekers_;
    HashMap<unsigned, SharedPtr<ChaoMine> > chaoMines_;
    Vector<SharedPtr<HitFX> > hitFXs_;
    Vector<SharedPtr<ChaoZap> > chaoZaps_;
    Vector<SharedPtr<Flash> > flashes_;
    Vector<SharedPtr<Explosion> > explosions_;
    Vector<SharedPtr<Bubble> > bubbles_;
    Vector<SharedPtr<Line> > lines_;

    void Clear();
    Vector3 SpawnPoint();

    int CountActiveRazors();
    int CountActiveSpires();
    int CountActiveLines();

    void SpawnChaoZap(const Vector3 &position, int playerID);
    void SpawnChaoMine(const Vector3 &position, int playerID);
    void SpawnSeeker(const Vector3& position);
    void SpawnHitFX(const Vector3& position, int playerID, bool sound = true);
    void SpawnFlash(const Vector3& position, bool big);
    void SpawnBubble(const Vector3& position);
    bool SpawnExplosion(const Vector3& position, const Color &color, float size, int playerID);
    void SpawnLine(int playerID_);

private:
    void HandleSceneUpdate(StringHash eventType, VariantMap &eventData);

    bool spawning_;

    float razorInterval_;
    float sinceRazorSpawn_;

    float spireInterval_;
    float sinceSpireSpawn_;

    float bubbleInterval_;
    float sinceBubbleSpawn_;

    void SpawnRazor(const Vector3& position);
    bool RespawnRazor(const Vector3& position);
    void SpawnSpire(const Vector3& position);
    bool RespawnSpire(const Vector3& position);

    bool RespawnChaoZap(const Vector3& position, int playerID);
    bool RespawnChaoMine(const Vector3& position, int playerID);
    bool RespawnSeeker(const Vector3& position);
    bool RespawnFlash(const Vector3& position, bool big);
    bool RespawnBubble(const Vector3& position);
    bool RespawnLine(int playerID);
    bool RespawnExplosion(const Vector3& position, const Color& color, float size, int playerID);
    bool RespawnHitFX(const Vector3& position, int playerID, bool sound = true);

    Vector3 BubbleSpawnPoint();
    Vector3 LineSpawnPoint(int playerID);

    void Activate();
    void Deactivate();
    void Restart();
};

#endif // SPAWNMASTER_H

///Template template
//    template <class T>
//    T* Create()
//    {
//        static_assert(std::is_base_of<T,SceneObject>(),"Must be SceneObject");
//
//        auto sceneObject = new T();
//        census_[T].Push(sceneObject);
//        return sceneObject;
//    }
//
//    template <class T>
//    T* Spawn(const Vector3& position)
//    {
//        WeakPtr<T> sceneObject = nullptr;
//        for (unsigned t = 0; t < census_[T].Size(); ++t){
//            if (!census_[T][t]->IsEnabled()){
//                sceneObject = census_[T][t];
//            }
//        }
//        if (sceneObject == nullptr) sceneObject = Create<T>();
//
//        sceneObject->Set(position);
//        return sceneObject;
//    }
