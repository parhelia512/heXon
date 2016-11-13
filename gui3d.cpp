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

#include "gui3d.h"


GUI3D::GUI3D(Context* context) : LogicComponent(context),
    appleCount_{0},
    heartCount_{0}
{
}

void GUI3D::RegisterObject(Context* context)
{
    context->RegisterFactory<GUI3D>();
}


void GUI3D::OnNodeSet(Node* node)
{ (void)node;

}
void GUI3D::Initialize(int playerId)
{
    playerId_ = playerId;

    scoreNode_ = node_->CreateChild("Score");
    for (int d{0}; d < 10; ++d){
        scoreDigits_[d] = scoreNode_->CreateChild("Digit");
        scoreDigits_[d]->SetEnabled( d == 0 );
        scoreDigits_[d]->Translate(Vector3::RIGHT * (playerId == 2 ? -0.5f : 0.5f) * d);
        scoreDigits_[d]->Rotate(Quaternion(playerId == 2 ? 0.0f : 180.0f, Vector3::UP), TS_WORLD);
        StaticModel* digitModel = scoreDigits_[d]->CreateComponent<StaticModel>();
        digitModel->SetModel(MC->GetModel("0"));
        digitModel->SetMaterial(playerId==2
                                ? MC->GetMaterial("PurpleGlowEnvmap")
                                : MC->GetMaterial("GreenGlowEnvmap"));
    }
    scoreNode_->SetPosition(Vector3(playerId == 2 ? 5.94252f : -5.94252f, 0.9f, 0.82951f));
    scoreNode_->Rotate(Quaternion(-90.0f, Vector3::RIGHT));
    scoreNode_->Rotate(Quaternion(playerId == 2 ? 60.0f : -60.0f, Vector3::UP), TS_WORLD);

    Model* barModel = (playerId == 2)
            ? MC->GetModel("BarRight")
            : MC->GetModel("BarLeft");

    healthBarNode_ = node_->CreateChild("HealthBar");
    healthBarNode_->SetPosition(Vector3(0.0f, 1.0f, 21.0f));
    healthBarModel_ = healthBarNode_->CreateComponent<StaticModel>();
    healthBarModel_->SetModel(barModel);
    healthBarModel_->SetMaterial(MC->GetMaterial("GreenGlowEnvmap")->Clone());

    shieldBarNode_ = node_->CreateChild("HealthBar");
    shieldBarNode_->SetPosition(Vector3(0.0f, 1.0f, 21.0f));
    shieldBarModel_ = shieldBarNode_->CreateComponent<StaticModel>();
    shieldBarModel_->SetModel(barModel);
    shieldBarModel_->SetMaterial(MC->GetMaterial("BlueGlowEnvmap"));

    Node* healthBarHolderNode{ node_->CreateChild("HealthBarHolder") };
    healthBarHolderNode->SetPosition(Vector3(0.0f, 1.0f, 21.0f));
    StaticModel* healthBarHolderModel = healthBarHolderNode->CreateComponent<StaticModel>();
    healthBarHolderModel->SetModel(MC->GetModel("BarHolder"));
    healthBarHolderModel->SetMaterial(MC->GetMaterial("BarHolder"));

    appleCounterRoot_ = node_->CreateChild("AppleCounter");
    for (int a{0}; a < 4; ++a){

        appleCounter_[a] = appleCounterRoot_->CreateChild();
        appleCounter_[a]->SetEnabled(false);
        appleCounter_[a]->SetPosition(Vector3(playerId == 2 ? (a + 8.0f) : -(a + 8.0f), 1.0f, 21.0f));
        appleCounter_[a]->SetScale(0.333f);
        StaticModel* apple{ appleCounter_[a]->CreateComponent<StaticModel>() };
        apple->SetModel(MC->GetModel("Apple"));
        apple->SetMaterial(MC->GetMaterial("GoldEnvmap"));
    }

    heartCounterRoot_ = node_->CreateChild("HeartCounter");
    for (int h{0}; h < 4; ++h){

        heartCounter_[h] = heartCounterRoot_->CreateChild();
        heartCounter_[h]->SetEnabled(false);
        heartCounter_[h]->SetPosition(Vector3(playerId == 2 ? (h + 8.0f) : -(h + 8.0f), 1.0f, 21.0f));
        heartCounter_[h]->SetScale(0.333f);
        StaticModel* heart{ heartCounter_[h]->CreateComponent<StaticModel>() };
        heart->SetModel(MC->GetModel("Heart"));
        heart->SetMaterial(MC->GetMaterial("RedEnvmap"));
    }
}

void GUI3D::Update(float timeStep)
{
    for (int i{0}; i < 4; ++i){
        appleCounter_[i]->Rotate(Quaternion(0.0f, (i * i + 10.0f) * 23.0f * timeStep, 0.0f));
        appleCounter_[i]->SetScale(MC->Sine((0.023f + (appleCount_)) * 0.5f, 0.2f, 0.4, -i * 2.3f));
        heartCounter_[i]->Rotate(Quaternion(0.0f, (i * i + 10.0f) * 23.0f * timeStep, 0.0f));
        heartCounter_[i]->SetScale(MC->Sine((0.023f + (heartCount_)) * 0.5f, 0.2f, 0.4, -i * 2.3f));
    }

    //Update HealthBar color
    healthBarModel_->GetMaterial()->SetShaderParameter("MatEmissiveColor", HealthToColor(health_));
    healthBarModel_->GetMaterial()->SetShaderParameter("MatSpecularColor", HealthToColor(health_));
}

void GUI3D::SetHealth(float health)
{
    health_ = health;

    healthBarNode_->SetScale(Vector3(Min(health_, 10.0f), 1.0f, 1.0f));
    shieldBarNode_->SetScale(Vector3(health_, 0.95f, 0.95f));
}
void GUI3D::SetHeartsAndApples(int hearts, int apples)
{
    appleCount_ = apples;
    heartCount_ = hearts;

    for (int a{0}; a < 4; ++a){
        appleCounter_[a]->SetEnabled(appleCount_ > a);
    }
    for (int h{0}; h < 4; ++h){
        heartCounter_[h]->SetEnabled(heartCount_ > h);
    }
}
void GUI3D::SetScore(unsigned score)
{
    score_ = score;
    //Update score graphics
    for (int d{0}; d < 10; ++d){
        StaticModel* digitModel{scoreDigits_[d]->GetComponent<StaticModel>()};
        digitModel->SetModel(MC->GetModel(String(
                             static_cast<int>(score_ / static_cast<unsigned>(pow(10, d)))%10 )));

        scoreDigits_[d]->SetEnabled( score_ >= static_cast<unsigned>(pow(10, d)) || d == 0 );
    }
}

void GUI3D::EnterLobby()
{
    scoreNode_->SetWorldScale(1.0f);
    scoreNode_->SetPosition(Vector3(playerId_ == 2 ? 5.94252f : -5.94252f, 0.9f, 0.82951f));
}
void GUI3D::EnterPlay()
{
    scoreNode_->SetPosition(Vector3(playerId_ == 2 ? 23.5f : -23.5f, 2.23f, 1.23f));
    if (MC->GetAspectRatio() > 1.5f)
        scoreNode_->SetWorldScale(4.2f);
    else {
        scoreNode_->SetWorldScale(3.666f);
        scoreNode_->Translate((playerId_ == 2 ? Vector3::LEFT : Vector3::RIGHT) * 2.3f);
    }
}

Color GUI3D::HealthToColor(float health)
{
    Color color(1.0f, 1.0f, 0.05f, 1.0f);
    health = Clamp(health, 0.0f, 10.0f);
    float maxBright{1.0f};
    if (health < 5.0f) maxBright = MC->Sine(0.2f + 0.3f * (5.0f-health), 0.2f*health, 1.0f);
    color.r_ = Clamp((3.0f - (health - 3.0f))/3.0f, 0.0f, maxBright);
    color.g_ = Clamp((health - 3.0f)/4.0f, 0.0f, maxBright);
    return color;
}
