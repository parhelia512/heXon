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

#include "pilot.h"

Pilot::Pilot(Node* parent) : SceneObject(),
    male_{false},
    hairStyle_{0},
    colors_{}
{
    Initialize(parent);

    Randomize();
}


Pilot::Pilot(Node* parent, const std::string file, unsigned& score) : SceneObject()
{
    Initialize(parent);

    Load(file, score);

    score = 0;
}

void Pilot::Initialize(Node* parent)
{
    rootNode_ = parent->CreateChild("Pilot");
    bodyModel_ = rootNode_->CreateComponent<AnimatedModel>();
    bodyModel_->SetModel(MC->resources.models.pilots.male);
    bodyModel_->SetCastShadows(true);
    hairModel_ = rootNode_->GetChild("Head", true)->CreateComponent<StaticModel>();
    hairModel_->SetCastShadows(true);
    animCtrl_ = rootNode_->CreateComponent<AnimationController>();

    //Animate highest pilot
    animCtrl_->PlayExclusive("Models/IdleAlert.ani", 0, true);
    animCtrl_->SetSpeed("Models/IdleAlert.ani", 0.5f);
    animCtrl_->SetStartBone("Models/IdleAlert.ani", "MasterBone");
}

void Pilot::Save(int playerID, unsigned score)
{
    using namespace std;
    ofstream fPilot{};
    fPilot.open("Resources/.Pilot" + to_string(playerID) + ".lkp");
    fPilot << male_ << '\n';
    fPilot << hairStyle_ << '\n';
    for (Color c : colors_) {
        fPilot << c.r_ << ' '
               << c.g_ << ' '
               << c.b_ << ' '
               << '\n';
    }
    fPilot << score;
}

void Pilot::Load(std::string file, unsigned &score)
{
    using namespace std;
    ifstream fPilot{file};
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

        male_ = static_cast<bool>(stoi(gender_str));
        hairStyle_ = stoi(hairStyle_str);
        colors_.Clear();
        colors_.Push(Color(stof(color1_r_str),stof(color1_g_str),stof(color1_b_str)));
        colors_.Push(Color(stof(color2_r_str),stof(color2_g_str),stof(color2_b_str)));
        colors_.Push(Color(stof(color3_r_str),stof(color3_g_str),stof(color3_b_str)));
        colors_.Push(Color(stof(color4_r_str),stof(color4_g_str),stof(color4_b_str)));
        colors_.Push(Color(stof(color5_r_str),stof(color5_g_str),stof(color5_b_str)));

        score = static_cast<unsigned>(stoul(score_str, 0, 10));
    }

    if (!colors_.Size() || score == 0)
        Randomize();

    UpdateModel();
}

void Pilot::UpdateModel()
{
    //Set body model
    if (male_)
        bodyModel_->SetModel(MC->resources.models.pilots.male);
    else
        bodyModel_->SetModel(MC->resources.models.pilots.female);

    //Set colors for body model
    for (unsigned m{0}; m < bodyModel_->GetNumGeometries(); ++m){
        bodyModel_->SetMaterial(m, MC->cache_->GetTempResource<Material>("Materials/Basic.xml"));
        Color diffColor{colors_[m]};
        bodyModel_->GetMaterial(m)->SetShaderParameter("MatDiffColor", diffColor);
        Color specColor{diffColor*(1.0f-0.1f*m)};
        specColor.a_ = 23.0f - 2.0f * m;
        bodyModel_->GetMaterial(m)->SetShaderParameter("MatSpecColor", specColor);
    }

    //Set hair model
    if (!hairStyle_)
        hairModel_->SetModel(nullptr);
    else {
        hairModel_->SetModel(MC->resources.models.pilots.hairStyles[hairStyle_ - 1]);
        //Set color for hair model
        hairModel_->SetMaterial(MC->cache_->GetTempResource<Material>("Materials/Basic.xml"));
        Color diffColor{colors_[4]};
        hairModel_->GetMaterial()->SetShaderParameter("MatDiffColor", diffColor);
        Color specColor{diffColor * 0.23f};
        specColor.a_ = 23.0f;
        hairModel_->GetMaterial()->SetShaderParameter("MatSpecColor", specColor);
    }
}

void Pilot::Randomize(bool autoPilot)
{
    male_ = Random(2);
    hairStyle_ = Random(static_cast<int>(MC->resources.models.pilots.hairStyles.Size() + 1));

    rootNode_->SetRotation(Quaternion(0.0f, 0.0f, 0.0f));

    colors_.Clear();
    for (int c{0}; c < 5; ++c) {
        switch (c){
        case 0:{
            colors_.Push(autoPilot
                         ? Color::GRAY
                         : LucKey::RandomSkinColor());
        } break;
        case 4:{
            colors_.Push(LucKey::RandomHairColor());
        } break;
        default: colors_.Push(LucKey::RandomColor()); break;
        }
    }
    UpdateModel();
}
