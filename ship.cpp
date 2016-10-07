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

#include "inputmaster.h"
#include "arena.h"
#include "pilot.h"
#include "bullet.h"

#include "ship.h"

void Ship::RegisterObject(Context *context)
{
    context->RegisterFactory<Ship>();
}

Ship::Ship(Context* context) : Controllable(context),
    playerId_{1},
    initialHealth_{1.0f},
    health_{initialHealth_},
    weaponLevel_{0},
    bulletAmount_{1},
    bulletDamage_{0.15f},
    initialShotInterval_{0.30f},
    shotInterval_{initialShotInterval_},
    sinceLastShot_{0.0f}
{
    thrust_ = 2342.0f;
    maxSpeed_ = 23.0f;
}

void Ship::OnNodeSet(Node *node)
{
    Controllable::OnNodeSet(node);

    //Set player ID
    if (node_->GetPosition().x_ > 0) playerId_ = 2;

    model_->SetModel(MC->GetModel("KlåMk10"));
    model_->SetMaterial(0, playerId_==2 ? MC->GetMaterial("PurpleGlow") : MC->GetMaterial("GreenGlow"));
    model_->SetMaterial(1, playerId_==2 ? MC->GetMaterial("Purple") : MC->GetMaterial("Green"));

    particleEmitter_ = node_->CreateComponent<ParticleEmitter>();
    SharedPtr<ParticleEffect> particleEffect{ CACHE->GetTempResource<ParticleEffect>("Particles/Shine.xml") };
    Vector<ColorFrame> colorFrames{};
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.0f));
    colorFrames.Push(ColorFrame(playerId_==2 ? Color(0.42f, 0.0f, 0.88f, 0.05f) : Color(0.42f, 0.7f, 0.23f, 0.23f), 0.05f));
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.4f));
    particleEffect->SetColorFrames(colorFrames);
    particleEmitter_->SetEffect(particleEffect);
    particleEmitter_->SetEmitting(false);

    CreateTails();

    //Setup ship physics
    rigidBody_ = node_->CreateComponent<RigidBody>();
    rigidBody_->SetRestitution(0.666f);
    rigidBody_->SetMass(1.0f);
    rigidBody_->SetLinearFactor(Vector3::ONE - Vector3::UP);
    rigidBody_->SetLinearDamping(0.5f);
    rigidBody_->SetAngularFactor(Vector3::ZERO);
    rigidBody_->SetLinearRestThreshold(0.01f);
    rigidBody_->SetAngularRestThreshold(0.1f);

    collisionShape_ = node_->CreateComponent<CollisionShape>();
    collisionShape_->SetSphere(2.0f);

    MC->arena_->AddToAffectors(WeakPtr<Node>(node_), WeakPtr<RigidBody>(rigidBody_));
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Ship, BlinkCheck));
}

void Ship::EnterPlay()
{
    particleEmitter_->SetEmitting(true);
    model_->SetMaterial(0, playerId_==2 ? MC->GetMaterial("PurpleGlowEnvmap") : MC->GetMaterial("GreenGlowEnvmap"));
    model_->SetMaterial(1, playerId_==2 ? MC->GetMaterial("PurpleEnvmap") : MC->GetMaterial("GreenEnvmap"));
}

void Ship::CreateTails()
{
    for (int t{0}; t < 3; ++t) {
        Node* tailNode{node_->CreateChild("Tail")};
        tailNode->SetPosition(Vector3(-0.85f + 0.85f * t, t==1? 0.0f : -0.5f, t==1? -0.5f : -0.23f));
        TailGenerator* tailGen{tailNode->CreateComponent<TailGenerator>()};
        tailGen->SetDrawHorizontal(true);
        tailGen->SetDrawVertical(t==1?true:false);
        tailGen->SetTailLength(t==1? 0.05f : 0.025f);
        tailGen->SetNumTails(t==1? 13 : 7);
        tailGen->SetWidthScale(t==1? 0.5f : 0.13f);
        tailGen->SetColorForHead(playerId_==2 ? Color(1.0f, 0.666f, 0.23f) : Color(0.8f, 0.8f, 0.2f));
        tailGen->SetColorForTip(playerId_==2 ? Color(1.0f, 0.23f, 0.0f) : Color(0.23f, 0.6f, 0.0f));
        tailGens_.Push(tailGen);
    }
}

void Ship::Update(float timeStep)
{

    if (MC->GetGameState() != GS_PLAY || !node_->IsEnabled())
        return;

    //Update shield
//    Quaternion randomRotation = Quaternion(Random(360.0f),Random(360.0f),Random(360.0f));
//    shieldNode_->SetRotation(shieldNode_->GetRotation().Slerp(randomRotation, Random(1.0f)));
//    Color shieldColor = shieldMaterial_->GetShaderParameter("MatDiffColor").GetColor();
//    Color newColor = Color(shieldColor.r_ * Random(0.6f, 0.9f),
//                           shieldColor.g_ * Random(0.7f, 0.95f),
//                           shieldColor.b_ * Random(0.8f, 0.9f));
//    shieldMaterial_->SetShaderParameter("MatDiffColor", shieldColor.Lerp(newColor, Min(timeStep * 23.5f, 1.0f)));

    //Float
//        ship_.node_->SetPosition(Vector3::UP *MC->Sine(2.3f, -0.1f, 0.1f));
    //Apply movement
    Vector3 force = move_ * thrust_ * timeStep;
    if (rigidBody_->GetLinearVelocity().Length() < maxSpeed_ ||
            (rigidBody_->GetLinearVelocity().Normalized() + force.Normalized()).Length() < 1.0f) {
        rigidBody_->ApplyForce(force);
    }

    //Update rotation according to direction of the ship's movement.
    if (rigidBody_->GetLinearVelocity().Length() > 0.1f)
        node_->LookAt(node_->GetPosition()+rigidBody_->GetLinearVelocity());

    //Update tails
    for (int t = 0; t < 3; t++)
    {
        float velocityToScale = Clamp(0.23f * rigidBody_->GetLinearVelocity().Length(), 0.0f, 1.0f);
        TailGenerator* tailGen{tailGens_[t]};
        if (tailGen) {
            tailGen->SetTailLength(t==1? velocityToScale * 0.1f : velocityToScale * 0.075f);
            //            tailGen->SetNumTails(t==1? (int)(velocityToScale * 23) : (int)(velocityToScale * 16));
            tailGen->SetWidthScale(t==1? velocityToScale * 0.666f : velocityToScale * 0.23f);
        }
    }

    //Shooting
    sinceLastShot_ += timeStep;
    if (aim_.Length()) {
        if (sinceLastShot_ > shotInterval_)
        {
            Shoot(aim_);
        }
    }
}

void Ship::Shoot(Vector3 aim)
{
    for (int i = 0; i < bulletAmount_; i++) {
        float angle = 0.0f;
        switch (i) {
        case 0: angle = (bulletAmount_ == 2 || bulletAmount_ == 3) ?
                        -5.0f : 0.0f;
            break;
        case 1: angle = bulletAmount_ < 4 ?
                        5.0f : 7.5f;
            break;
        case 2: angle = bulletAmount_ < 5 ?
                        180.0f : 175.0f;
            break;
        case 3: angle = -7.5f;
            break;
        case 4: angle = 185.0f;
            break;
        default: break;
        }
        Vector3 direction = Quaternion(angle, Vector3::UP) * aim;
        FireBullet(direction);
    }
    sinceLastShot_ = 0.0f;
    //Create a single muzzle flash
    if (bulletAmount_ > 0){
//        MoveMuzzle();
        PlaySample(MC->GetSample("Shot"), 0.17f);
    }
}


void Ship::FireBullet(Vector3 direction){
    SharedPtr<Bullet> bullet{};
    if (bullets_.Size() > 0){
        for (SharedPtr<Bullet> b : bullets_){
            if (!b->IsEnabled()){
                bullet = b;
            }
        }
    }
    if (bullet == nullptr){
        bullet = MC->scene_->CreateChild("Bullet")->CreateComponent<Bullet>();
        bullets_.Push(bullet);
    }
    bullet->Set(node_->GetPosition() + direction + Vector3::DOWN*0.42f);
    bullet->node_->LookAt(bullet->node_->GetPosition() + direction * 5.0f);
    bullet->rigidBody_->ApplyForce(direction * (1500.0f + 23.0f * weaponLevel_));
    bullet->damage_ = 0.15f + 0.00666f * weaponLevel_;
}
//void Player::MoveMuzzle()
//{
//    if (muzzle_ == nullptr)
//        muzzle_ = new Muzzle(playerID_);
//    muzzle_->Set(node_->GetPosition() + Vector3::DOWN * 0.42f);
//}