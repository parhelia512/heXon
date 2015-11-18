/* heXon
// Copyright (C) 2015 LucKey Productions (luckeyproductions.nl)
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

Player::Player(Context *context, MasterControl *masterControl):
    SceneObject(context, masterControl),
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
    LoadScore();

    rootNode_->SetName("Player");
    //Setup GUI elements
    CreateGUI();

    //Setup ship
    SetupShip();

    //Setup pilot
    pilot_.node_ = rootNode_->CreateChild("Pilot");
    pilot_.node_->Translate(Vector3(0.0f, -0.5f, 0.0f));
    pilot_.model_ = pilot_.node_->CreateComponent<AnimatedModel>();
    pilot_.model_->SetCastShadows(true);

    LoadPilot();
    animCtrl_ = pilot_.node_->CreateComponent<AnimationController>();
    //Setup shield
    shieldNode_ = rootNode_->CreateChild("Shield");
    shieldModel_ = shieldNode_->CreateComponent<StaticModel>();
    shieldModel_->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Shield.mdl"));
    shieldMaterial_ = masterControl_->cache_->GetResource<Material>("Resources/Materials/Shield.xml");
    shieldModel_->SetMaterial(shieldMaterial_);
    
    //Setup ChaoFlash
    chaoFlash_ = new ChaoFlash(context_, masterControl_);

    //Setup player audio
    shot_ = masterControl_->cache_->GetResource<Sound>("Resources/Samples/Shot.ogg");
    shot_->SetLooped(false);
    for (int i = 0; i < 3; i++){
        sampleSources_.Push(SharedPtr<SoundSource>(rootNode_->CreateComponent<SoundSource>()));
        sampleSources_[i]->SetGain(0.3f);
        sampleSources_[i]->SetSoundType(SOUND_EFFECT);
    }

    //Setup player physics
    rigidBody_ = rootNode_->CreateComponent<RigidBody>();
    rigidBody_->SetRestitution(0.666);
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
        Bullet* bullet = new Bullet(context_, masterControl_);
        bullets_.Push(SharedPtr<Bullet>(bullet));
    }
}

void Player::CreateGUI()
{
    UI* ui = GetSubsystem<UI>();
    Text* scoreText = ui->GetRoot()->CreateChild<Text>();
    scoreText->SetName("Score");
    scoreTextName_ = scoreText->GetName();
    scoreText->SetText(String(score_));
    scoreText->SetFont(masterControl_->cache_->GetResource<Font>("Resources/Fonts/skirmishergrad.ttf"), 32);
    scoreText->SetColor(Color(0.5f, 0.95f, 1.0f, 0.9f));
    scoreText->SetHorizontalAlignment(HA_CENTER);
    scoreText->SetVerticalAlignment(VA_CENTER);
    scoreText->SetPosition(0, ui->GetRoot()->GetHeight()/2.5f);

    //Setup 3D GUI elements
    guiNode_ = masterControl_->world.scene->CreateChild("GUI3D");
    healthBarNode_ = guiNode_->CreateChild("HealthBar");
    healthBarNode_->SetPosition(Vector3(0.0f, 1.0f, 21.0f));
    healthBarNode_->SetScale(Vector3(health_, 1.0f, 1.0f));
    healthBarModel_ = healthBarNode_->CreateComponent<StaticModel>();
    healthBarModel_->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Bar.mdl"));
    healthBarModel_->SetMaterial(masterControl_->cache_->GetTempResource<Material>("Resources/Materials/GreenGlowEnvmap.xml"));

    shieldBarNode_ = guiNode_->CreateChild("HealthBar");
    shieldBarNode_->SetPosition(Vector3(0.0f, 1.0f, 21.0f));
    shieldBarNode_->SetScale(Vector3(health_, 0.9f, 0.9f));
    shieldBarModel_ = shieldBarNode_->CreateComponent<StaticModel>();
    shieldBarModel_->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Bar.mdl"));
    shieldBarModel_->SetMaterial(masterControl_->cache_->GetResource<Material>("Resources/Materials/BlueGlowEnvmap.xml"));

    Node* healthBarHolderNode = guiNode_->CreateChild("HealthBarHolder");
    healthBarHolderNode->SetPosition(Vector3(0.0f, 1.0f, 21.0f));
    StaticModel* healthBarHolderModel = healthBarHolderNode->CreateComponent<StaticModel>();
    healthBarHolderModel->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/BarHolder.mdl"));
    healthBarHolderModel->SetMaterial(masterControl_->cache_->GetResource<Material>("Resources/Materials/Metal.xml"));

    appleCounterRoot_ = guiNode_->CreateChild("AppleCounter");
    for (int a = 0; a < 5; a++){
        appleCounter_[a] = appleCounterRoot_->CreateChild();
        appleCounter_[a]->SetEnabled(false);
        appleCounter_[a]->SetPosition(Vector3(-((float)a + 8.0f), 1.0f, 21.0f));
        appleCounter_[a]->SetScale(0.333f);
        StaticModel* apple = appleCounter_[a]->CreateComponent<StaticModel>();
        apple->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Apple.mdl"));
        apple->SetMaterial(masterControl_->cache_->GetTempResource<Material>("Resources/Materials/GoldEnvmap.xml"));
    }

    heartCounterRoot_ = guiNode_->CreateChild("HeartCounter");
    for (int h = 0; h < 5; h++){
        heartCounter_[h] = heartCounterRoot_->CreateChild();
        heartCounter_[h]->SetEnabled(false);
        heartCounter_[h]->SetPosition(Vector3((float)h + 8.0f, 1.0f, 21.0f));
        heartCounter_[h]->SetScale(0.333f);
        StaticModel* heart = heartCounter_[h]->CreateComponent<StaticModel>();
        heart->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Heart.mdl"));
        heart->SetMaterial(masterControl_->cache_->GetTempResource<Material>("Resources/Materials/RedEnvmap.xml"));
    }
}


void Player::SetScore(int points)
{
    score_ = points;
    UI* ui = GetSubsystem<UI>();
    UIElement* scoreElement = ui->GetRoot()->GetChild(scoreTextName_);
    Text* scoreText = (Text*)scoreElement;
    scoreText->SetText(String(points));
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
void Player::LoadScore()
{
    std::ifstream f_score("Resources/.LucKey.lks");
    std::string score_str;
    f_score >> score_str;
    if (!score_str.empty()){
        unsigned long score = stoul(score_str, 0, 10);
        score_ = score;
    }
    f_score.close();
}
void Player::PlaySample(Sound* sample)
{
    for (unsigned i = 0; i < sampleSources_.Size(); ++i){
        if (!sampleSources_[i]->IsPlaying()){
            sampleSources_[i]->Play(sample);
            break;
        }
    }
}


void Player::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    using namespace Update;

    //Take the frame time step, which is stored as a double
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    //Pulse and spin the counters' apples and hearts
    UpdateGUI(timeStep);

    //Only handle input when player is active
    if (!rootNode_->IsEnabled()) return;

    Input* input = GetSubsystem<Input>();

    //Movement values
    Vector3 move = Vector3::ZERO;
    Vector3 moveJoy = Vector3::ZERO;
    Vector3 moveKey = Vector3::ZERO;
    float thrust = pilotMode_ ? 256.0f : 2323.0f;
    float maxSpeed = pilotMode_? 1.8f : 23.0f;    //Firing values
    Vector3 fire = Vector3::ZERO;
    Vector3 fireJoy = Vector3::ZERO;
    Vector3 fireKey = Vector3::ZERO;

    //Read input
    if (input->GetJoystickByIndex(0)){
        moveJoy = Vector3::RIGHT * input->GetJoystickByIndex(0)->GetAxisPosition(0) +
                Vector3::BACK * input->GetJoystickByIndex(0)->GetAxisPosition(1);
        fireJoy = Vector3::RIGHT * input->GetJoystickByIndex(0)->GetAxisPosition(2) +
                Vector3::BACK * input->GetJoystickByIndex(0)->GetAxisPosition(3);
    }
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

    //Pick most significant input
    moveJoy.Length() > moveKey.Length() ? move = moveJoy : move = moveKey;
    fireJoy.Length() > fireKey.Length() ? fire = fireJoy : fire = fireKey;

    //Restrict move vector length
    if (move.Length() > 1.0f) move /= move.Length();
    //Deadzone
    else if (move.Length() < 0.1f) move *= 0.0f;

    if (fire.Length() < 0.1f) fire *= 0.0f;
    else fire.Normalize();

    //When in pilot mode
    if (pilotMode_){
        //Apply movement
        Vector3 force = move * thrust * timeStep;
        if (rigidBody_->GetLinearVelocity().Length() < maxSpeed ||
                (rigidBody_->GetLinearVelocity().Normalized() + force.Normalized()).Length() < 1.0f) {
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
        if (velocity.Length() > 0.05f){
            animCtrl_->PlayExclusive("Resources/Models/WalkRelax.ani", 0, true, 0.15f);
            animCtrl_->SetSpeed("Resources/Models/WalkRelax.ani", velocity.Length()*2.3f);
            animCtrl_->SetStartBone("Resources/Models/WalkRelax.ani", "MasterBone");
        }
        else {
            animCtrl_->PlayExclusive("Resources/Models/IdleRelax.ani", 0, true, 0.15f);
            animCtrl_->SetStartBone("Resources/Models/IdleRelax.ani", "MasterBone");
        }
    // When in ship mode
    } else {
        //Update shield
        shieldNode_->Rotate(Quaternion(1010.0f*timeStep, 200.0f*timeStep, Random(10.0f, 100.0f)));
        Color shieldColor = shieldMaterial_->GetShaderParameter("MatDiffColor").GetColor();
        shieldMaterial_->SetShaderParameter("MatDiffColor", Color(shieldColor.r_ * Random(0.6f, 0.9f),
                                                                  shieldColor.g_ * Random(0.7f, 0.95f),
                                                                  shieldColor.b_ * Random(0.8f, 0.9f)));
        
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
    for (int i = 0; i < 5; i++){
        appleCounter_[i]->Rotate(Quaternion(0.0f, (i*i+10.0f) * 23.0f * timeStep, 0.0f));
        appleCounter_[i]->SetScale(masterControl_->Sine((1.0f+(appleCount_))*2.0f, 0.2f, 0.4, -i));
        heartCounter_[i]->Rotate(Quaternion(0.0f, (i*i+10.0f) * 23.0f * timeStep, 0.0f));
        heartCounter_[i]->SetScale(masterControl_->Sine((1.0f+(heartCount_))*2.0f, 0.2f, 0.4, -i));
    }
    //Update HealthBar color
    healthBarModel_->GetMaterial()->SetShaderParameter("MatEmissiveColor", HealthToColor(health_));
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
        PlaySample(shot_);
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
        bullet = new Bullet(context_, masterControl_);
        bullets_.Push(bullet);
    }
    bullet->Set(rootNode_->GetPosition() + direction);
    bullet->rootNode_->LookAt(bullet->rootNode_->GetPosition() + direction*5.0f);
    bullet->rigidBody_->ApplyForce(direction*(1500.0f+23.0f*weaponLevel_));
    bullet->damage_ = 0.15f + 0.00666f * weaponLevel_;
}
void Player::MoveMuzzle()
{
    if (muzzle_ == nullptr)
        muzzle_ = new Muzzle(context_, masterControl_);
    muzzle_->Set(rootNode_->GetPosition());
}


void Player::Pickup(PickupType pickup)
{
    switch (pickup) {
    case PT_APPLE: {
        bulletAmount_ = (bulletAmount_ == 0)?1:bulletAmount_;
        ++appleCount_;
        heartCount_ = 0;
        AddScore(23);
        if (appleCount_ >= 5){
            appleCount_ = 0;
            UpgradeWeapons();
        }
    } break;
    case PT_HEART: {
        ++heartCount_;
        appleCount_ = 0;
        if (heartCount_ >= 5){
            heartCount_ = 0;
            SetHealth(15.0);
        }
        else SetHealth(Max(health_, Clamp(health_+5.0f, 0.0f, 10.0f)));
    } break;
    case PT_MULTIX: {
        multiplier_++;
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
    for (int a = 0; a < 5; a++){
        if (appleCount_ > a) appleCounter_[a]->SetEnabled(true);
        else appleCounter_[a]->SetEnabled(false);
    }
    for (int h = 0; h < 5; h++){
        if (heartCount_ > h) heartCounter_[h]->SetEnabled(true);
        else heartCounter_[h]->SetEnabled(false);
    }
}

void Player::PickupChaoBall()
{
//    ship_.model_->GetMaterial(0)->SetShaderParameter("MatDiffColor", Color(Random(), Random(), Random()));
//    ship_.model_->GetMaterial(1)->SetShaderParameter("MatDiffColor", Color(Random(), Random(), Random()));
    chaoFlash_->Set(GetPosition());
}

void Player::Die()
{
    Disable();
    masterControl_->spawnMaster_->SpawnExplosion(rootNode_->GetPosition(), Color::GREEN, 2.0f);
    masterControl_->SetGameState(GS_DEAD);
}

void Player::EnterPlay()
{
    guiNode_->SetEnabledRecursive(true);
    Pickup(PT_RESET);

    rootNode_->SetRotation(Quaternion(180.0f, Vector3::UP));
    rigidBody_->ResetForces();
    rigidBody_->SetLinearVelocity(Vector3::ZERO);
    shieldMaterial_->SetShaderParameter("MatDiffColor", Color::BLACK);

    SetHealth(initialHealth_);
    flightScore_ = 0;
    multiplier_ = 1;
    weaponLevel_ = 0;
    bulletAmount_ = 1;
    shotInterval_ = initialShotInterval_;
    RemoveTails();
    CreateTails();
    Set(Vector3::ZERO);
    SetPilotMode(false);
}
void Player::EnterLobby()
{
    guiNode_->SetEnabledRecursive(false);
    chaoFlash_->Disable();
    SetPilotMode(true);
    rootNode_->SetPosition(Vector3::BACK*7.0f);
    rigidBody_->SetLinearVelocity(Vector3::FORWARD*5.0f);
    rigidBody_->ResetForces();
}
void Player::SetPilotMode(bool pilotMode){
    pilotMode_ = pilotMode;
    rootNode_->SetEnabled(true);
    pilot_.node_->SetEnabledRecursive(pilotMode_);
    ship_.node_->SetEnabledRecursive(!pilotMode_);
    shieldNode_->SetEnabled(!pilotMode_);
    collisionShape_->SetSphere(pilotMode_? 0.666f : 2.0f);
    rigidBody_->SetLinearDamping(pilotMode_? 0.75f : 0.5f);
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
    float maxBright = 0.666f;
    if (health < 5.0f) maxBright = masterControl_->Sine(2.0f+3.0f*(5.0f-health), 0.2f*health, 1.0f);
    color.r_ = Clamp((3.0f - (health - 3.0f))/3.0f, 0.0f, maxBright);
    color.g_ = Clamp((health - 3.0f)/4.0f, 0.0f, maxBright);
    return color;
}

void Player::Hit(float damage, bool melee)
{
    if (health_ > 10.0f){
        damage *= (melee ? 0.9f : 0.5f);
        shieldMaterial_->SetShaderParameter("MatDiffColor", Color(2.0f, 3.0f, 5.0f, 1.0f));
    }
    SetHealth(health_ - damage);
}

void Player::UpgradeWeapons()
{
    if (weaponLevel_ < 23){
        ++weaponLevel_;
        bulletAmount_ = 1 + ((weaponLevel_+5) / 6);
        shotInterval_ = initialShotInterval_ - 0.01f*weaponLevel_;
    }
}

void Player::LoadPilot()
{
    using namespace std;
    ifstream fPilot("Resources/Pilot.lkp");
    while (!fPilot.eof()){
        string gender_str;
        string color1_r_str, color1_g_str, color1_b_str;
        string color2_r_str, color2_g_str, color2_b_str;
        string color3_r_str, color3_g_str, color3_b_str;
        string color4_r_str, color4_g_str, color4_b_str;
        string color5_r_str, color5_g_str, color5_b_str;
        fPilot >> gender_str;
        if (gender_str.empty()) break;
        fPilot >>
                color1_r_str >> color1_g_str >> color1_b_str >>
                color2_r_str >> color2_g_str >> color2_b_str >>
                color3_r_str >> color3_g_str >> color3_b_str >>
                color4_r_str >> color4_g_str >> color4_b_str >>
                color5_r_str >> color5_g_str >> color5_b_str;

        pilot_.male_ = stoi(gender_str);
        pilot_.colors_.Clear();
        pilot_.colors_.Push(Color(stof(color1_r_str),stof(color1_g_str),stof(color1_b_str)));
        pilot_.colors_.Push(Color(stof(color2_r_str),stof(color2_g_str),stof(color2_b_str)));
        pilot_.colors_.Push(Color(stof(color3_r_str),stof(color3_g_str),stof(color3_b_str)));
        pilot_.colors_.Push(Color(stof(color4_r_str),stof(color4_g_str),stof(color4_b_str)));
        pilot_.colors_.Push(Color(stof(color5_r_str),stof(color5_g_str),stof(color5_b_str)));
    }
    if (!pilot_.colors_.Size()) CreateNewPilot();
    UpdatePilot();
}

void Player::UpdatePilot()
{
    if (pilot_.male_){
        pilot_.model_->SetModel(masterControl_->resources.models.pilots.male);}
    else{
        pilot_.model_->SetModel(masterControl_->resources.models.pilots.female);
    }

    for (unsigned m = 0; m < pilot_.model_->GetNumGeometries(); m++){
        pilot_.model_->SetMaterial(m, masterControl_->cache_->GetTempResource<Material>("Resources/Materials/Basic.xml"));
        Color diffColor = pilot_.colors_[m];
        pilot_.model_->GetMaterial(m)->SetShaderParameter("MatDiffColor", diffColor);
        Color specColor = diffColor*(1.0f-0.1f*m);
        specColor.a_ = 23.0f - 2.0f * m;
        pilot_.model_->GetMaterial(m)->SetShaderParameter("MatSpecColor", specColor);
    }
}

void Player::CreateNewPilot()
{
    pilot_.male_ = Random(2);
    pilot_.node_->SetRotation(Quaternion(0.0f, 0.0f, 0.0f));

    pilot_.colors_.Clear();
    for (int c = 0; c < 5; c++)
    {
        switch (c){
        case 0:{
            pilot_.colors_.Push(RandomSkinColor());
        } break;
        case 4:{
            pilot_.colors_.Push(RandomHairColor());
        } break;
        default: pilot_.colors_.Push(RandomColor()); break;
        }
    }
    UpdatePilot();
}

Color Player::RandomHairColor()
{
    Color hairColor{};
    hairColor.FromHSV(Random(0.1666f), Random(0.05f, 0.7f), Random(0.9f));
    return hairColor;
}

Color Player::RandomSkinColor()
{
    Color skinColor{};
    skinColor.FromHSV(Random(0.05f, 0.18f), Random(0.5f, 0.75f), Random(0.23f, 0.8f));
    return skinColor;
}
Color Player::RandomColor()
{
    Color color{};
    color.FromHSV(Random(), Random(), Random());
    return color;
}

void Player::SetupShip()
{
    ship_.node_ = rootNode_->CreateChild("Ship");
    ship_.model_ = ship_.node_->CreateComponent<StaticModel>();
    ship_.model_->SetModel(masterControl_->resources.models.ships.swift);
    ship_.model_->SetMaterial(0, masterControl_->resources.materials.shipSecondary);
    ship_.model_->SetMaterial(1, masterControl_->resources.materials.shipPrimary);

    ParticleEmitter* particleEmitter = ship_.node_->CreateComponent<ParticleEmitter>();
    SharedPtr<ParticleEffect> particleEffect = masterControl_->cache_->GetTempResource<ParticleEffect>("Resources/Particles/Shine.xml");
    Vector<ColorFrame> colorFrames;
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.0f));
    colorFrames.Push(ColorFrame(Color(0.42f, 0.7f, 0.23f, 0.23f), 0.2f));
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
        assert(tailNode);
        tailNode->SetPosition(Vector3(-0.85f+0.85f*n, n==1? 0.0f : -0.5f, n==1? -0.5f : -0.23f));
        TailGenerator* tailGen = tailNode->CreateComponent<TailGenerator>();
        tailGen->SetDrawHorizontal(true);
        tailGen->SetDrawVertical(n==1?true:false);
        tailGen->SetTailLength(n==1? 0.075f : 0.05f);
        tailGen->SetNumTails(n==1? 14 : 12);
        tailGen->SetWidthScale(n==1? 0.666f : 0.23f);
        tailGen->SetColorForHead(Color(0.9f, 1.0f, 0.5f));
        tailGen->SetColorForTip(Color(0.0f, 1.0f, 0.0f));
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
