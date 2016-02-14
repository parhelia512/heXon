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

#include "mastercontrol.h"
#include "player.h"
#include "seeker.h"
#include "spire.h"
#include "spawnmaster.h"

Spire::Spire(Context *context, MasterControl *masterControl):
    Enemy(context, masterControl),
    initialShotInterval_{5.0f},
    shotInterval_{initialShotInterval_},
    sinceLastShot_{0.0f}
{
    rootNode_->SetName("Spire");

    health_ = initialHealth_ = 5.0f;
    worth_ = 10;
    rigidBody_->SetMass(3.0f);
    rigidBody_->SetLinearFactor(Vector3::ZERO);

    SharedPtr<Material> black = masterControl_->cache_->GetTempResource<Material>("Resources/Materials/Spire.xml");

    topNode_ = rootNode_->CreateChild();
    topModel_ = topNode_->CreateComponent<StaticModel>();
    topModel_->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/SpireTop.mdl"));
    topModel_->SetMaterial(black);

    bottomNode_ = rootNode_->CreateChild();
    bottomModel_ = bottomNode_->CreateComponent<StaticModel>();
    bottomModel_->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/SpireBottom.mdl"));
    bottomModel_->SetMaterial(black);

    SubscribeToEvent(E_SCENEPOSTUPDATE, URHO3D_HANDLER(Spire, HandleSpireUpdate));
}

void Spire::HandleSpireUpdate(StringHash eventType, VariantMap &eventData)
{
    if (!rootNode_->IsEnabled()) return;

    double timeStep = eventData[ScenePostUpdate::P_TIMESTEP].GetFloat();

    //Pulse
    topModel_->GetMaterial()->SetShaderParameter("MatEmissiveColor", GetGlowColor());
    //Spin
    topNode_->Rotate(Quaternion(0.0f, timeStep*(50.0f+panic_*300.0f), 0.0f));
    bottomNode_->Rotate(Quaternion(0.0f, timeStep*-(50.0f+panic_*300.0f), 0.0f));

    if (masterControl_->GetGameState() == GS_PLAY && IsEmerged()){
        //Shoot
        sinceLastShot_ += timeStep;
        if (sinceLastShot_ > shotInterval_){
            sinceLastShot_ = 0.0f;
            masterControl_->spawnMaster_->SpawnSeeker(rootNode_->GetPosition());
        }
    }
}

void Spire::Hit(float damage, int ownerID)
{
    Enemy::Hit(damage, ownerID);
    shotInterval_ = initialShotInterval_ - 3.0f * panic_;
}

void Spire::Set(Vector3 position)
{
    shotInterval_ = initialShotInterval_;
    Enemy::Set(position);
    SubscribeToEvent(E_SCENEPOSTUPDATE, URHO3D_HANDLER(Spire, HandleSpireUpdate));
}
