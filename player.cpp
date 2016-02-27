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

#include <fstream>

#include "mastercontrol.h"
#include "hexocam.h"
#include "spawnmaster.h"
#include "tilemaster.h"
#include "multix.h"
#include "bullet.h"
#include "muzzle.h"
#include "explosion.h"
#include "player.h"

Player::Player(Context *context, MasterControl *masterControl, int playerID):
    SceneObject(context, masterControl),
    playerID_{playerID},
    alive_{true},
    appleCount_{0},
    heartCount_{0},
    initialHealth_{1.0f},
    health_{initialHealth_},
    score_{0},
    flightScore_{0},
    multiplier_{1},
    weaponLevel_{0},
    bulletAmount_{1},
    initialShotInterval_{0.30f},
    shotInterval_{initialShotInterval_},
    sinceLastShot_{0.0f}
{

    rootNode_->SetName("Player");
    CreateGUI();

    SetupShip();

    //Setup pilot
    pilot_.node_ = rootNode_->CreateChild("Pilot");
    pilot_.bodyModel_ = pilot_.node_->CreateComponent<AnimatedModel>();
    pilot_.bodyModel_->SetModel(masterControl_->resources.models.pilots.male);
    pilot_.bodyModel_->SetCastShadows(true);
    pilot_.hairModel_ = pilot_.node_->GetChild("Head", true)->CreateComponent<StaticModel>();
    pilot_.hairModel_->SetCastShadows(true);
    LoadPilot();
    animCtrl_ = pilot_.node_->CreateComponent<AnimationController>();

    //Setup shield
    shieldNode_ = rootNode_->CreateChild("Shield");
    shieldModel_ = shieldNode_->CreateComponent<StaticModel>();
    shieldModel_->SetModel(masterControl_->cache_->GetResource<Model>("Models/Shield.mdl"));
    shieldMaterial_ = masterControl_->cache_->GetTempResource<Material>("Materials/Shield.xml");
    shieldModel_->SetMaterial(shieldMaterial_);
    
    //Setup ChaoFlash
    chaoFlash_ = new ChaoFlash(context_, masterControl_, playerID_);

    //Setup player audio
    shot_s = masterControl_->cache_->GetResource<Sound>("Samples/Shot.ogg");
    shot_s->SetLooped(false);
    shieldHit_s = masterControl_->cache_->GetResource<Sound>("Samples/ShieldHit.ogg");
    shieldHit_s->SetLooped(false);
    pickup_s = masterControl_->cache_->GetResource<Sound>("Samples/Pickup.ogg");
    pickup_s->SetLooped(false);
    powerup_s = masterControl_->cache_->GetResource<Sound>("Samples/Powerup.ogg");
    powerup_s->SetLooped(false);
    multix_s= masterControl_->cache_->GetResource<Sound>("Samples/MultiX.ogg");
    multix_s->SetLooped(false);
    chaoball_s = masterControl_->cache_->GetResource<Sound>("Samples/Chaos.ogg");
    chaoball_s->SetLooped(false);
    for (int s = 1; s < 5; ++s){
        seekerHits_s.Push(SharedPtr<Sound>(masterControl_->cache_->GetResource<Sound>("Samples/SeekerHit"+String(s)+".ogg")));
    }
    //Some extra sources for the players
    for (int i = 0; i < 5; ++i){
        SharedPtr<SoundSource> extraSampleSource = SharedPtr<SoundSource>(rootNode_->CreateComponent<SoundSource>());
        extraSampleSource->SetSoundType(SOUND_EFFECT);
        sampleSources_.Push(extraSampleSource);
    }

    //Setup player physics
    rigidBody_ = rootNode_->CreateComponent<RigidBody>();
    rigidBody_->SetRestitution(0.666f);
    rigidBody_->SetMass(1.0f);
    rigidBody_->SetLinearFactor(Vector3::ONE - Vector3::UP);
    rigidBody_->SetLinearDamping(0.5f);
    rigidBody_->SetAngularFactor(Vector3::ZERO);
    rigidBody_->SetLinearRestThreshold(0.01f);
    rigidBody_->SetAngularRestThreshold(0.1f);

    collisionShape_ = rootNode_->CreateComponent<CollisionShape>();
    collisionShape_->SetSphere(2.0f);

    masterControl_->tileMaster_->AddToAffectors(WeakPtr<Node>(rootNode_), WeakPtr<RigidBody>(rigidBody_));

    //Subscribe to events
    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Player, HandleSceneUpdate));

    for (int b = 0; b < 64; b++){
        Bullet* bullet = new Bullet(context_, masterControl_, playerID_);
        bullets_.Push(SharedPtr<Bullet>(bullet));
    }
}

void Player::CreateGUI()
{
    //Setup 3D GUI elements
    guiNode_ = masterControl_->world.scene->CreateChild("GUI3D");

    scoreNode_ = guiNode_->CreateChild("Score");
    for (int d = 0; d < 10; ++d){
        scoreDigits_[d] = scoreNode_->CreateChild("Digit");
        scoreDigits_[d]->SetEnabled( d == 0 );
        scoreDigits_[d]->Translate(Vector3::RIGHT * (playerID_ == 2 ? -0.5f : 0.5f) * d);
        scoreDigits_[d]->Rotate(Quaternion(playerID_ == 2 ? 0.0f : 180.0f, Vector3::UP), TS_WORLD);
        StaticModel* digitModel = scoreDigits_[d]->CreateComponent<StaticModel>();
        digitModel->SetModel(masterControl_->cache_->GetResource<Model>("Models/0.mdl"));
        digitModel->SetMaterial(playerID_==2
                                ? masterControl_->resources.materials.ship2Secondary
                                : masterControl_->resources.materials.ship1Secondary);
    }
    scoreNode_->SetPosition(Vector3(playerID_ == 2 ? 5.94252f : -5.94252f, 0.88069f, 0.82951f));
    scoreNode_->Rotate(Quaternion(-90.0f, Vector3::RIGHT));
    scoreNode_->Rotate(Quaternion(playerID_ == 2 ? 60.0f : -60.0f, Vector3::UP), TS_WORLD);

    Model* barModel = (playerID_ == 2)
            ? masterControl_->cache_->GetResource<Model>("Models/BarRight.mdl")
            : masterControl_->cache_->GetResource<Model>("Models/BarLeft.mdl");

    healthBarNode_ = guiNode_->CreateChild("HealthBar");
    healthBarNode_->SetPosition(Vector3(0.0f, 1.0f, 21.0f));
    healthBarNode_->SetScale(Vector3(health_, 1.0f, 1.0f));
    healthBarModel_ = healthBarNode_->CreateComponent<StaticModel>();
    healthBarModel_->SetModel(barModel);
    healthBarModel_->SetMaterial(masterControl_->cache_->GetTempResource<Material>("Materials/GreenGlowEnvmap.xml"));

    shieldBarNode_ = guiNode_->CreateChild("HealthBar");
    shieldBarNode_->SetPosition(Vector3(0.0f, 1.0f, 21.0f));
    shieldBarNode_->SetScale(Vector3(health_, 0.9f, 0.9f));
    shieldBarModel_ = shieldBarNode_->CreateComponent<StaticModel>();
    shieldBarModel_->SetModel(barModel);
    shieldBarModel_->SetMaterial(masterControl_->cache_->GetResource<Material>("Materials/BlueGlowEnvmap.xml"));

    Node* healthBarHolderNode = guiNode_->CreateChild("HealthBarHolder");
    healthBarHolderNode->SetPosition(Vector3(0.0f, 1.0f, 21.0f));
    StaticModel* healthBarHolderModel = healthBarHolderNode->CreateComponent<StaticModel>();
    healthBarHolderModel->SetModel(masterControl_->cache_->GetResource<Model>("Models/BarHolder.mdl"));
    healthBarHolderModel->SetMaterial(masterControl_->cache_->GetResource<Material>("Materials/Metal.xml"));

    appleCounterRoot_ = guiNode_->CreateChild("AppleCounter");
    for (int a = 0; a < 4; a++){
        appleCounter_[a] = appleCounterRoot_->CreateChild();
        appleCounter_[a]->SetEnabled(false);
        appleCounter_[a]->SetPosition(Vector3(playerID_ == 2 ? (a + 8.0f) : -(a + 8.0f), 1.0f, 21.0f));
        appleCounter_[a]->SetScale(0.333f);
        StaticModel* apple = appleCounter_[a]->CreateComponent<StaticModel>();
        apple->SetModel(masterControl_->cache_->GetResource<Model>("Models/Apple.mdl"));
        apple->SetMaterial(masterControl_->cache_->GetTempResource<Material>("Materials/GoldEnvmap.xml"));
    }

    heartCounterRoot_ = guiNode_->CreateChild("HeartCounter");
    for (int h = 0; h < 4; h++){
        heartCounter_[h] = heartCounterRoot_->CreateChild();
        heartCounter_[h]->SetEnabled(false);
        heartCounter_[h]->SetPosition(Vector3(playerID_ == 2 ? (h + 8.0f) : -(h + 8.0f), 1.0f, 21.0f));
        heartCounter_[h]->SetScale(0.333f);
        StaticModel* heart = heartCounter_[h]->CreateComponent<StaticModel>();
        heart->SetModel(masterControl_->cache_->GetResource<Model>("Models/Heart.mdl"));
        heart->SetMaterial(masterControl_->cache_->GetTempResource<Material>("Materials/RedEnvmap.xml"));
    }
}

void Player::SetScore(int points)
{
    score_ = points;

    for (int d = 0; d < 10; ++d){
        StaticModel* digitModel = scoreDigits_[d]->GetComponent<StaticModel>();
        digitModel->SetModel(masterControl_->cache_->GetResource<Model>("Models/"+String((int)(score_ / pow(10, d))%10)+".mdl"));
        scoreDigits_[d]->SetEnabled( score_ >= pow(10, d) || d == 0 );
//        digitModel->SetMaterial(playerID_==2
//                                ? masterControl_->resources.materials.ship2Secondary
//                                : masterControl_->resources.materials.ship1Secondary);
    }
}
void Player::ResetScore()
{
    SetScore(0);
}
void Player::AddScore(int points)
{
    points *= multiplier_;
    unsigned nextMultiX = pow(10, multiplier_+1);
    if (flightScore_ < nextMultiX && flightScore_ + points > nextMultiX){
        masterControl_->multiX_->Respawn();
    }

    SetScore(GetScore()+points);
    flightScore_ += points;
}

void Player::KillPilot()
{
    alive_ = false;
    EnterLobby();
}

void Player::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    //Take the frame time step, which is stored as a double
    float timeStep = eventData[Update::P_TIMESTEP].GetFloat();
    //Pulse and spin the counters' apples and hearts
    UpdateGUI(timeStep);

    //Only handle input when player is active
    if (!rootNode_->IsEnabled()) return;

    Input* input = GetSubsystem<Input>();

    //Movement values
    Vector3 move = Vector3::ZERO;
    Vector3 moveJoy = Vector3::ZERO;
    Vector3 moveKey = Vector3::ZERO;
    float thrust = pilotMode_ ? 256.0f : 2342.0f;
    float maxSpeed = pilotMode_? 1.8f : 23.0f;    //Firing values
    Vector3 fire = Vector3::ZERO;
    Vector3 fireJoy = Vector3::ZERO;
    Vector3 fireKey = Vector3::ZERO;

    //Read input
    if (input->GetJoystickByIndex(playerID_ - 1)){
        JoystickState* joystick = input->GetJoystickByIndex(playerID_ - 1);
        moveJoy = Vector3::RIGHT * joystick->GetAxisPosition(0) +
                Vector3::BACK * joystick->GetAxisPosition(1);
        fireJoy = Vector3::RIGHT * joystick->GetAxisPosition(2) +
                Vector3::BACK * joystick->GetAxisPosition(3);
    } else if (playerID_ == 1 || input->GetJoystickByIndex(0)) {
        moveKey = Vector3::LEFT * input->GetKeyDown(KEY_A) +
                Vector3::RIGHT * input->GetKeyDown(KEY_D) +
                Vector3::FORWARD * input->GetKeyDown(KEY_W) +
                Vector3::BACK * input->GetKeyDown(KEY_S);
        fireKey = Vector3::LEFT * (input->GetKeyDown(KEY_J) || input->GetKeyDown(KEY_KP_4)) +
                Vector3::RIGHT * (input->GetKeyDown(KEY_L) || input->GetKeyDown(KEY_KP_6)) +
                Vector3::FORWARD * (input->GetKeyDown(KEY_I) || input->GetKeyDown(KEY_KP_8)) +
                Vector3::BACK * (input->GetKeyDown(KEY_K) || input->GetKeyDown(KEY_KP_2) || input->GetKeyDown(KEY_KP_5)) +
                Quaternion(45.0f, Vector3::UP)*Vector3::LEFT * input->GetKeyDown(KEY_KP_7) +
                Quaternion(45.0f, Vector3::UP)*Vector3::RIGHT * input->GetKeyDown(KEY_KP_3) +
                Quaternion(45.0f, Vector3::UP)*Vector3::FORWARD * input->GetKeyDown(KEY_KP_9) +
                Quaternion(45.0f, Vector3::UP)*Vector3::BACK * input->GetKeyDown(KEY_KP_1);
    }

    //Pick most significant input
    moveJoy.Length() > moveKey.Length() ? move = moveJoy : move = moveKey;
    fireJoy.Length() > fireKey.Length() ? fire = fireJoy : fire = fireKey;

    //Restrict move vector length
    if (move.Length() > 1.0f) move /= move.Length();
    //Deadzones
    else if (move.Length() < 0.1f) move *= 0.0f;

    if (fire.Length() < 0.1f) fire *= 0.0f;
    else fire.Normalize();

    //When in pilot mode
    if (pilotMode_){
        //Apply movement
        Vector3 force = move * thrust * timeStep;
        if (rigidBody_->GetLinearVelocity().Length() < maxSpeed ||
                (rigidBody_->GetLinearVelocity().Normalized() + force.Normalized()).Length() < 1.4142f) {
            rigidBody_->ApplyForce(force);
        }

        //Update rotation according to direction of the player's movement.
        Vector3 velocity = rigidBody_->GetLinearVelocity();
        Vector3 lookDirection = velocity + 2.0f*fire;
        Quaternion rotation = rootNode_->GetWorldRotation();
        Quaternion aimRotation = rotation;
        aimRotation.FromLookRotation(lookDirection);
        rootNode_->SetRotation(rotation.Slerp(aimRotation, 7.0f * timeStep * velocity.Length()));

        //Update animation
        if (velocity.Length() > 0.1f){
            animCtrl_->PlayExclusive("Models/WalkRelax.ani", 0, true, 0.15f);
            animCtrl_->SetSpeed("Models/WalkRelax.ani", velocity.Length()*2.3f);
            animCtrl_->SetStartBone("Models/WalkRelax.ani", "MasterBone");
        }
        else {
            animCtrl_->PlayExclusive("Models/IdleRelax.ani", 0, true, 0.15f);
            animCtrl_->SetStartBone("Models/IdleRelax.ani", "MasterBone");
        }
    // When in ship mode
    } else {
        //Update shield
        Quaternion randomRotation = Quaternion(Random(360.0f),Random(360.0f),Random(360.0f));
        shieldNode_->SetRotation(shieldNode_->GetRotation().Slerp(randomRotation, Random(1.0f)));
        Color shieldColor = shieldMaterial_->GetShaderParameter("MatDiffColor").GetColor();
        Color newColor = Color(shieldColor.r_ * Random(0.6f, 0.9f),
                               shieldColor.g_ * Random(0.7f, 0.95f),
                               shieldColor.b_ * Random(0.8f, 0.9f));
        shieldMaterial_->SetShaderParameter("MatDiffColor", shieldColor.Lerp(newColor, Min(timeStep * 23.5f, 1.0f)));
        
        //Float
        ship_.node_->SetPosition(Vector3::UP *masterControl_->Sine(2.3f, -0.1f, 0.1f));
        //Apply movement
        Vector3 force = move * thrust * timeStep;
        if (rigidBody_->GetLinearVelocity().Length() < maxSpeed ||
                (rigidBody_->GetLinearVelocity().Normalized() + force.Normalized()).Length() < 1.0f) {
            rigidBody_->ApplyForce(force);
        }

        //Update rotation according to direction of the ship's movement.
        if (rigidBody_->GetLinearVelocity().Length() > 0.1f)
            rootNode_->LookAt(rootNode_->GetPosition()+rigidBody_->GetLinearVelocity());

        //Update tails
        for (int t = 0; t < 3; t++)
        {
            float velocityToScale = Clamp(0.23f * rigidBody_->GetLinearVelocity().Length(), 0.0f, 1.0f);
            TailGenerator* tailGen = tailGens_[t];
            tailGen->SetTailLength(t==1? velocityToScale * 0.1f : velocityToScale * 0.075f);
//            tailGen->SetNumTails(t==1? (int)(velocityToScale * 23) : (int)(velocityToScale * 16));
            tailGen->SetWidthScale(t==1? velocityToScale * 0.666f : velocityToScale * 0.23f);
        }

        //Shooting
        sinceLastShot_ += timeStep;
        if (fire.Length()) {
            if (sinceLastShot_ > shotInterval_)
            {
                Shoot(fire);
            }
        }
    }
}

void Player::UpdateGUI(float timeStep)
{
    for (int i = 0; i < 4; i++){
        appleCounter_[i]->Rotate(Quaternion(0.0f, (i*i+10.0f) * 23.0f * timeStep, 0.0f));
        appleCounter_[i]->SetScale(masterControl_->Sine((1.0f+(appleCount_))*2.0f, 0.2f, 0.4, -i));
        heartCounter_[i]->Rotate(Quaternion(0.0f, (i*i+10.0f) * 23.0f * timeStep, 0.0f));
        heartCounter_[i]->SetScale(masterControl_->Sine((1.0f+(heartCount_))*2.0f, 0.2f, 0.4, -i));
    }
    //Update HealthBar color
    healthBarModel_->GetMaterial()->SetShaderParameter("MatEmissiveColor", HealthToColor(health_));
    healthBarModel_->GetMaterial()->SetShaderParameter("MatSpecularColor", HealthToColor(health_));
}

void Player::Shoot(Vector3 fire)
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
        Vector3 direction = Quaternion(angle, Vector3::UP) * fire;
        FireBullet(direction);
    }
    sinceLastShot_ = 0.0;
    //Create a single muzzle flash
    if (bulletAmount_ > 0){
        MoveMuzzle();
        PlaySample(shot_s, 0.17f);
    }
}

void Player::FireBullet(Vector3 direction){
    SharedPtr<Bullet> bullet;
    if (bullets_.Size() > 0){
        for (unsigned b = 0; b < bullets_.Size(); ++b){
            if (!bullets_[b]->rootNode_->IsEnabled()){
                bullet = bullets_[b];
            }
        }
    }
    if (bullet == nullptr){
        bullet = new Bullet(context_, masterControl_, playerID_);
        bullets_.Push(bullet);
    }
    bullet->Set(rootNode_->GetPosition() + direction + Vector3::DOWN*0.42f);
    bullet->rootNode_->LookAt(bullet->rootNode_->GetPosition() + direction * 5.0f);
    bullet->rigidBody_->ApplyForce(direction * (1500.0f + 23.0f * weaponLevel_));
    bullet->damage_ = 0.15f + 0.00666f * weaponLevel_;
}
void Player::MoveMuzzle()
{
    if (muzzle_ == nullptr)
        muzzle_ = new Muzzle(context_, masterControl_, playerID_);
    muzzle_->Set(rootNode_->GetPosition() + Vector3::DOWN * 0.42f);
}


void Player::Pickup(PickupType pickup)
{
    if (health_ <= 0.0f) return;

    switch (pickup) {
    case PT_APPLE: {
        bulletAmount_ = (bulletAmount_ == 0)?1:bulletAmount_;
        heartCount_ = 0;
        AddScore(23);
        if (weaponLevel_ < 23)
            ++appleCount_;
        if (appleCount_ >= 5){
            UpgradeWeapons();
            appleCount_ = 0;
            PlaySample(powerup_s, 0.42f);
        }
        else PlaySample(pickup_s, 0.23f);
    } break;
    case PT_HEART: {
        ++heartCount_;
        appleCount_ = 0;
        if (heartCount_ >= 5){
            SetHealth(15.0);
            heartCount_ = 0;
            PlaySample(powerup_s, 0.42f);
        }
        else {
            SetHealth(Max(health_, Clamp(health_+5.0f, 0.0f, 10.0f)));
            PlaySample(pickup_s, 0.23f);
        }
    } break;
    case PT_MULTIX: {
        multiplier_++;
        PlaySample(multix_s, 0.42f);
    } break;
    case PT_CHAOBALL: {
       PickupChaoBall();
    } break;
    case PT_RESET: {
        appleCount_ = 0;
        heartCount_= 0;
    }
    }

    //Update Pickup GUI elements
    for (int a = 0; a < 4; a++){
        if (appleCount_ > a) appleCounter_[a]->SetEnabled(true);
        else appleCounter_[a]->SetEnabled(false);
    }
    for (int h = 0; h < 4; h++){
        if (heartCount_ > h) heartCounter_[h]->SetEnabled(true);
        else heartCounter_[h]->SetEnabled(false);
    }
}

void Player::PickupChaoBall()
{
    chaoFlash_->Set(GetPosition());
    rootNode_->Translate(Quaternion(Random(360.0f), Vector3::UP) * Vector3::FORWARD * Random(5.0f));
    PlaySample(chaoball_s, 0.8f);
}

void Player::Die()
{
    alive_ = false;

    Disable();
    masterControl_->spawnMaster_->SpawnExplosion(rootNode_->GetPosition(), playerID_ == 2 ? Color(1.0f, 0.42f, 0.0f) : Color(0.23f, 1.0f, 0.0f), 2.0f, playerID_);

    int otherplayer = playerID_ == 2 ? 1 : 2;
    if (!masterControl_->GetPlayer(otherplayer)->IsAlive())
        masterControl_->SetGameState(GS_DEAD);
}

void Player::EnterPlay()
{
    rootNode_->SetRotation(Quaternion(playerID_==2 ? -90.0f : 90.0f, Vector3::UP));
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
    RemoveTails();
    CreateTails();
    Set(Vector3(playerID_==2 ? 4.2f : -4.2f, 0.0f, 0.0f));
    SetPilotMode(false);

    scoreNode_->SetWorldScale(4.2f);
    scoreNode_->SetPosition(Vector3(playerID_ == 2 ? 23.5f : -23.5f, 2.23f, 1.23f));
}
void Player::EnterLobby()
{
    StopAllSound();
    chaoFlash_->Disable();
    rootNode_->SetPosition(Vector3(playerID_==2 ? 2.23f + 0.5f*alive_ : -2.23f - 0.5f*alive_, 0.0f, !alive_ * 5.5f));
    rigidBody_->SetLinearVelocity(!alive_ * Vector3::BACK*2.3f + alive_ * ((playerID_ == 2) ? Vector3::LEFT : Vector3::RIGHT));
    rigidBody_->ResetForces();

    scoreNode_->SetWorldScale(1.0f);
    scoreNode_->SetPosition(Vector3(playerID_ == 2 ? 5.94252f : -5.94252f, 0.88069f, 0.82951f));

    if (!IsAlive()){
        CreateNewPilot();
        ResetScore();
    }
    SetPilotMode(true);
}
void Player::SetPilotMode(bool pilotMode){
    pilotMode_ = pilotMode;
    rootNode_->SetEnabled(true);
    pilot_.node_->SetEnabledRecursive(pilotMode_);
    ship_.node_->SetEnabledRecursive(!pilotMode_);
    shieldNode_->SetEnabled(!pilotMode_);
    collisionShape_->SetSphere(pilotMode_? 0.23f : 2.0f);
    rigidBody_->SetLinearDamping(pilotMode_? 0.88f : 0.5f);
}

void Player::SetHealth(float health)
{
    health_ = Clamp(health, 0.0f, 15.0f);
    healthBarNode_->SetScale(Vector3(Min(health_, 10.0f), 1.0f, 1.0f));
    shieldBarNode_->SetScale(Vector3(health_, 0.95f, 0.95f));

    if (health_ <= 0.0f){
        Die();
    }
}

Color Player::HealthToColor(float health)
{
    Color color(1.0f, 1.0f, 0.05f, 1.0f);
    health = Clamp(health, 0.0f, 10.0f);
    float maxBright = 1.0f;
    if (health < 5.0f) maxBright = masterControl_->Sine(2.0f+3.0f*(5.0f-health), 0.2f*health, 1.0f);
    color.r_ = Clamp((3.0f - (health - 3.0f))/3.0f, 0.0f, maxBright);
    color.g_ = Clamp((health - 3.0f)/4.0f, 0.0f, maxBright);
    return color;
}

void Player::Hit(float damage, bool melee)
{
    if (health_ > 10.0f){
        damage *= (melee ? 0.75f : 0.25f);
        shieldMaterial_->SetShaderParameter("MatDiffColor", Color(2.0f, 3.0f, 5.0f, 1.0f));
        PlaySample(shieldHit_s, 0.23f);
    }
    else if (!melee) PlaySample(seekerHits_s[Random((int)seekerHits_s.Size())], 0.75f);
    SetHealth(health_ - damage);
}

void Player::UpgradeWeapons()
{
    ++weaponLevel_;
    bulletAmount_ = 1 + ((weaponLevel_+5) / 6);
    shotInterval_ = initialShotInterval_ - 0.0042f*weaponLevel_;
}

void Player::LoadPilot()
{
    using namespace std;
    ifstream fPilot("Pilot"+to_string(playerID_)+".lkp");
    while (!fPilot.eof()){
        string gender_str;
        string hairStyle_str;
        string color1_r_str, color1_g_str, color1_b_str;
        string color2_r_str, color2_g_str, color2_b_str;
        string color3_r_str, color3_g_str, color3_b_str;
        string color4_r_str, color4_g_str, color4_b_str;
        string color5_r_str, color5_g_str, color5_b_str;
        string score_str;

        fPilot >> gender_str;
        if (gender_str.empty()) break;
        fPilot >>
                hairStyle_str >>
                color1_r_str >> color1_g_str >> color1_b_str >>
                color2_r_str >> color2_g_str >> color2_b_str >>
                color3_r_str >> color3_g_str >> color3_b_str >>
                color4_r_str >> color4_g_str >> color4_b_str >>
                color5_r_str >> color5_g_str >> color5_b_str >>
                score_str;

        pilot_.male_ = stoi(gender_str);
        pilot_.hairStyle_ = stoi(hairStyle_str);
        pilot_.colors_.Clear();
        pilot_.colors_.Push(Color(stof(color1_r_str),stof(color1_g_str),stof(color1_b_str)));
        pilot_.colors_.Push(Color(stof(color2_r_str),stof(color2_g_str),stof(color2_b_str)));
        pilot_.colors_.Push(Color(stof(color3_r_str),stof(color3_g_str),stof(color3_b_str)));
        pilot_.colors_.Push(Color(stof(color4_r_str),stof(color4_g_str),stof(color4_b_str)));
        pilot_.colors_.Push(Color(stof(color5_r_str),stof(color5_g_str),stof(color5_b_str)));

        unsigned long score = stoul(score_str, 0, 10);
        SetScore(score);
    }
    if (!pilot_.colors_.Size()) {
        CreateNewPilot();
        alive_ = false;
    }

    UpdatePilot();
}

void Player::UpdatePilot()
{
    //Set body model
    if (pilot_.male_){
        pilot_.bodyModel_->SetModel(masterControl_->resources.models.pilots.male);}
    else{
        pilot_.bodyModel_->SetModel(masterControl_->resources.models.pilots.female);
    }

    //Set colors for body model
    for (unsigned m = 0; m < pilot_.bodyModel_->GetNumGeometries(); m++){
        pilot_.bodyModel_->SetMaterial(m, masterControl_->cache_->GetTempResource<Material>("Materials/Basic.xml"));
        Color diffColor = pilot_.colors_[m];
        pilot_.bodyModel_->GetMaterial(m)->SetShaderParameter("MatDiffColor", diffColor);
        Color specColor = diffColor*(1.0f-0.1f*m);
        specColor.a_ = 23.0f - 2.0f * m;
        pilot_.bodyModel_->GetMaterial(m)->SetShaderParameter("MatSpecColor", specColor);
    }

    //Set hair model
    if (!pilot_.hairStyle_)
        pilot_.hairModel_->SetModel(nullptr);
    else {
        pilot_.hairModel_->SetModel(masterControl_->resources.models.pilots.hairStyles[pilot_.hairStyle_ - 1]);
        //Set color for hair model
        pilot_.hairModel_->SetMaterial(masterControl_->cache_->GetTempResource<Material>("Materials/Basic.xml"));
        Color diffColor = pilot_.colors_[4];
        pilot_.hairModel_->GetMaterial()->SetShaderParameter("MatDiffColor", diffColor);
        Color specColor = diffColor*0.23f;
        specColor.a_ = 23.0f;
        pilot_.hairModel_->GetMaterial()->SetShaderParameter("MatSpecColor", specColor);
    }
}

void Player::CreateNewPilot()
{
    alive_ = true;

    pilot_.male_ = Random(2);
    pilot_.hairStyle_ = Random((int)masterControl_->resources.models.pilots.hairStyles.Size() + 1);

    pilot_.node_->SetRotation(Quaternion(0.0f, 0.0f, 0.0f));

    pilot_.colors_.Clear();
    for (int c = 0; c < 5; c++)
    {
        switch (c){
        case 0:{
            pilot_.colors_.Push(LucKey::RandomSkinColor());
        } break;
        case 4:{
            pilot_.colors_.Push(LucKey::RandomHairColor());
        } break;
        default: pilot_.colors_.Push(LucKey::RandomColor()); break;
        }
    }
    UpdatePilot();
}

void Player::SetupShip()
{
    ship_.node_ = rootNode_->CreateChild("Ship");
    ship_.model_ = ship_.node_->CreateComponent<StaticModel>();
    ship_.model_->SetModel(masterControl_->resources.models.ships.swift);
    ship_.model_->SetMaterial(0, playerID_==2 ? masterControl_->resources.materials.ship2Secondary : masterControl_->resources.materials.ship1Secondary);
    ship_.model_->SetMaterial(1, playerID_==2 ? masterControl_->resources.materials.ship2Primary : masterControl_->resources.materials.ship1Primary);

    ParticleEmitter* particleEmitter = ship_.node_->CreateComponent<ParticleEmitter>();
    SharedPtr<ParticleEffect> particleEffect = masterControl_->cache_->GetTempResource<ParticleEffect>("Particles/Shine.xml");
    Vector<ColorFrame> colorFrames;
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.0f));
    colorFrames.Push(ColorFrame(playerID_==2 ? Color(0.666f, 0.23f, 0.88f, 0.23f) : Color(0.42f, 0.7f, 0.23f, 0.23f), 0.2f));
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.4f));
    particleEffect->SetColorFrames(colorFrames);
    particleEmitter->SetEffect(particleEffect);

    CreateTails();
}

void Player::CreateTails()
{
    for (int n = 0; n < 3; n++)
    {
        Node* tailNode = ship_.node_->CreateChild("Tail");
        tailNode->SetPosition(Vector3(-0.85f+0.85f*n, n==1? 0.0f : -0.5f, n==1? -0.5f : -0.23f));
        TailGenerator* tailGen = tailNode->CreateComponent<TailGenerator>();
        tailGen->SetDrawHorizontal(true);
        tailGen->SetDrawVertical(n==1?true:false);
        tailGen->SetTailLength(n==1? 0.1f : 0.075f);
        tailGen->SetNumTails(n==1? 23 : 16);
        tailGen->SetWidthScale(n==1? 0.666f : 0.23f);
        tailGen->SetColorForHead(playerID_==2 ? Color(1.0f, 0.666f, 0.23f) : Color(1.0f, 1.0f, 0.23f));
        tailGen->SetColorForTip(playerID_==2 ? Color(1.0f, 0.23f, 0.0f) : Color(0.42f, 1.0f, 0.0f));
        tailGens_.Push(SharedPtr<TailGenerator>(tailGen));
    }
}
void Player::RemoveTails()
{
    for (unsigned i = 0; i < tailGens_.Size(); i++){
        tailGens_[i]->GetNode()->Remove();
    }
    tailGens_.Clear();
}
