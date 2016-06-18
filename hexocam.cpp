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

#include "hexocam.h"

void heXoCam::RegisterObject(Context *context)
{
    context->RegisterFactory<heXoCam>();
}

heXoCam::heXoCam(Context* context):
    LogicComponent(context),
    yaw_{0.0f},
    pitch_{0.0f},
    yawDelta_{0.0f},
    pitchDelta_{0.0f},
    forceMultiplier{1.0f}
{
}

void heXoCam::OnNodeSet(Node *node)
{ (void)node;

    /* Ready for VR :)
    Node* leftEye{ node_->CreateChild("Left Eye") };
    leftEye->SetPosition(Vector3::LEFT);
    stereoCam_.first_ = leftEye->CreateComponent<Camera>();
    Node* rightEye{ node_->CreateChild("Right Eye") };
    rightEye->SetPosition(Vector3::RIGHT);
    stereoCam_.second_ = rightEye->CreateComponent<Camera>();
    */

    camera_ = node_->CreateComponent<Camera>();
    camera_->SetFarClip(128.0f);
    node_->SetPosition(Vector3(0.0f, 42.0f, -23.0f));
    node_->SetRotation(Quaternion(65.0f, 0.0f, 0.0f));
    rigidBody_ = node_->CreateComponent<RigidBody>();
    rigidBody_->SetAngularDamping(10.0f);
    CollisionShape* collisionShape{node_->CreateComponent<CollisionShape>()};
    collisionShape->SetSphere(0.1f);
    rigidBody_->SetMass(1.0f);

    SetupViewport();

}

void heXoCam::Start()
{
}

void heXoCam::Stop()
{
}

void heXoCam::SetupViewport()
{
    ResourceCache* cache{GetSubsystem<ResourceCache>()};
    Renderer* renderer{GetSubsystem<Renderer>()};

    SharedPtr<Viewport> viewport{new Viewport(MC->GetContext(), MC->scene_, camera_)};
    viewport_ = viewport;

    //Add anti-asliasing, bloom and a greyscale effects
    effectRenderPath_ = viewport_->GetRenderPath()->Clone();
    effectRenderPath_->Append(cache->GetResource<XMLFile>("PostProcess/FXAA3.xml"));
    effectRenderPath_->SetEnabled("FXAA3", true);
    effectRenderPath_->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
    effectRenderPath_->SetShaderParameter("BloomHDRThreshold", 0.42f);
    effectRenderPath_->SetShaderParameter("BloomHDRMix", Vector2(1.75f, 2.25f));
    effectRenderPath_->SetEnabled("BloomHDR", true);
    effectRenderPath_->Append(cache->GetResource<XMLFile>("PostProcess/GreyScale.xml"));
    SetGreyScale(false);
    viewport_->SetRenderPath(effectRenderPath_);
    renderer->SetViewport(0, viewport_);

}

Vector3 heXoCam::GetWorldPosition()
{
    return node_->GetWorldPosition();
}

Quaternion heXoCam::GetRotation()
{
    return node_->GetRotation();
}

void heXoCam::Update(float timeStep)
{
    node_->SetPosition(node_->GetPosition().Lerp(closeUp_ ?
                                     Vector3(0.0f, 13.5f, -6.23f):
                                     Vector3(0.0f, 42.0f, -23.0f),
                                                    Clamp(5.0f * timeStep, 0.0f, 1.0f)));
}

void heXoCam::SetGreyScale(const bool enabled)
{
    effectRenderPath_->SetEnabled("GreyScale", enabled);
}

void heXoCam::EnterLobby(){
    closeUp_ = true;
    effectRenderPath_->SetShaderParameter("BloomHDRThreshold", 0.42f);
}
void heXoCam::EnterPlay(){
    closeUp_ = false;
    effectRenderPath_->SetShaderParameter("BloomHDRThreshold", 0.32f);
}
