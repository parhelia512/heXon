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

#include "splatterpillar.h"

#include "player.h"

SplatterPillar::SplatterPillar(bool right):
    Object(MC->GetContext()),
    player_{},
    right_{right},
    spun_{false},
    reset_{true},
    delayed_{0.0f},
    delay_{0.5f},
    sequenceLength_{5.0f},
    lastTriggered_{-5.0f},
    rotationSpeed_{}
{
    rootNode_ = MC->lobbyNode_->CreateChild("SplatterPillar");
    rootNode_->SetPosition(Vector3(right_? 2.26494f : -2.26494f, 0.0f, -3.91992f));
    rootNode_->Rotate(Quaternion(Random(6)*60.0f, Vector3::UP));
    pillarNode_ = rootNode_->CreateChild("Pillar");
    bloodNode_ = rootNode_->CreateChild("Blood");
    pillar_ = pillarNode_->CreateComponent<AnimatedModel>();
    pillar_->SetModel(MC->GetModel("SplatterPillar"));
    pillar_->SetMorphWeight(0, 0.0f);
    pillar_->SetCastShadows(true);

    pillar_->SetMaterial(0, MC->GetMaterial("Basic"));
    if (!right_)
            pillar_->SetMaterial(1, MC->GetMaterial("GreenGlow"));
    else
        pillar_->SetMaterial(1, MC->GetMaterial("PurpleGlow"));

    pillar_->SetMaterial(2, MC->GetMaterial("Metal"));
    pillar_->SetMaterial(3, MC->GetMaterial("Drain"));

    blood_ = bloodNode_->CreateComponent<AnimatedModel>();
    blood_->SetEnabled(false);
    blood_->SetCastShadows(true);
    blood_->SetModel(MC->GetModel("Blood"));
    blood_->SetMaterial(0, MC->GetMaterial("Blood")->Clone());

    particleNode_ = rootNode_->CreateChild("BloodParticles");
    particleNode_->Translate(Vector3::UP*2.3f);
    splatEmitter_ = particleNode_->CreateComponent<ParticleEmitter>();
    splatEmitter_->SetEffect(CACHE->GetResource<ParticleEffect>("Particles/BloodSplat.xml"));
    splatEmitter_->SetEmitting(false);
    dripEmitter_ = particleNode_->CreateComponent<ParticleEmitter>();
    dripEmitter_->SetEffect(CACHE->GetTempResource<ParticleEffect>("Particles/BloodDrip.xml"));
    dripEmitter_->SetEmitting(false);

    soundSource_ = rootNode_->CreateComponent<SoundSource>();
    soundSource_->SetGain(1.0f);

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(SplatterPillar, HandleSceneUpdate));
}

void SplatterPillar::Trigger()
{
    rotationSpeed_ = Random(-1.0f, 1.0f);
    lastTriggered_ = MC->world.scene->GetElapsedTime();
    player_->KillPilot();
    bloodNode_->Rotate(Quaternion(Random(360.0f), Vector3::UP));
    blood_->SetEnabled(true);
    soundSource_->Play(CACHE->GetResource<Sound>("Samples/Splatter" + String(Random(1,6)) + ".ogg"));
}

void SplatterPillar::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    if (player_ == nullptr) player_ = MC->GetPlayer(right_+1);

    if (MC->GetGameState() != GS_LOBBY) return;

    float timeStep_{eventData[SceneUpdate::P_TIMESTEP].GetFloat()};
    float elapsedTime{MC->world.scene->GetElapsedTime()};
    float intoSequence{(elapsedTime - lastTriggered_)/sequenceLength_};
    unsigned numMorphs{blood_->GetNumMorphs()};

    //Animate
    if (intoSequence < 1.0f) {
        //Animate blood
        if (!bloodNode_->IsEnabled()){
            bloodNode_->SetEnabled(true);
            splatEmitter_->SetEmitting(true);
            particleNode_->Rotate(Quaternion{Random(360.0f), Vector3::UP});
        }
        if (intoSequence > 0.023f) {
            splatEmitter_->SetEmitting(false);
            dripEmitter_->SetEmitting(true);
        }
        for (unsigned m{0}; m < numMorphs; ++m){
            float intoMorph{Clamp(intoSequence * numMorphs - m, 0.0f, 2.0f)};
            if (intoMorph > 1.0f) intoMorph = Max(2.0f - intoMorph, 0.0f);
            else if (m == 0) Min(intoMorph *= 5.0f, 1.0f);
            blood_->SetMorphWeight(m, intoMorph);
        }
        blood_->GetMaterial()->SetShaderParameter("MatDiffColor", Color(0.23f, 0.32f, 0.32f, Clamp(1.0f - (intoSequence - 0.75f) * 5.0f, 0.0f, 1.0f)));
        blood_->GetMaterial()->SetShaderParameter("Dissolve", 0.75f*intoSequence + 0.23f);
        ParticleEffect* dripEffect{dripEmitter_->GetEffect()};
        dripEffect->SetEmitterSize(Vector3{1.0f - intoSequence, 0.0f, 1.0f - intoSequence});
        dripEffect->SetMinEmissionRate(Max(123.0f - 200.0f * intoSequence, 0.0f));
        dripEffect->SetMaxEmissionRate(Max(320.0f - 340.0f * intoSequence, 0.0f));
        //Animate pillar
        if      (intoSequence < 0.125f) pillar_->SetMorphWeight(0, 123.0f * intoSequence);
        else if (intoSequence < 0.1666f) {
            pillar_->SetMorphWeight(0, 1.0f);
            if (!spun_){
                pillarNode_->Rotate(Quaternion{Random(6)*60.0f, Vector3::UP});
                spun_ = true;
            }
        }
        else if (intoSequence > (1.0f/6.0f)) {
            spun_ = false;
            float weight{Max(2.0f * (1.0f - 3.0f*intoSequence), 0.0f)};
            pillar_->SetMorphWeight(0, weight*weight*weight);
        }
    } else {
    //When idle
        //Reset
        if (bloodNode_->IsEnabled()) {
            bloodNode_->SetEnabled(false);
            dripEmitter_->SetEmitting(false);
        }
        if (pillar_->GetMorphWeight(0) != 0.0f) pillar_->SetMorphWeight(0, 0.0f);
        //Trigger
        if (player_ && LucKey::Distance(player_->GetPosition(), rootNode_->GetWorldPosition()) < 0.23f) {
            delayed_ += timeStep_;
            if (delayed_ > delay_){
                Trigger();
                delayed_ = 0.0f;
            }
        } else delayed_ = 0.0f;
    }
}

bool SplatterPillar::IsIdle() const
{
    return !bloodNode_->IsEnabled();
}
