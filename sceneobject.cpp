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

#include "tilemaster.h"
#include "sceneobject.h"
#include "spawnmaster.h"
#include "bullet.h"

SceneObject::SceneObject(Context* context, MasterControl* masterControl):
    Object(context),
    blink_{true}
{
    masterControl_ = masterControl;

    rootNode_ = masterControl_->world.scene->CreateChild("SceneObject");

    flashSample_ = masterControl_->cache_->GetResource<Sound>("Resources/Samples/Flash.ogg");
    flashSample_->SetLooped(false);
    for (int i = 0; i < 5; ++i){
        SharedPtr<SoundSource> sampleSource = SharedPtr<SoundSource>(rootNode_->CreateComponent<SoundSource>());
        sampleSource->SetSoundType(SOUND_EFFECT);
        sampleSources_.Push(sampleSource);
    }
}

void SceneObject::Set(const Vector3 position)
{
    rootNode_->SetEnabledRecursive(true);
    rootNode_->SetPosition(position);
    if (blink_)
        SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(SceneObject, BlinkCheck));
}

void SceneObject::Disable()
{
    masterControl_->tileMaster_->RemoveFromAffectors(rootNode_);
    rootNode_->SetEnabledRecursive(false);
    if (blink_)
        UnsubscribeFromEvent(E_POSTRENDERUPDATE);
    UnsubscribeFromEvent(E_NODECOLLISIONSTART);
}

void SceneObject::PlaySample(Sound* sample, const float gain)
{
    for (unsigned i = 0; i < sampleSources_.Size(); ++i){
        if (!sampleSources_[i]->IsPlaying()){
            sampleSources_[i]->SetGain(gain);
            sampleSources_[i]->Play(sample);
            return;
        }
    }
}
void SceneObject::StopAllSound()
{
    for (unsigned i = 0; i < sampleSources_.Size(); ++i){
        sampleSources_[i]->Stop();
    }
}
bool SceneObject::IsPlayingSound()
{
    for (unsigned i = 0; i < sampleSources_.Size(); ++i){
        if (sampleSources_[i]->IsPlaying()) return true;
    }
    return false;
}

void SceneObject::BlinkCheck(StringHash eventType, VariantMap &eventData)
{
    if (masterControl_->GetPaused()) return;

    Vector3 flatPosition = LucKey::Scale(rootNode_->GetPosition(), Vector3::ONE-Vector3::UP);
    float radius = 20.0f;
    if (flatPosition.Length() > radius){
        Vector3 hexantNormal = Vector3::FORWARD;
        int sides = 6;
        for (int h = 0; h < sides; h++){
            Vector3 otherHexantNormal = Quaternion(h * (360.0f/sides), Vector3::UP)*Vector3::FORWARD;
            hexantNormal = flatPosition.Angle(otherHexantNormal) < flatPosition.Angle(hexantNormal)
                    ? otherHexantNormal : hexantNormal;
        }
        float boundsCheck = flatPosition.Length() * masterControl_->Cosine(M_DEGTORAD * flatPosition.Angle(hexantNormal));
        if (boundsCheck > radius){
            if (rootNode_->GetNameHash() == N_BULLET){
                masterControl_->spawnMaster_->SpawnHitFX(GetPosition(), 0, false);
                Disable();
            } else if (blink_){
                masterControl_->spawnMaster_->SpawnFlash(rootNode_->GetPosition());
                Vector3 newPosition = rootNode_->GetPosition()-(1.995f*radius)*hexantNormal;
                rootNode_->SetPosition(newPosition);
                masterControl_->spawnMaster_->SpawnFlash(newPosition);
                PlaySample(flashSample_, 0.16f);
            }
        }
    }
}

void SceneObject::Emerge(const float timeStep)
{
    if (!IsEmerged()) {
        rootNode_->Translate(2.3f*Vector3::UP * timeStep * (0.23f - rootNode_->GetPosition().y_), TS_WORLD);
    }
}
