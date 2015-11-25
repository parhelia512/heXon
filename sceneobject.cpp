/* heXon
// Copyright (C) 2015 LucKey Productions (luckeyproductions.nl)
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

SceneObject::SceneObject(Context* context, MasterControl* masterControl):
    Object(context),
    blink_{true}
{
    masterControl_ = masterControl;

    rootNode_ = masterControl_->world.scene->CreateChild("SceneObject");

    flashSample_ = masterControl_->cache_->GetResource<Sound>("Resources/Samples/Flash.ogg");
    flashSample_->SetLooped(false);
    for (int i = 0; i < 5; i++){
        sampleSources_.Push(SharedPtr<SoundSource>(rootNode_->CreateComponent<SoundSource>()));
        sampleSources_[i]->SetGain(0.3f);
        sampleSources_[i]->SetSoundType(SOUND_EFFECT);
    }

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SceneObject, BlinkCheck));
}

void SceneObject::Set(Vector3 position)
{
    rootNode_->SetPosition(position);
    rootNode_->SetEnabledRecursive(true);
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SceneObject, BlinkCheck));
}

void SceneObject::Disable()
{
    masterControl_->tileMaster_->RemoveFromAffectors(rootNode_);
    rootNode_->SetEnabledRecursive(false);
    UnsubscribeFromEvent(E_UPDATE);
    UnsubscribeFromEvent(E_NODECOLLISIONSTART);
}

void SceneObject::PlaySample(Sound* sample)
{
    for (unsigned i = 0; i < sampleSources_.Size(); ++i){
        if (!sampleSources_[i]->IsPlaying()){
            sampleSources_[i]->Play(sample);
            return;
        }
    }
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
            if (blink_){
                masterControl_->spawnMaster_->SpawnFlash(rootNode_->GetPosition());
                Vector3 newPosition = rootNode_->GetPosition()-(1.995f*radius)*hexantNormal;
                rootNode_->SetPosition(newPosition);
                masterControl_->spawnMaster_->SpawnFlash(newPosition);
                PlaySample(flashSample_);
            } else if (rootNode_->GetNameHash() == N_BULLET){
                masterControl_->spawnMaster_->SpawnHitFX(GetPosition(), false);
                Disable();
            }
        }
    }
}
