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

#include "ship.h"

void Ship::RegisterObject(Context *context)
{
    context->RegisterFactory<Ship>();
}

Ship::Ship(Context* context) : LogicComponent(context),
    playerID_{1}
{

}

void Ship::OnNodeSet(Node *node)
{ (void)node;

    StaticModel* model{ node_->CreateComponent<StaticModel>() };
    model->SetModel(MC->GetModel("KlåMk10"));
    model->SetMaterial(0, playerID_==2 ? MC->GetMaterial("PurpleGlowEnvmap") : MC->GetMaterial("GreenGlowEnvmap"));
    model->SetMaterial(1, playerID_==2 ? MC->GetMaterial("PurpleEnvmap") : MC->GetMaterial("GreenEnvmap"));

    ParticleEmitter* particleEmitter{node_->CreateComponent<ParticleEmitter>()};
    SharedPtr<ParticleEffect> particleEffect{CACHE->GetTempResource<ParticleEffect>("Particles/Shine.xml")};
    Vector<ColorFrame> colorFrames{};
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.0f));
    colorFrames.Push(ColorFrame(playerID_==2 ? Color(0.42f, 0.0f, 0.88f, 0.05f) : Color(0.42f, 0.7f, 0.23f, 0.23f), 0.05f));
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.4f));
    particleEffect->SetColorFrames(colorFrames);
    particleEmitter->SetEffect(particleEffect);

    CreateTails();
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
        tailGen->SetColorForHead(playerID_==2 ? Color(1.0f, 0.666f, 0.23f) : Color(0.8f, 0.8f, 0.2f));
        tailGen->SetColorForTip(playerID_==2 ? Color(1.0f, 0.23f, 0.0f) : Color(0.23f, 0.6f, 0.0f));
        tailGens_.Push(tailGen);
    }
}

void Ship::Update(float timeStep)
{/*
    //Update shield
    Quaternion randomRotation = Quaternion(Random(360.0f),Random(360.0f),Random(360.0f));
    shieldNode_->SetRotation(shieldNode_->GetRotation().Slerp(randomRotation, Random(1.0f)));
    Color shieldColor = shieldMaterial_->GetShaderParameter("MatDiffColor").GetColor();
    Color newColor = Color(shieldColor.r_ * Random(0.6f, 0.9f),
                           shieldColor.g_ * Random(0.7f, 0.95f),
                           shieldColor.b_ * Random(0.8f, 0.9f));
    shieldMaterial_->SetShaderParameter("MatDiffColor", shieldColor.Lerp(newColor, Min(timeStep * 23.5f, 1.0f)));

    //Float
//        ship_.node_->SetPosition(Vector3::UP *MC->Sine(2.3f, -0.1f, 0.1f));
    //Apply movement
    Vector3 force = move * thrust * timeStep;
    if (rigidBody_->GetLinearVelocity().Length() < maxSpeed ||
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
    if (fire.Length()) {
        if (sinceLastShot_ > shotInterval_)
        {
            Shoot(fire);
        }
    }*/
}
