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

#include "chaoball.h"

#include "spawnmaster.h"

void ChaoBall::RegisterObject(Context *context)
{
    context->RegisterFactory<ChaoBall>();
}

ChaoBall::ChaoBall(Context* context):
    Pickup(context)
{
}

void ChaoBall::OnNodeSet(Node *node)
{
    Pickup::OnNodeSet(node);

    node_->SetName("ChaoBall");
    pickupType_ = PT_CHAOBALL;
    node_->SetRotation(Quaternion(Random(360.0f), Random(360.0f), Random(360.0f)));
    initialPosition_ = Vector3::FORWARD * 5.0f;
    node_->SetPosition(initialPosition_);
    model_->SetModel(MC->GetModel("Chaosphere"));
    model_->SetMaterial(MC->GetMaterial("Chaosphere")->Clone());

    rigidBody_->SetMass(3.0f);

    Vector<ColorFrame> colorFrames;
    colorFrames.Push(ColorFrame(Color(0.0f, 1.0f, 0.0f, 0.0f), 0.0f));
    colorFrames.Push(ColorFrame(Color(0.23f, 0.05f, 0.5f, 0.23f), 0.1f));
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.2f));
    colorFrames.Push(ColorFrame(Color(0.666f, 0.0f, 0.0f, 0.42f), 0.25f));
    colorFrames.Push(ColorFrame(Color(0.0f, 0.666f, 0.0f, 0.42f), 0.3f));
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.666f, 0.42f), 0.35f));
    colorFrames.Push(ColorFrame(Color(0.0f, 0.0f, 0.0f, 0.0f), 0.4f));
    particleEmitter_->GetEffect()->SetColorFrames(colorFrames);

    Disable();

}

void ChaoBall::Update(float timeStep)
{
    Pickup::Update(timeStep);

    //Spin
    node_->Rotate(Quaternion(23.0f  * timeStep,
                             100.0f * timeStep,
                             42.0f  * timeStep));
    //Float like a float
    float floatFactor = 0.5f - Min(0.5f, 0.5f * Abs(node_->GetPosition().y_));
    graphicsNode_->SetPosition(Vector3::UP * MC->Sine(0.5f, -floatFactor, floatFactor, 0.23f));

    Player* player1 = MC->GetPlayer(1);
    Player* player2 = MC->GetPlayer(2);

    if (IsEmerged() && MC->GetGameState() == GS_PLAY){
        Vector3 force{};
        force += player1->IsAlive() * -3.0f*player1->GetPosition() - rigidBody_->GetLinearVelocity();
        force += player2->IsAlive() * -3.0f*player2->GetPosition() - rigidBody_->GetLinearVelocity();
        rigidBody_->ApplyForce(force);
    }
}

void ChaoBall::Deactivate()
{
    Pickup::Deactivate();
}
