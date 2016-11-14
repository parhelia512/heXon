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

#define CHAOINTERVAL Random(23.0f, 100.0f)


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
    void ChaoPickup() { sinceLastChaoPickup_ = 0.0f; }

    template <class T> T* Create(bool recycle = true)
    {
        T* created{ nullptr };

        if (recycle) {
            PODVector<Node*> correctType{};
            MC->scene_->GetChildrenWithComponent<T>(correctType);
            for (Node* n : correctType) {

                if (!n->IsEnabled()) {
                    created = n->GetComponent<T>();
                    break;
                }
            }
        }
        if(!created) {

            Node* spawnedNode{ MC->scene_->CreateChild(T::GetTypeStatic().ToString()) };
            created = spawnedNode->CreateComponent<T>();
            spawnedNode->SetEnabledRecursive(false);
        }

        return created;
    }

    template <class T> int CountActive()
    {
        int count{0};
        PODVector<Node*> result{};
        MC->scene_->GetChildrenWithComponent<T>(result);

        for (Node* r : result) {

            if (r->IsEnabled()) ++count;
        }
        return count;
    }

private:
    void HandleSceneUpdate(StringHash eventType, VariantMap &eventData);

    bool spawning_;

    float razorInterval_;
    float sinceRazorSpawn_;

    float spireInterval_;
    float sinceSpireSpawn_;

    float bubbleInterval_;
    float sinceBubbleSpawn_;

    float sinceLastChaoPickup_;
    float chaoInterval_;

    Vector3 BubbleSpawnPoint();

    void Activate();
    void Deactivate();
    void Restart();
};

#endif // SPAWNMASTER_H
