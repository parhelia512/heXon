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

#include "chaozap.h"

#include "player.h"
#include "spawnmaster.h"

ChaoZap::ChaoZap(MasterControl *masterControl):
    SceneObject(masterControl),
    playerID_{0},
    size_{5.0f}
{
    rootNode_->SetName("ChaoZap");
    rootNode_->SetEnabled(false);

    chaoModel_ = rootNode_->CreateComponent<StaticModel>();
    chaoModel_->SetModel(masterControl_->cache_->GetResource<Model>("Models/ChaoFlash.mdl"));
    chaoMaterial_ = masterControl_->cache_->GetTempResource<Material>("Materials/ChaoFlash.xml");
    chaoModel_->SetMaterial(chaoMaterial_);

    samples_.Push(masterControl_->cache_->GetResource<Sound>("Samples/Mine1.ogg"));
    samples_.Push(masterControl_->cache_->GetResource<Sound>("Samples/Mine2.ogg"));
    samples_.Push(masterControl_->cache_->GetResource<Sound>("Samples/Mine3.ogg"));
    samples_.Push(masterControl_->cache_->GetResource<Sound>("Samples/Mine4.ogg"));
    samples_.Push(masterControl_->cache_->GetResource<Sound>("Samples/Mine5.ogg"));

    rigidBody_ = rootNode_->CreateComponent<RigidBody>();
}

void ChaoZap::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    if (!IsPlayingSound()) Disable();
    if (!IsEnabled()) return;

    float timeStep{eventData[Update::P_TIMESTEP].GetFloat()};

    Color chaoColor{chaoMaterial_->GetShaderParameter("MatDiffColor").GetColor()};
    rigidBody_->SetMass(chaoColor.a_);
    Color newDiffColor{chaoColor.r_ * Random(0.1f , 1.23f),
                       chaoColor.g_ * Random(0.23f, 0.9f),
                       chaoColor.b_ * Random(0.16f, 0.5f),
                       chaoColor.a_ * Random(0.42f, 0.9f)};
    chaoMaterial_->SetShaderParameter("MatDiffColor", chaoColor.Lerp(newDiffColor, Clamp(23.0f*timeStep, 0.0f, 1.0f)));
    Color newSpecColor{Random(0.3f, 1.5f),
                       Random(0.5f, 1.8f),
                       Random(0.4f, 1.4f),
                       Random(4.0f, 64.0f)};
    chaoMaterial_->SetShaderParameter("MatSpecColor", newSpecColor);
    rootNode_->SetRotation(Quaternion(Random(360.0f), Random(360.0f), Random(360.0f)));
}

void ChaoZap::Set(const Vector3 position, int playerID)
{
    playerID_ = playerID;
    SceneObject::Set(position);
    rootNode_->SetScale(size_);
    rigidBody_->SetMass(size_ * 0.5f);
    PlaySample(samples_[ Random(static_cast<int>(samples_.Size())) ], 0.75f);
    PODVector<RigidBody* > hitResults{};
    rootNode_->SetEnabled(true);
    chaoMaterial_->SetShaderParameter("MatDiffColor", Color(0.1f, 0.5f, 0.2f, 0.5f));

    if (masterControl_->PhysicsSphereCast(hitResults,rootNode_->GetPosition(), size_, M_MAX_UNSIGNED)) {
        for (RigidBody* r : hitResults) {
            unsigned hitID{r->GetNode()->GetID()};

            if(masterControl_->spawnMaster_->spires_.Contains(hitID)) {
                WeakPtr<Spire> spire{masterControl_->spawnMaster_->spires_[hitID]};
                spire->Hit(spire->GetHealth(), 1);
                masterControl_->GetPlayer(playerID)->AddScore(Random(23, 42));
            }
            else if(masterControl_->spawnMaster_->razors_.Contains(hitID)) {
                WeakPtr<Razor> razor{masterControl_->spawnMaster_->razors_[hitID]};
                razor->Hit(razor->GetHealth(), 1);
                masterControl_->GetPlayer(playerID)->AddScore(Random(5, 23));
            }
            else if(masterControl_->spawnMaster_->seekers_.Contains(hitID)) {
                WeakPtr<Seeker> seeker{masterControl_->spawnMaster_->seekers_[hitID]};
                seeker->Disable();
                masterControl_->GetPlayer(playerID)->AddScore(Random(2, 3));
            }
        }
    }

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(ChaoZap, HandleSceneUpdate));
}

void ChaoZap::Disable()
{
    UnsubscribeFromEvent(E_SCENEUPDATE);
    SceneObject::Disable();
}
