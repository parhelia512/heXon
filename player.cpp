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

#include "player.h"

#include "TailGenerator.h"

#include "mastercontrol.h"
#include "hexocam.h"
#include "inputmaster.h"
#include "spawnmaster.h"
#include "arena.h"
#include "bullet.h"
#include "muzzle.h"
#include "chaoflash.h"
#include "explosion.h"
#include "heart.h"
#include "apple.h"
#include "chaoball.h"
#include "pilot.h"
#include "splatterpillar.h"
#include "door.h"
#include "ship.h"
#include "phaser.h"
#include "gui3d.h"

Player::Player(int playerId, Context* context): Object(context),
    playerId_{playerId},
    autoPilot_{playerId_ == 2 && !GetSubsystem<Input>()->GetJoystickByIndex(playerId_-1)},
//    autoPilot_{false},
//    autoPilot_{true},
//    autoMove_{Vector3::ZERO},
//    autoFire_{Vector3::ZERO},
    alive_{false},
//    appleCount_{0},
//    heartCount_{0},

    score_{0},
    flightScore_{0},
//    toCount_{0},
    multiplier_{1}
{
    Node* guiNode{ MC->scene_->CreateChild("GUI3D") };
    gui3d_ = guiNode->CreateComponent<GUI3D>();
    gui3d_->Initialize(playerId_);
}

void Player::Die()
{
    alive_ = false;
}
void Player::Respawn()
{
    ResetScore();
    alive_ = true;
}

void Player::SetScore(int points)
{
    score_ = points;
    gui3d_->SetScore(score_);

}
void Player::ResetScore()
{
    SetScore(0);
}

void Player::EnterLobby()
{

    PODVector<Node*> pilotNodes{};
    MC->scene_->GetChildrenWithComponent<Pilot>(pilotNodes);
    for (Node* n : pilotNodes){
        Pilot* pilot{ n->GetComponent<Pilot>() };
        if (playerId_ == pilot->GetPlayerId()){
            GetSubsystem<InputMaster>()->SetPlayerControl(playerId_, pilot);
            if (!alive_){
                pilot->Revive();
            } else pilot->EnterLobbyFromShip();
        }
    }

    gui3d_->EnterLobby();
}
void Player::EnterPlay()
{
    gui3d_->EnterPlay();
}

void Player::AddScore(int points)
{
    if (!alive_) return;

    points *= static_cast<int>(pow(2.0, static_cast<double>(multiplier_-1)));
    SetScore(GetScore()+points);
    //Check for multiplier increase
    for (int i{0}; i < 10; ++i){
        unsigned tenPow{static_cast<unsigned>(pow(10, i))};
        if (flightScore_ < tenPow && (flightScore_ + points) > tenPow){
            ++multiplier_;
            GetSubsystem<InputMaster>()->GetControllableByPlayer(playerId_)->
                    PlaySample(MC->GetSample("MultiX"), 0.42f);
            MC->arena_->FlashX(playerId_);
            break;
        }
    }
    flightScore_ += points;
//    toCount_ += points;
}

Vector3 Player::GetPosition()
{
    return GetSubsystem<InputMaster>()->GetControllableByPlayer(playerId_)->GetPosition();
}

Ship* Player::GetShip()
{
    Controllable* controllable{ GetSubsystem<InputMaster>()->GetControllableByPlayer(playerId_) };
    if (controllable->IsInstanceOf<Ship>()){
        Ship* ship{ static_cast<Ship*>(controllable) };
        return ship;
    }
    return nullptr;
}

/*
void Player::OnNodeSet(Node *node)
{
    SceneObject::OnNodeSet(node);

    node_->SetName("Player");
    CreateGUI();

    //Setup pilot
//    if (!autoPilot_){
//        pilot_ = new Pilot(node_.Get(),
//                           "Resources/.Pilot"+std::to_string(playerID_)+".lkp",
//                           score_);
//    } else {
//        pilot_ = new Pilot(node_.Get());
//    }

//    if (score_ != 0) {
//        alive_ = true;
//        SetScore(score_);
//    }

    //Setup shield
    shieldNode_ = node_->CreateChild("Shield");
    shieldModel_ = shieldNode_->CreateComponent<StaticModel>();
    shieldModel_->SetModel(MC->GetModel("Shield"));
    shieldMaterial_ = MC->GetMaterial("Shield")->Clone();
    shieldModel_->SetMaterial(shieldMaterial_);

    //Setup ChaoFlash
//    chaoFlash_ = new ChaoFlash(playerID_);

    //Setup player audio
    shot_s = MC->GetSample("Shot");
    shot_s->SetLooped(false);
    shieldHit_s = MC->GetSample("ShieldHit");
    shieldHit_s->SetLooped(false);
    death_s = MC->GetSample("Death");
    death_s->SetLooped(false);
    for (int p{1}; p <= 4; ++p){
        pickup_s.Push(SharedPtr<Sound>(MC->GetSample("Pickup"+String{p})));
        pickup_s[p-1]->SetLooped(false);
    }

    powerup_s = MC->GetSample("Powerup");
    powerup_s->SetLooped(false);
    multix_s= MC->GetSample("MultiX");
    multix_s->SetLooped(false);
    chaoball_s = MC->GetSample("Chaos");
    chaoball_s->SetLooped(false);
    for (int s{1}; s < 5; ++s){
        seekerHits_s.Push(SharedPtr<Sound>(MC->GetSample("SeekerHit"+String(s))));
    }
    //Some extra sources for the players
    for (int i{0}; i < 5; ++i){
        SharedPtr<SoundSource> extraSampleSource = SharedPtr<SoundSource>(node_->CreateComponent<SoundSource>());
        extraSampleSource->SetSoundType(SOUND_EFFECT);
        sampleSources_.Push(extraSampleSource);
    }
    Node* deathSourceNode = MC->scene_->CreateChild("DeathSound");
    deathSource_ = deathSourceNode->CreateComponent<SoundSource>();
    deathSource_->SetSoundType(SOUND_EFFECT);
    deathSource_->SetGain(2.3f);

    //Subscribe to events
    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Player, HandleSceneUpdate));

//    for (int b{0}; b < 64; ++b){
//        Bullet* bullet{new Bullet(playerID_)};
//        bullets_.Push(SharedPtr<Bullet>(bullet));
//    }
}

void Player::SavePilot()
{
    if (IsHuman())
        pilot_->Save(playerID_, score_);
}

void Player::CreateGUI()
{

}

void Player::KillPilot()
{
//    if (MC->highestScore_ < score_ && IsHuman()){
//        pilot_->Save(0, score_);
//        MC->LoadHighest();
//    }
    alive_ = false;
    EnterLobby();
}

void Player::CountScore()
{
    int threshold{1024};
    int lines{GetSubsystem<SpawnMaster>()->CountActiveLines()};
    while (toCount_ > 0 && lines < threshold){
        GetSubsystem<SpawnMaster>()->SpawnLine(playerID_);
        --toCount_;
        ++lines;
    }
}

void Player::Eject()
{
//    new Phaser(ship_.model_->GetModel(), GetPosition(),
//               rigidBody_->GetLinearVelocity() + node_->GetDirection() * 10e-5);
    Disable();
}

void Player::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{ (void)eventType;

    //Count the score
    CountScore();

    //Take the frame time step, which is stored as a double
    float timeStep{eventData[Update::P_TIMESTEP].GetFloat()};
    //Pulse and spin the counters' apples and hearts
    UpdateGUI(timeStep);

    //Only handle input when player is active
    if (!node_->IsEnabled()) return;

    //Movement values
    Vector3 move{};
    Vector3 moveJoy{};
    Vector3 moveKey{};
    float thrust = pilotMode_ ? 256.0f : 2342.0f;
    float maxSpeed = pilotMode_? 1.23f + 0.5f * pilot_->pilotColors_[static_cast<int>(PC_SHOES)].r_ : 23.0f;
    //Firing values
    Vector3 fire{};
    Vector3 fireJoy{};
    Vector3 fireKey{};

    //Read input
    if (JOY){
        moveJoy = Vector3::RIGHT * JOY->GetAxisPosition(0) +
                  Vector3::BACK * JOY->GetAxisPosition(1);
        fireJoy = Vector3::RIGHT * JOY->GetAxisPosition(2) +
                  Vector3::BACK * JOY->GetAxisPosition(3);
    } else {
        if (playerID_ == 1 || INPUT->GetJoystickByIndex(0)) {

            moveKey = Vector3::LEFT * INPUT->GetKeyDown(KEY_A) +
                      Vector3::RIGHT * INPUT->GetKeyDown(KEY_D) +
                      Vector3::FORWARD * INPUT->GetKeyDown(KEY_W) +
                      Vector3::BACK * INPUT->GetKeyDown(KEY_S);
            fireKey = Vector3::LEFT * (INPUT->GetKeyDown(KEY_J) || INPUT->GetKeyDown(KEY_KP_4)) +
                      Vector3::RIGHT * (INPUT->GetKeyDown(KEY_L) || INPUT->GetKeyDown(KEY_KP_6)) +
                      Vector3::FORWARD * (INPUT->GetKeyDown(KEY_I) || INPUT->GetKeyDown(KEY_KP_8)) +
                      Vector3::BACK * (INPUT->GetKeyDown(KEY_K) || INPUT->GetKeyDown(KEY_KP_2) || INPUT->GetKeyDown(KEY_KP_5)) +
                      Quaternion(45.0f, Vector3::UP) * Vector3::LEFT * INPUT->GetKeyDown(KEY_KP_7) +
                      Quaternion(45.0f, Vector3::UP) * Vector3::RIGHT * INPUT->GetKeyDown(KEY_KP_3) +
                      Quaternion(45.0f, Vector3::UP) * Vector3::FORWARD * INPUT->GetKeyDown(KEY_KP_9) +
                      Quaternion(45.0f, Vector3::UP) * Vector3::BACK * INPUT->GetKeyDown(KEY_KP_1);
        }
    }

    //Pick most significant input
    moveJoy.Length() > moveKey.Length() ? move = moveJoy : move = moveKey;
    fireJoy.Length() > fireKey.Length() ? fire = fireJoy : fire = fireKey;

    if (autoPilot_){
        Think();
        move = autoMove_;
        fire = autoFire_;
    }

    //Restrict move vector length
    if (move.Length() > 1.0f)
        move /= move.Length();
    //Deadzones
    else if (move.Length() < 0.1f)
        move *= 0.0f;

    if (fire.Length() < 0.1f)
        fire *= 0.0f;
    else
        fire.Normalize();

    //Flatten input
    Vector3 xzPlane{Vector3::ONE - Vector3::UP};
    LucKey::Scale(move, xzPlane);
    LucKey::Scale(fire, xzPlane);

    //When in pilot mode
    if (pilotMode_){
        //Apply movement
        Vector3 force = move * thrust * timeStep;
        if ( rigidBody_->GetLinearVelocity().Length() < maxSpeed
         || (rigidBody_->GetLinearVelocity().Normalized() + force.Normalized()).Length() < 1.4142f )
        {
            rigidBody_->ApplyForce(force);
        }

        //Update rotation according to direction of the player's movement.
        Vector3 velocity = rigidBody_->GetLinearVelocity();
        Vector3 lookDirection = velocity + 2.0f*fire;
        Quaternion rotation = node_->GetWorldRotation();
        Quaternion aimRotation = rotation;
        aimRotation.FromLookRotation(lookDirection);
        node_->SetRotation(rotation.Slerp(aimRotation, 7.0f * timeStep * velocity.Length()));

        //Update animation
        if (velocity.Length() > 0.1f){
            pilot_->animCtrl_->PlayExclusive("Models/WalkRelax.ani", 0, true, 0.15f);
            pilot_->animCtrl_->SetSpeed("Models/WalkRelax.ani", velocity.Length()*2.3f);
            pilot_->animCtrl_->SetStartBone("Models/WalkRelax.ani", "MasterBone");
        }
        else {
            pilot_->animCtrl_->PlayExclusive("Models/IdleRelax.ani", 0, true, 0.15f);
            pilot_->animCtrl_->SetStartBone("Models/IdleRelax.ani", "MasterBone");
        }
    // When in ship mode
    } else {


    }
}

void Player::UpdateGUI(float timeStep)
{


}



void Player::PickupChaoBall()
{
    bool swap{chaoFlash_->Set(MC->chaoBall_->GetPosition()) > 1};
    if (swap){
        Vector3 tempPos{node_->GetPosition()};
        node_->SetPosition(MC->GetPlayer(playerID_, true)->GetPosition());
        MC->GetPlayer(playerID_, true)->SetPosition(tempPos);
    } else{
        node_->SetPosition(Quaternion(Random(360.0f), Vector3::UP) * (Vector3::FORWARD * Random(5.0f)) +
                               MC->chaoBall_->GetPosition() * Vector3(1.0f, 0.0f, 1.0f));
    }
    PlaySample(chaoball_s, 0.8f);
}
void Player::SetPosition(Vector3 pos)
{
    node_->SetPosition(pos);
}


void Player::EnterPlay()
{
    node_->SetRotation(Quaternion(playerID_==2 ? -90.0f : 90.0f, Vector3::UP));
    rigidBody_->ResetForces();
    rigidBody_->SetLinearVelocity(Vector3::ZERO);
    shieldMaterial_->SetShaderParameter("MatDiffColor", Color::BLACK);

    SetHealth(initialHealth_);
    Pickup(PT_RESET);
    flightScore_ = 0;
    multiplier_ = 1;
    weaponLevel_ = 0;
    bulletAmount_ = 1;
    shotInterval_ = initialShotInterval_;
    Set(Vector3(playerID_==2 ? 4.2f : -4.2f, 0.0f, 0.0f));
    SetPilotMode(false);

}
void Player::EnterLobby()
{
    bool enterThroughDoor{true};// !alive_ || MC->GetPreviousGameState() == GS_INTRO};

    StopAllSound();
    chaoFlash_->Disable();

    if (!IsAlive()){
        alive_ = true;
//        pilot_->Randomize(autoPilot_);
        ResetScore();
    }
    SetPilotMode(true);

    if (enterThroughDoor){
        node_->SetPosition(Vector3(playerID_==2 ? 2.23f : -2.23f, 0.0f, 5.5f));
        rigidBody_->SetLinearVelocity(Vector3::BACK * 2.3f);
    } else {
        rigidBody_->SetLinearVelocity(((playerID_ == 2) ? Vector3::LEFT : Vector3::RIGHT));
        node_->SetPosition(Vector3(playerID_==2 ? 2.23f + 0.5f : -2.23f - 0.5f, 0.0f, 0.0f));
    }

    rigidBody_->ResetForces();

}
void Player::SetPilotMode(bool pilotMode){
    pilotMode_ = pilotMode;
    node_->SetEnabled(true);
    pilot_->node_->SetEnabledRecursive(pilotMode_);
//    ship_.node_->SetEnabledRecursive(!pilotMode_);
    shieldNode_->SetEnabled(!pilotMode_);
    collisionShape_->SetSphere(pilotMode_? 0.23f : 2.0f);
    rigidBody_->SetLinearDamping(pilotMode_? 0.88f : 0.5f);
    rigidBody_->SetRestitution(!pilotMode_ * 0.666f);
}

void Player::SetHealth(float health)
{
    health_ = Clamp(health, 0.0f, 15.0f);


    if (health_ <= 0.0f){
        Die();
    }
}

Color Player::HealthToColor(float health)
{
    Color color(1.0f, 1.0f, 0.05f, 1.0f);
    health = Clamp(health, 0.0f, 10.0f);
    float maxBright{1.0f};
    if (health < 5.0f) maxBright = MC->Sine(2.0f+3.0f*(5.0f-health), 0.2f*health, 1.0f);
    color.r_ = Clamp((3.0f - (health - 3.0f))/3.0f, 0.0f, maxBright);
    color.g_ = Clamp((health - 3.0f)/4.0f, 0.0f, maxBright);
    return color;
}



//Updates autopilot input
void Player::Think()
{
    float playerFactor{( playerID_==2 ? 3.4f : 2.3f )};

    if (MC->GetSinceStateChange() < playerFactor * 0.1f){
        autoMove_ = Vector3::ZERO;
        return;
    }

    Vector3 pickupPos{Vector3::ZERO};
    Vector3 smell{Sniff(playerFactor) * playerFactor};
    Vector3 taste{Sniff(playerFactor, true) * playerFactor};

    Player* otherPlayer{MC->GetPlayer(playerID_, true)};
    
    switch (MC->GetGameState()) {
    case GS_LOBBY: {
//        bool splatterPillarsIdle{SPLATTERPILLAR->IsIdle() && OTHERSPLATTERPILLAR->IsIdle()};
//        Vector3 toPillar{SPLATTERPILLAR->GetPosition() - GetPosition() * static_cast<float>(SPLATTERPILLAR->IsIdle())};

//        if (MC->NoHumans()){
//            //Enter play
//            if (GetScore() == 0 && MC->GetPlayer(playerID_, true)->GetScore() == 0 && splatterPillarsIdle)
//                autoMove_ = 4.2f * (playerID_==2 ? Vector3::RIGHT : Vector3::LEFT) - GetPosition();
//            //Reset Score
//            else if (GetScore() != 0)
//                autoMove_ = toPillar;
//            //Stay put
//            else
//                autoMove_ = Vector3::ZERO;
//        }
//        //Reset Score
//        else if (otherPlayer->GetScore() == 0 && GetScore() != 0)
//            autoMove_ = toPillar;
//        //Exit
//        else if (DOOR->HidesPlayer() == 0.0f && OTHERDOOR->HidesPlayer() > 0.1f * playerFactor)
//            autoMove_ = Vector3::FORWARD;
//        else autoMove_ = Vector3::ZERO;
//        autoFire_ = Vector3::ZERO;
    } break;
    case GS_PLAY: {
        //Decide which pickup to pick up
        if (health_ < (5.0f - appleCount_)
                || flightScore_ == 0
                || (appleCount_ == 0 && health_ < 8.0f)
                || (heartCount_ != 0 && health_ <= 10.0f && weaponLevel_ > 13)
                || (weaponLevel_==23 && health_ <= 10.0f)
                ||  heartCount_ == 4) {
            pickupPos = MC->heart_->GetPosition();
        }
        else {
            pickupPos = MC->apple_->GetPosition();
        }
        //Calculate shortest route
        Vector3 newPickupPos{pickupPos};
        for (int i{0}; i < 6; ++i){
            Vector3 projectedPickupPos{pickupPos + (Quaternion(i * 60.0f, Vector3::UP) * Vector3::FORWARD * 46.0f)};
            if (LucKey::Distance(GetPosition(), projectedPickupPos - rigidBody_->GetLinearVelocity() * 0.42f) < LucKey::Distance(GetPosition(), pickupPos))
                newPickupPos = projectedPickupPos;
        }
        pickupPos = newPickupPos;
        //Calculate move vector
        if (pickupPos.y_ < -10.0f || LucKey::Distance(
                    GetPosition(), LucKey::Scale(pickupPos, Vector3(1.0f, 0.0f, 1.0f))) < playerFactor)
            pickupPos = GetPosition() + node_->GetDirection() * playerFactor;

        autoMove_ = 0.5f * (autoMove_ +
                            LucKey::Scale(pickupPos - node_->GetPosition()
                                          - 0.05f * playerFactor * rigidBody_->GetLinearVelocity()
                                          - 0.1f * playerFactor * node_->GetDirection()
                                          , Vector3(1.0f, 0.0f, 1.0f)).Normalized());
        if (LucKey::Distance(pickupPos, GetPosition()) > playerFactor)
            autoMove_ += smell * 13.0f;
        autoMove_ += Vector3(
                    MC->Sine(playerFactor, -0.05f, 0.05f, playerFactor),
                    0.0f,
                    MC->Sine(playerFactor, -0.05f, 0.05f, -playerFactor));
        //Pick firing target
        bool fire{false};
        Pair<float, Vector3> target{};
        for (SharedPtr<Razor> r : GetSubsystem<SpawnMaster>()->razors_.Values()){
            if (r->IsEnabled() && r->GetPosition().y_ > (-playerFactor * 0.1f)){
                float distance = LucKey::Distance(this->GetPosition(), r->GetPosition());
                float panic = r->GetPanic();
                float weight = (5.0f * panic) - (distance / playerFactor) + 42.0f;
                if (weight > target.first_){
                    target.first_ = weight;
                    target.second_ = r->GetPosition() + r->GetLinearVelocity() * 0.42f;
                    fire = true;
                }
            }
        }
        for (SharedPtr<Spire> s : GetSubsystem<SpawnMaster>()->spires_.Values()){
            if (s->IsEnabled() && s->GetPosition().y_ > (-playerFactor * 0.23f) && flightScore_ != 0){
                float distance = LucKey::Distance(this->GetPosition(), s->GetPosition());
                float panic = s->GetPanic();
                float weight = (23.0f * panic) - (distance / playerFactor) + 32.0f;
                if (weight > target.first_){
                    target.first_ = weight;
                    target.second_ = s->GetPosition();
                    fire = true;
                }
            }
        }
        if (fire){
            autoFire_ = (target.second_ - GetPosition()).Normalized();
            if (bulletAmount_ == 2 || bulletAmount_ == 3)
                autoFire_ = Quaternion((playerID_==2?-1.0f:1.0f)*Min(0.666f * LucKey::Distance(this->GetPosition(), target.second_), 5.0f), Vector3::UP) * autoFire_;
            autoFire_ = Quaternion((playerID_==2?-1.0f:1.0f)*MC->Sine(playerFactor, -playerFactor, playerFactor), Vector3::UP) * autoFire_;
        }
        else autoFire_ = Vector3::ZERO;
        autoFire_ -= taste;
    } break;
    default: break;
    }
}

Vector3 Player::Sniff(float playerFactor, bool taste)
{
    Vector3 smell;
    int whiskers{42};
    int detected{0};

    //Smell across borders
    for (int p{-1}; p < (taste ? 0 : 6); ++p){
        Vector3 projectedPlayerPos{( (p != -1) ? GetPosition() + (Quaternion(p * 60.0f, Vector3::UP) * Vector3::FORWARD * 46.0f)
                                               : GetPosition() )};
        for (int w = 0; w < whiskers; ++w){
            PODVector<PhysicsRaycastResult> hitResults{};
            Vector3 whiskerDirection = Quaternion((360.0f / whiskers) * w, Vector3::UP) * (2.0f * node_->GetDirection() + 3.0f * autoMove_.Normalized());
            Ray whiskerRay{projectedPlayerPos + Vector3::DOWN * Random(0.666f), whiskerDirection};
            if (MC->PhysicsRayCast(hitResults, whiskerRay, playerFactor * playerFactor, M_MAX_UNSIGNED)){
                ++detected;
                PhysicsRaycastResult r{hitResults[0]};
                    StringHash nodeNameHash{r.body_->GetNode()->GetNameHash()};
                    float distSquared{(r.distance_ * r.distance_) *
                                (0.005f * whiskerDirection.Angle(autoMove_) +
                                 playerFactor * playerFactor)};
                    if (nodeNameHash ==N_APPLE) {
                        smell += 230.0f * (whiskerDirection / (distSquared)) * (appleCount_ - static_cast<float>(flightScore_ == 0));
                    } else if (nodeNameHash == N_HEART) {
                        smell += 235.0f * (whiskerDirection / (distSquared)) * (heartCount_ * 2.0f - appleCount_ * 10.0f + (10.0f - health_));
                    } else if ((nodeNameHash == N_CHAOBALL) && !taste) {
                        if (r.body_->GetNode()->GetComponent<RigidBody>()->GetLinearVelocity().Length() < 5.0f)
                            smell += 666.0f * whiskerDirection / (distSquared * distSquared);
                    } else if ((nodeNameHash == N_CHAOMINE) && !taste) {
                        if (r.body_->GetNode()->GetComponent<RigidBody>()->GetLinearVelocity().Length() < 5.0f)
                            smell += 9000.0f * whiskerDirection / (distSquared * distSquared * Random(5.0f));
                    } else if (nodeNameHash == N_RAZOR) {
                        smell -= 320.0f * (whiskerDirection / (distSquared));
                    } else if (nodeNameHash == N_SPIRE) {
                        smell -= 3200.0f * (whiskerDirection / (distSquared * distSquared));
                    } else if (nodeNameHash == N_SEEKER && !taste) {
                        smell -= 1000.0f * (whiskerDirection / r.distance_) * (3.0f - 2.0f * static_cast<float>(health_ > 10.0f));
                        ++detected;
                    }
                    if (!taste){
                        if (nodeNameHash != N_APPLE && nodeNameHash != N_HEART
                                && nodeNameHash != N_CHAOBALL && nodeNameHash != N_CHAOMINE
                                && nodeNameHash != StringHash("PickupTrigger"))
                            smell += 0.005f * whiskerDirection * r.distance_;
                        else smell += 0.005f * whiskerDirection * playerFactor * playerFactor;
                    }
                }
            else if (!taste) smell += 0.005f * whiskerDirection * playerFactor * playerFactor;
//            }
        }
    }
    //Rely more on scent in crowded spaces
    if (!taste){
        float scentEffect{0.23f * static_cast<float>(detected) / whiskers};
        autoMove_ *= 1.0f - scentEffect;
        autoMove_ -= 0.5f * scentEffect * rigidBody_->GetLinearVelocity();
        smell *= 1.0f + (0.5f * scentEffect);
    }
    return smell / whiskers;
}
*/
