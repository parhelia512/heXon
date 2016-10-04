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

void ChaoZap::RegisterObject(Context *context)
{
    context->RegisterFactory<ChaoZap>();
}

ChaoZap::ChaoZap(Context* context):
    SceneObject(context),
    playerID_{0},
    size_{5.0f}
{
}

void ChaoZap::OnNodeSet(Node *node)
{
    SceneObject::OnNodeSet(node);

    node_->SetName("ChaoZap");
    node_->SetEnabled(false);

    chaoModel_ = node_->CreateComponent<StaticModel>();
    chaoModel_->SetModel(MC->GetModel("ChaoFlash"));
    chaoMaterial_ = MC->GetMaterial("ChaoFlash")->Clone();
    chaoModel_->SetMaterial(chaoMaterial_);

    samples_.Push(MC->GetSample("Mine1"));
    samples_.Push(MC->GetSample("Mine2"));
    samples_.Push(MC->GetSample("Mine3"));
    samples_.Push(MC->GetSample("Mine4"));
    samples_.Push(MC->GetSample("Mine5"));

    rigidBody_ = node_->CreateComponent<RigidBody>();

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
    node_->SetRotation(Quaternion(Random(360.0f), Random(360.0f), Random(360.0f)));
}

void ChaoZap::Set(const Vector3 position, int playerID)
{
    playerID_ = playerID;
    SceneObject::Set(position);
    node_->SetScale(size_);
    rigidBody_->SetMass(size_ * 0.5f);
    PlaySample(samples_[ Random(static_cast<int>(samples_.Size())) ], 0.75f);
    PODVector<RigidBody* > hitResults{};
    node_->SetEnabled(true);
    chaoMaterial_->SetShaderParameter("MatDiffColor", Color(0.1f, 0.5f, 0.2f, 0.5f));

    if (MC->PhysicsSphereCast(hitResults,node_->GetPosition(), size_, M_MAX_UNSIGNED)) {
        for (RigidBody* r : hitResults) {
            unsigned hitID{r->GetNode()->GetID()};

            if(GetSubsystem<SpawnMaster>()->spires_.Contains(hitID)) {
                WeakPtr<Spire> spire{GetSubsystem<SpawnMaster>()->spires_[hitID]};
                spire->Hit(spire->GetHealth(), 1);
//                MC->GetPlayer(playerID)->AddScore(Random(23, 42));
            }
            else if(GetSubsystem<SpawnMaster>()->razors_.Contains(hitID)) {
                WeakPtr<Razor> razor{GetSubsystem<SpawnMaster>()->razors_[hitID]};
                razor->Hit(razor->GetHealth(), 1);
//                MC->GetPlayer(playerID)->AddScore(Random(5, 23));
            }
            else if(GetSubsystem<SpawnMaster>()->seekers_.Contains(hitID)) {
                WeakPtr<Seeker> seeker{GetSubsystem<SpawnMaster>()->seekers_[hitID]};
                seeker->Disable();
//                MC->GetPlayer(playerID)->AddScore(Random(2, 3));
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
