#include "splatterpillar.h"

SplatterPillar::SplatterPillar(Context *context, MasterControl *masterControl, bool right):
    Object(context),
    masterControl_{masterControl},
    right_{right}
{
    float mirror = right ? 1.0f : -1.0f;
    Node* rootNode = masterControl->lobbyNode_->CreateChild("SplatterPillar");
    rootNode->SetPosition(Vector3(mirror * -2.26494f, 0.0f, 3.91992f));
    pillar_ = rootNode->CreateComponent<AnimatedModel>();
    pillar_->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/SplatterPillar.mdl"));
    pillar_->SetCastShadows(true);
    pillar_->SetMaterial(0, masterControl_->resources.materials.basic);
    pillar_->SetMaterial(0, masterControl_->cache_->GetResource<Material>("Resources/Materials/Metal.xml"));

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(SplatterPillar, HandleSceneUpdate));
}

void SplatterPillar::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    pillar_->SetMorphWeight(0, pow(masterControl_->Sine(7.0f, 0.0f, 1.0f, M_PI * right_), 5.0f));
}
