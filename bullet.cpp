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

#include "bullet.h"

Bullet::Bullet(Context *context, MasterControl *masterControl, int playerID):
    SceneObject(context, masterControl),
    playerID_{playerID},
    lifeTime_{1.0f},
    damage_{0.0f}
{
    rootNode_->SetName("Bullet");
    rootNode_->SetEnabled(false);
    rootNode_->SetScale(Vector3(1.0f+damage_, 1.0f+damage_, 0.1f));
    model_ = rootNode_->CreateComponent<StaticModel>();
    model_->SetModel(masterControl_->cache_->GetTempResource<Model>("Models/Bullet.mdl"));
    model_->SetMaterial(playerID_ == 2
                        ? masterControl_->cache_->GetResource<Material>("Materials/PurpleBullet.xml")
                        : masterControl_->cache_->GetResource<Material>("Materials/GreenBullet.xml"));

    rigidBody_ = rootNode_->CreateComponent<RigidBody>();
    rigidBody_->SetMass(0.5f);
    rigidBody_->SetLinearFactor(Vector3::ONE - Vector3::UP);
    rigidBody_->SetFriction(0.0f);

    Light* light = rootNode_->CreateComponent<Light>();
    light->SetRange(6.66f);
    light->SetColor( playerID_ == 2 ? Color(1.0f + damage_, 0.6f, 0.2f) : Color(0.6f, 1.0f+damage_, 0.2f));
}

void Bullet::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

    age_ += timeStep;
    rootNode_->SetScale(Vector3(Max(1.75f - 10.0f*age_, 1.0f+damage_),
                                Max(1.75f - 10.0f*age_, 1.0f+damage_),
                                Min(Min(35.0f*age_, 2.0f), Max(2.0f-timeSinceHit_*42.0f, 0.1f))
                                ));
    if (age_ > lifeTime_) {
        Disable();
    }

    if (timeStep > 0.0f && !fading_) HitCheck(timeStep);
}

void Bullet::Set(const Vector3 position)
{
    age_ = 0.0f;
    timeSinceHit_ = 0.0f;
    fading_ = false;

    rigidBody_->SetLinearVelocity(Vector3::ZERO);
    rigidBody_->ResetForces();
    SceneObject::Set(position);
    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Bullet, HandleSceneUpdate));
}

void Bullet::Disable()
{
    fading_ = true;
    SceneObject::Disable();
    UnsubscribeFromEvent(E_SCENEUPDATE);
}

void Bullet::HitCheck(float timeStep) {
    if (!fading_) {
        PODVector<PhysicsRaycastResult> hitResults{};
        Ray bulletRay(rootNode_->GetPosition() - rigidBody_->GetLinearVelocity()*timeStep, rootNode_->GetDirection());
        if (masterControl_->PhysicsRayCast(hitResults, bulletRay, 2.3f * rigidBody_->GetLinearVelocity().Length()*timeStep, M_MAX_UNSIGNED)){
            for (PhysicsRaycastResult h : hitResults){
                if (!h.body_->IsTrigger()){// && h.body_->GetNode()->GetNameHash() != N_PLAYER){
                    h.body_->ApplyImpulse(rigidBody_->GetLinearVelocity()*0.05f);
                    masterControl_->spawnMaster_->SpawnHitFX(h.position_, playerID_);
                    //Deal damage
                    unsigned hitID = h.body_->GetNode()->GetID();
                    if(masterControl_->spawnMaster_->spires_.Keys().Contains(hitID)){
                        masterControl_->spawnMaster_->spires_[hitID]->Hit(damage_, playerID_);
                    }
                    else if(masterControl_->spawnMaster_->razors_.Keys().Contains(hitID)){
                        masterControl_->spawnMaster_->razors_[hitID]->Hit(damage_, playerID_);
                    }
                    else if(masterControl_->spawnMaster_->chaoMines_.Keys().Contains(hitID)){
                        masterControl_->spawnMaster_->chaoMines_[hitID]->Hit(damage_, playerID_);
                    }
                    Disable();
                }
            }
        }
    }
}
