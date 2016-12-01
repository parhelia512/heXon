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

#include "effectmaster.h"
#include "inputmaster.h"
#include "ship.h"
#include "splatterpillar.h"

#include "pilot.h"

void Pilot::RegisterObject(Context *context)
{
    context->RegisterFactory<Pilot>();
}

Pilot::Pilot(Context* context) : Controllable(context),
    male_{ false },
    alive_{ true },
    deceased_{ 0.0f },
    autoPilot_{ false },
    hairStyle_{ 0 },
    pilotColors_{}
{
    thrust_ = 256.0f;
    maxSpeed_ = 1.23f;
}

void Pilot::OnNodeSet(Node *node)
{
    Controllable::OnNodeSet(node);

    model_->SetModel(MC->GetModel("Male"));
    model_->SetCastShadows(true);
    Node* head{ node_->GetChild("Head", true) };
    Node* hairNode{ head->CreateChild("Hair") };
    hairModel_ = hairNode->CreateComponent<AnimatedModel>();
    hairModel_->SetCastShadows(true);
    rigidBody_->SetMass(1.0f);
    rigidBody_->SetRestitution(0.0f);
    rigidBody_->SetLinearFactor(Vector3::ONE - Vector3::UP);
    rigidBody_->SetLinearDamping(0.88f);
    rigidBody_->SetLinearRestThreshold(0.0f);
    rigidBody_->SetAngularFactor(Vector3::ZERO);
    rigidBody_->SetAngularRestThreshold(0.1f);
    collisionShape_->SetCapsule(0.23f, 0.5f);

    //Also animates highest
    animCtrl_->PlayExclusive("Models/IdleAlert.ani", 0, true);
    animCtrl_->SetSpeed("Models/IdleAlert.ani", 0.5f);
    animCtrl_->SetStartBone("Models/IdleAlert.ani", "MasterBone");

    SubscribeToEvent(node_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Pilot, HandleNodeCollisionStart));

}

void Pilot::Update(float timeStep)
{
    Controllable::Update(timeStep);

    if (node_->GetName() == "HighestPilot")
        return;

    if (!alive_){
        deceased_ += timeStep;
        if (deceased_ > 0.125f)
            Revive();

        return;
    }

    //Apply movement
    Vector3 force{ move_ * thrust_ * timeStep };
    if ( rigidBody_->GetLinearVelocity().Length() < maxSpeed_
     || (rigidBody_->GetLinearVelocity().Normalized() + force.Normalized()).Length() < 1.4142f )
    {
        rigidBody_->ApplyForce(force);
    }

    //Update rotation according to direction of the player's movement.
    Vector3 velocity{ rigidBody_->GetLinearVelocity() };
    Vector3 lookDirection{ velocity + 2.0f * aim_ };
    Quaternion rotation{ node_->GetWorldRotation() };
    Quaternion aimRotation{ rotation };
    aimRotation.FromLookRotation(lookDirection);
    node_->SetRotation(rotation.Slerp(aimRotation, 7.0f * timeStep * velocity.Length()));

    //Update animation
    if (velocity.Length() > 0.1f){
        animCtrl_->PlayExclusive("Models/WalkRelax.ani", 1, true, 0.15f);
        animCtrl_->SetSpeed("Models/WalkRelax.ani", velocity.Length() * 2.3f);
        animCtrl_->SetStartBone("Models/WalkRelax.ani", "MasterBone");
    }
    else {
        animCtrl_->PlayExclusive("Models/IdleRelax.ani", 1, true, 0.15f);
        animCtrl_->SetStartBone("Models/IdleRelax.ani", "MasterBone");
    }
}

void Pilot::Initialize(int playerId)
{
    playerId_ = playerId;
    Load();
}

void Pilot::Save(int playerID, unsigned score)
{
    using namespace std;
    ofstream fPilot{};
    fPilot.open("Resources/.Pilot" + to_string(playerID) + ".lkp");
    fPilot << male_ << '\n';
    fPilot << hairStyle_ << '\n';
    for (Color c : pilotColors_.Values()) {
        fPilot << c.r_ << ' '
               << c.g_ << ' '
               << c.b_ << ' '
               << '\n';
    }
    fPilot << score;
}

void Pilot::Load()
{
    using namespace std;

    ifstream fPilot{ "Resources/.Pilot" + to_string(playerId_) + ".lkp" };
    while (!fPilot.eof()){
        string gender_str{};
        string hairStyle_str{};
        string color1_r_str{}, color1_g_str{}, color1_b_str{};
        string color2_r_str{}, color2_g_str{}, color2_b_str{};
        string color3_r_str{}, color3_g_str{}, color3_b_str{};
        string color4_r_str{}, color4_g_str{}, color4_b_str{};
        string color5_r_str{}, color5_g_str{}, color5_b_str{};
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
        pilotColors_.Clear();
        pilotColors_[PC_SKIN]   = Color(stof(color1_r_str), stof(color1_g_str), stof(color1_b_str));
        pilotColors_[PC_SHIRT]  = Color(stof(color2_r_str), stof(color2_g_str), stof(color2_b_str));
        pilotColors_[PC_PANTS]  = Color(stof(color3_r_str), stof(color3_g_str), stof(color3_b_str));
        pilotColors_[PC_SHOES]  = Color(stof(color4_r_str), stof(color4_g_str), stof(color4_b_str));
        pilotColors_[PC_HAIR]   = Color(stof(color5_r_str), stof(color5_g_str), stof(color5_b_str));

        score_ = static_cast<unsigned>(stoul(score_str, 0, 10));
    }

    if (!pilotColors_.Size() || score_ == 0)
        Randomize();

    UpdateModel();

    if (!(node_->GetName() == "HighestPilot"))
        EnterLobbyThroughDoor();
}

void Pilot::UpdateModel()
{
    maxSpeed_ = 1.23f + 0.5f * pilotColors_[static_cast<int>(PC_SHOES)].r_;

    //Set body model
    if (male_)  model_->SetModel(MC->GetModel("Male"));
    else        model_->SetModel(MC->GetModel("Female"));

    //Set colors for body model
    for (unsigned c{PC_SKIN}; c < PC_ALL; ++c){

        model_->SetMaterial(c, MC->GetMaterial("Basic")->Clone());
        Color diffColor{ pilotColors_[c] };
        if (c == 4){

            if (hairStyle_ == HAIR_BALD)
                diffColor = pilotColors_[0];
            else if (hairStyle_ == HAIR_MOHAWK)
                diffColor = LucKey::RandomHairColor(true);
        }
        model_->GetMaterial(c)->SetShaderParameter("MatDiffColor", diffColor);
        Color specColor{ diffColor * (1.0f-0.1f*c) };
        specColor.a_ = 23.0f - 4.0f * c;
        model_->GetMaterial(c)->SetShaderParameter("MatSpecColor", specColor);
    }

    //Set hair model
    hairModel_->GetNode()->SetScale(1.0f - (0.1f * !male_));

    switch (hairStyle_){
    default: case HAIR_BALD: case HAIR_SHORT: hairModel_->SetModel(nullptr);
        break;
    case HAIR_MOHAWK: hairModel_->SetModel(MC->GetModel("Mohawk"));
        break;
    case HAIR_SEAGULL: hairModel_->SetModel(MC->GetModel("Seagull"));
        break;
    case HAIR_MUSTAIN: hairModel_->SetModel(MC->GetModel("Mustain"));
        break;
    case HAIR_FROTOAD: hairModel_->SetModel(MC->GetModel("Frotoad"));
        break;
    case HAIR_FLATTOP: hairModel_->SetModel(MC->GetModel("Flattop"));
        break;
    }
    //Set hair color
    if (hairStyle_ != HAIR_BALD && hairStyle_ != HAIR_SHORT)
    {
        hairModel_->SetMorphWeight(0, Random());

        //Set color for hair model
        hairModel_->SetMaterial(MC->GetMaterial("Basic")->Clone());
        Color diffColor{ pilotColors_[4] };
        hairModel_->GetMaterial()->SetShaderParameter("MatDiffColor", diffColor);
        Color specColor{ diffColor * 0.23f };
        specColor.a_ = 23.0f;
        hairModel_->GetMaterial()->SetShaderParameter("MatSpecColor", specColor);
    }
}

void Pilot::Randomize()
{
    male_ = Random(2);
    hairStyle_ = Random(static_cast<int>(HAIR_ALL));

    for (int c{PC_SKIN}; c < PC_ALL; ++c) {
        switch (c){
        case 0:
            pilotColors_[c] = (autoPilot_
                               ? Color::GRAY * 0.666f
                               : LucKey::RandomSkinColor());
            break;
        case 4:
            pilotColors_[c] = LucKey::RandomHairColor();
            break;
        default: pilotColors_[c] = LucKey::RandomColor();
            break;
        }
    }
    UpdateModel();
}

void Pilot::Upload()
{
    GetSubsystem<EffectMaster>()->TranslateTo(node_, node_->GetPosition() + 2.0f * Vector3::UP, 0.1f);
    GetSubsystem<EffectMaster>()->ScaleTo(node_, Vector3::ONE - Vector3::UP * 0.9f, 0.125f);
    alive_ = false;
    animCtrl_->SetSpeed("Models/WalkRelax.ani", 0.0f);
    animCtrl_->SetSpeed("Models/IdleRelax.ani", 0.0f);
    rigidBody_->SetKinematic(true);
}

void Pilot::Die()
{
    alive_ = false;
}

void Pilot::EnterLobbyThroughDoor()
{
    node_->SetEnabledRecursive(true);

    node_->SetPosition(Vector3( node_->GetPosition().x_ < 0.0f ? -2.3f : 2.3f, 0.0f, 5.666f));
    node_->SetRotation(Quaternion::IDENTITY);
    rigidBody_->ApplyImpulse(Vector3::BACK * 3.666f);
}
void Pilot::EnterLobbyFromShip()
{
    node_->SetEnabledRecursive(true);

    node_->SetPosition(Vector3(node_->GetPosition().x_ < 0.0f ? -2.7f : 2.7f, 0.0f, 0.0f));
    node_->SetRotation(Quaternion::IDENTITY);
    rigidBody_->ApplyImpulse(node_->GetPosition().x_ < 0.0f ? Vector3::RIGHT
                                                            : Vector3::LEFT);
}

void Pilot::Revive()
{
    Randomize();
    alive_ = true;
    GetPlayer()->Respawn();

    node_->SetAttributeAnimation("Position", nullptr);
    node_->SetAttributeAnimation("Scale", nullptr);
    rigidBody_->SetKinematic(false);
    rigidBody_->SetLinearVelocity(Vector3::ZERO);
    rigidBody_->ResetForces();

    node_->SetScale(Vector3::ONE);
    animCtrl_->SetSpeed("Models/IdleRelax.ani", 1.0f);

    if (GetSubsystem<InputMaster>()->GetControllableByPlayer(playerId_) != this)
        GetSubsystem<InputMaster>()->SetPlayerControl(playerId_, this);

    EnterLobbyThroughDoor();
}

//Will make pilot fall over when colliding with door edge or occasionally on the grate
void Pilot::Trip(bool rightFoot)
{
    if (rightFoot) {
        //trip at right foot
    } else {
        //trip at left foot
    }
}

void Pilot::ClearControl()
{
    Controllable::ClearControl();

    node_->SetEnabledRecursive(false);
}

void Pilot::HandleNodeCollisionStart(StringHash eventType, VariantMap& eventData)
{ (void)eventType;

    Node* otherNode{ static_cast<Node*>(eventData[NodeCollisionStart::P_OTHERNODE].GetPtr()) };

    Ship* ship{ otherNode->GetComponent<Ship>() };
    if (ship)
    {
        GetSubsystem<InputMaster>()->SetPlayerControl(playerId_, ship);
    }
}

void Pilot::Think()
{
    Player* otherPlayer{ MC->GetPlayer(GetPlayer()->GetPlayerId() == 1 ? 2 : 1) };

    SplatterPillar* splatterPillar{};
    for (SplatterPillar* s : MC->GetComponentsRecursive<SplatterPillar>())
        if (s->GetPlayerId() == GetPlayer()->GetPlayerId()){
            splatterPillar = s;
            break;
        }

    bool splatterPillarsIdle{ splatterPillar->IsIdle() };
    Vector3 toPillar{ splatterPillar->GetPosition() - GetPosition() * static_cast<float>(splatterPillar->IsIdle()) };


//    if (MC->NoHumans()){

        //Enter play
    if ((MC->NoHumans() && GetPlayer()->GetScore() == 0 && otherPlayer->GetScore() == 0 && splatterPillarsIdle)
            || MC->AllReady(true))
        SetMove(2.3f * (GetPlayer()->GetPlayerId() == 2 ? Vector3::RIGHT : Vector3::LEFT) - GetPosition());
    //Reset Score
    else if (GetPlayer()->GetScore() != 0 && otherPlayer->GetScore() == 0)
        SetMove(toPillar);
    //Stay put)
    else
        SetMove(Vector3::ZERO);
    //    }
    //Reset Score
    //Exit
//    else if (DOOR->HidesPlayer() == 0.0f && OTHERDOOR->HidesPlayer() > 0.1f * playerFactor)
//        move_ = Vector3::FORWARD;
    SetAim(Vector3::ZERO);
             //Decide which pickup to pick up
}
