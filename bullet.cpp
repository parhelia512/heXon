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

void Bullet::RegisterObject(Context *context)
{
    context->RegisterFactory<Bullet>();
}

Bullet::Bullet(Context* context):
    SceneObject(context),
    colorSet_{1},
    lifeTime_{1.0f},
    damage_{0.0f}
{
}

void Bullet::OnNodeSet(Node *node)
{
    SceneObject::OnNodeSet(node);

    node_->SetName("Bullet");
    node_->SetEnabled(false);
    node_->SetScale(Vector3(1.0f + damage_, 1.0f + damage_, 0.1f));
    model_ = node_->CreateComponent<StaticModel>();
    model_->SetModel(MC->GetModel("Bullet"));

    rigidBody_ = node_->CreateComponent<RigidBody>();
    rigidBody_->SetMass(0.5f);
    rigidBody_->SetLinearFactor(Vector3::ONE - Vector3::UP);
    rigidBody_->SetFriction(0.0f);

    Light* light = node_->CreateComponent<Light>();
    light->SetBrightness(4.2f);
    light->SetRange(5.5f);
}

void Bullet::Update(float timeStep)
{
    age_ += timeStep;
    node_->SetScale(Vector3(Max(1.75f - 10.0f*age_, 1.0f+damage_),
                                Max(1.75f - 10.0f*age_, 1.0f+damage_),
                                Min(Min(35.0f*age_, 2.0f), Max(2.0f-timeSinceHit_*42.0f, 0.1f))
                                ));
    if (age_ > lifeTime_) {
        Disable();
    }

    if (timeStep > 0.0f && !fading_) HitCheck(timeStep);
}

void Bullet::Set(Vector3 position, int colorSet, Vector3 direction, Vector3 force, float damage)
{
    age_ = 0.0f;
    timeSinceHit_ = 0.0f;
    fading_ = false;
    colorSet_ = colorSet;
    damage_ = damage;

    Light* light{ node_->GetComponent<Light>() };
    light->SetColor( MC->colorSets_[colorSet].colors_.first_ * (1.0f + damage_) );// colorSet_ == 2 ? Color(1.0f + damage_, 0.6f, 0.2f) : Color(0.6f, 1.0f+damage_, 0.2f));
    light->SetBrightness(3.4f + damage_ * 2.3f);
    model_->SetMaterial(MC->colorSets_[colorSet].bulletMaterial_);

    rigidBody_->SetLinearVelocity(Vector3::ZERO);
    rigidBody_->ResetForces();
    SceneObject::Set(position);
    rigidBody_->ApplyForce(force);
    node_->LookAt(node_->GetPosition() + direction);
}

void Bullet::Disable()
{
    fading_ = true;
    SceneObject::Disable();
    UnsubscribeFromEvent(E_SCENEUPDATE);
}

void Bullet::HitCheck(float timeStep)
{
    if (!fading_) {
        PODVector<PhysicsRaycastResult> hitResults{};
        Ray bulletRay(node_->GetPosition() - rigidBody_->GetLinearVelocity()*timeStep, node_->GetDirection());
        if (MC->PhysicsRayCast(hitResults, bulletRay, 2.3f * rigidBody_->GetLinearVelocity().Length() * timeStep, M_MAX_UNSIGNED)) {
            for (PhysicsRaycastResult h : hitResults) {
                if (!h.body_->IsTrigger()) {
                    //Add effect
                    h.body_->ApplyImpulse(rigidBody_->GetLinearVelocity() * 0.05f);
                    HitFX* hitFx{ GetSubsystem<SpawnMaster>()->Create<HitFX>() };
                    hitFx->Set(h.position_, colorSet_, true);

                    //Deal damage
                    for (Component* c : h.body_->GetNode()->GetComponents()){
                        if (c->IsInstanceOf<Enemy>()){
                            Enemy* e{ static_cast<Enemy*>(c) };
                                e->Hit(damage_, colorSet_);
                            }
                    }
                    Disable();
                }
            }
        }
    }
}
