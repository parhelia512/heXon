#include "splatterpillar.h"

SplatterPillar::SplatterPillar(Context *context, MasterControl *masterControl, bool right):
    Object(context),
    masterControl_{masterControl},
    right_{right},
    sequenceLength_{1.23f},
    lastTriggered_{-sequenceLength_}
{
    player_ = masterControl_->GetPlayer(right_+1);

    float mirror = right_ ? 1.0f : -1.0f;
    rootNode_ = masterControl->lobbyNode_->CreateChild("SplatterPillar");
    rootNode_->SetPosition(Vector3(mirror * -2.26494f, 0.0f, 3.91992f));
    pillar_ = rootNode_->CreateComponent<AnimatedModel>();
    pillar_->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/SplatterPillar.mdl"));
//    pillar_->SetMorphWeight(0, 0.0f);
    pillar_->SetCastShadows(true);
    pillar_->SetMaterial(0, masterControl_->resources.materials.basic);
    pillar_->SetMaterial(1, masterControl_->cache_->GetResource<Material>("Resources/Materials/Metal.xml"));
    if (!right_) pillar_->SetMaterial(2, masterControl_->cache_->GetResource<Material>("Resources/Materials/GreenGlow.xml"));
    else pillar_->SetMaterial(2, masterControl_->cache_->GetResource<Material>("Resources/Materials/PurpleGlow.xml"));

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(SplatterPillar, HandleSceneUpdate));
}

void SplatterPillar::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    if (masterControl_->GetGameState() != GS_LOBBY) return;

    float elapsedTime = masterControl_->world.scene->GetElapsedTime();
    float intoSequence = (elapsedTime - lastTriggered_)/sequenceLength_;

    if (intoSequence < 1.0f) {
        if      (intoSequence < 0.025f) pillar_->SetMorphWeight(0, 40.0f * intoSequence);
        else if (intoSequence < 0.5f) pillar_->SetMorphWeight(0, 1.0f);
        else if (intoSequence > 0.5f) pillar_->SetMorphWeight(0,  2.0f * (1.0f - intoSequence));
    }
    else if (pillar_->GetMorphWeight(0) != 0.0f) pillar_->SetMorphWeight(0, 0.0f);

    if (LucKey::Distance(player_->GetPosition(), rootNode_->GetWorldPosition()) < 0.23f) {
        lastTriggered_ = elapsedTime;
        player_->KillPilot();
    }
}
