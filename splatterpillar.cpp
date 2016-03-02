#include "splatterpillar.h"
#include "player.h"

SplatterPillar::SplatterPillar(Context *context, MasterControl *masterControl, bool right):
    Object(context),
    masterControl_{masterControl},
    right_{right},
    spun_{false},
    reset_{true},
    sequenceLength_{5.0f},
    lastTriggered_{-5.0f},
    rotationSpeed_{}
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
    blood_->SetCastShadows(true);
    blood_->SetModel(masterControl_->cache_->GetResource<Model>("Models/Blood.mdl"));
    blood_->SetMaterial(0, masterControl_->cache_->GetResource<Material>("Materials/Blood.xml"));

    Node* particleNode = rootNode_->CreateChild("BloodParticles");
    particleNode->Translate(Vector3::UP*2.3f);
    splatEmitter_ = particleNode->CreateComponent<ParticleEmitter>();
    splatEmitter_->SetEffect(masterControl_->cache_->GetResource<ParticleEffect>("Particles/BloodSplat.xml"));
    splatEmitter_->SetEmitting(false);
    dripEmitter_ = particleNode->CreateComponent<ParticleEmitter>();
    dripEmitter_->SetEffect(masterControl_->cache_->GetResource<ParticleEffect>("Particles/BloodDrip.xml"));
    dripEmitter_->SetEmitting(false);

    soundSource_ = rootNode_->CreateComponent<SoundSource>();
    soundSource_->SetGain(3.0f);

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(SplatterPillar, HandleSceneUpdate));
}

void SplatterPillar::Trigger()
{
    rotationSpeed_ = Random(-1.0f, 1.0f);
    lastTriggered_ = masterControl_->world.scene->GetElapsedTime();
    player_->KillPilot();
    bloodNode_->Rotate(Quaternion(Random(360.0f), Vector3::UP));
    blood_->SetEnabled(true);
    soundSource_->Play(masterControl_->cache_->GetResource<Sound>("Samples/Splatter" + String(Random(1,6)) + ".ogg"));
}

void SplatterPillar::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    if (masterControl_->GetGameState() != GS_LOBBY) return;

    float elapsedTime = masterControl_->world.scene->GetElapsedTime();
    float intoSequence = (elapsedTime - lastTriggered_)/sequenceLength_;
    unsigned numMorphs = blood_->GetNumMorphs();

    //Animate morphs
    if (intoSequence < 1.0f) {
        if (!bloodNode_->IsEnabled()){
            bloodNode_->SetEnabled(true);
            splatEmitter_->SetEmitting(true);
        }
        if (intoSequence > 0.023f) {
            splatEmitter_->SetEmitting(false);
            dripEmitter_->SetEmitting(true);
        }
        //Animate blood
        for (unsigned m = 0; m < numMorphs; ++m){
            float intoMorph = Clamp(intoSequence * numMorphs - m, 0.0f, 2.0f);
            if (intoMorph > 1.0f) intoMorph = Max(2.0f - intoMorph, 0.0f);
            else if (m == 0) Min(intoMorph *= 23.0f, 1.0f);
            blood_->SetMorphWeight(m, intoMorph);
            bloodNode_->Rotate(Quaternion(rotationSpeed_ * eventData[SceneUpdate::P_TIMESTEP].GetFloat() / (1.0f + intoSequence * 23.0f), Vector3::UP));
        }
        blood_->GetMaterial()->SetShaderParameter("MatDiffColor", Color(0.23f, 0.32f, 0.32f, Clamp(1.0f - (intoSequence - 0.88f) * 7.0f, 0.0f, 1.0f)));
        ParticleEffect* dripEffect = dripEmitter_->GetEffect();
        dripEffect->SetEmitterSize(Vector3(1.23f - intoSequence, 0.0f, 1.23f - intoSequence));
        dripEffect->SetMinEmissionRate(Max(100.0f - 123.0f * intoSequence, 0.0f));
        dripEffect->SetMaxEmissionRate(Max(500.0f - 512.0f * intoSequence, 0.0f));
        //Animate pillar
        if      (intoSequence < 0.125f) pillar_->SetMorphWeight(0, 80.0f * intoSequence);
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
    //Trigger
    else {
        if (bloodNode_->IsEnabled()) {
            bloodNode_->SetEnabled(false);
            dripEmitter_->SetEmitting(false);
        }
        if (pillar_->GetMorphWeight(0) != 0.0f) pillar_->SetMorphWeight(0, 0.0f);
        if (LucKey::Distance(player_->GetPosition(), rootNode_->GetWorldPosition()) < 0.23f) {
            Trigger();
        }
    }
}
