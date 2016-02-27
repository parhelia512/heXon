#include "splatterpillar.h"

SplatterPillar::SplatterPillar(Context *context, MasterControl *masterControl, bool right):
    Object(context),
    masterControl_{masterControl},
    right_{right},
    spun_{false},
    sequenceLength_{2.0f},
    lastTriggered_{-2.0f}
{
    player_ = masterControl_->GetPlayer(right_+1);

    float mirror = right_ ? 1.0f : -1.0f;
    rootNode_ = masterControl->lobbyNode_->CreateChild("SplatterPillar");
    rootNode_->SetPosition(Vector3(mirror * -2.26494f, 0.0f, 3.91992f));
    rootNode_->Rotate(Quaternion(Random(6)*60.0f, Vector3::UP));
    pillarNode_ = rootNode_->CreateChild("Pillar");
    bloodNode_ = rootNode_->CreateChild("Blood");
    pillar_ = pillarNode_->CreateComponent<AnimatedModel>();
    pillar_->SetModel(masterControl_->cache_->GetResource<Model>("Models/SplatterPillar.mdl"));
    pillar_->SetMorphWeight(0, 0.0f);
    pillar_->SetCastShadows(true);
    pillar_->SetMaterial(0, masterControl_->resources.materials.basic);
    if (!right_) pillar_->SetMaterial(1, masterControl_->cache_->GetResource<Material>("Materials/GreenGlow.xml"));
    else pillar_->SetMaterial(1, masterControl_->cache_->GetResource<Material>("Materials/PurpleGlow.xml"));
    pillar_->SetMaterial(2, masterControl_->cache_->GetResource<Material>("Materials/Metal.xml"));
    pillar_->SetMaterial(3, masterControl_->cache_->GetResource<Material>("Materials/Drain.xml"));

    blood_ = bloodNode_->CreateComponent<AnimatedModel>();
    blood_->SetEnabled(false);
    blood_->SetModel(masterControl_->cache_->GetResource<Model>("Models/Blood.mdl"));
    blood_->SetMaterial(0, masterControl_->cache_->GetResource<Material>("Materials/Blood.xml"));

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(SplatterPillar, HandleSceneUpdate));
}

void SplatterPillar::Trigger()
{
    lastTriggered_ = masterControl_->world.scene->GetElapsedTime();
    player_->KillPilot();
    bloodNode_->Rotate(Quaternion(Random(360.0f), Vector3::UP));
    blood_->SetEnabled(true);
}

void SplatterPillar::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    if (masterControl_->GetGameState() != GS_LOBBY) return;

    float elapsedTime = masterControl_->world.scene->GetElapsedTime();
    float intoSequence = (elapsedTime - lastTriggered_)/sequenceLength_;

    if (intoSequence < 1.0f) {
        unsigned numMorphs = blood_->GetNumMorphs();
        for (unsigned m = 0; m < numMorphs; ++m){
            float intoMorph = Max(intoSequence * numMorphs - m, 0.0f);
            if (intoMorph > 1.0f) intoMorph = Max(2.0f - intoMorph, 0.0f);
            blood_->SetMorphWeight(m, intoMorph);
        }

        if      (intoSequence < 0.0125f) pillar_->SetMorphWeight(0, 80.0f * intoSequence);
        else if (intoSequence < 0.05f) {
            pillar_->SetMorphWeight(0, 1.0f);
            if (!spun_){
                pillarNode_->Rotate(Quaternion(Random(6)*60.0f, Vector3::UP));
                spun_ = true;
            }
        }
        else if (intoSequence > 0.05f) {
            spun_ = false;
            pillar_->SetMorphWeight(0,  2.0f * (1.0f - 3.0f*intoSequence));
        }
    }
    else if (pillar_->GetMorphWeight(0) != 0.0f) pillar_->SetMorphWeight(0, 0.0f);

    if (LucKey::Distance(player_->GetPosition(), rootNode_->GetWorldPosition()) < 0.23f) {
        Trigger();
    }
}
