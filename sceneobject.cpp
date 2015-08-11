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

//#include "mastercontrol.h"
#include "tilemaster.h"
#include "flash.h"
#include "sceneobject.h"

SceneObject::SceneObject(Context* context, MasterControl* masterControl):
    Object(context),
    blink_{true}
{
    masterControl_ = masterControl;

    //Create the root node.
    rootNode_ = masterControl_->world.scene->CreateChild("SceneObject");

    flashSample_ = masterControl_->cache_->GetResource<Sound>("Resources/Samples/Flash.ogg");
    flashSample_->SetLooped(false);
    flashSource_ = rootNode_->CreateComponent<SoundSource>();
    flashSource_->SetGain(0.5f);
    flashSource_->SetSoundType(SOUND_EFFECT);

    SubscribeToEvent(E_UPDATE, HANDLER(SceneObject, BlinkCheck));
}

void SceneObject::Set(Vector3 position)
{
    rootNode_->SetPosition(position);
    rootNode_->SetEnabledRecursive(true);
}

void SceneObject::Disable()
{
    masterControl_->tileMaster_->RemoveFromAffectors(rootNode_);
    rootNode_->SetEnabledRecursive(false);
}

void SceneObject::BlinkCheck(StringHash eventType, VariantMap &eventData)
{
    if (!blink_ || masterControl_->GetPaused()) return;

    Vector3 position = rootNode_->GetPosition();
    float radius = 20.0f;
    if (position.Length() >= radius){
        Vector3 hexantNormal = Vector3::FORWARD;
        int sides = 6;
        for (int h = 0; h < sides; h++){
            Vector3 otherHexantNormal = Quaternion(h * (360.0f/sides), Vector3::UP)*Vector3::FORWARD;
            hexantNormal = position.Angle(otherHexantNormal) < position.Angle(hexantNormal) ?
                        otherHexantNormal : hexantNormal;
        }
        float boundsCheck = position.Length() * masterControl_->Cosine(M_DEGTORAD * position.Angle(hexantNormal));
        if (boundsCheck > radius){
            new Flash(context_, masterControl_, position); //Should be recycled
            Vector3 newPosition = rootNode_->GetPosition()-(2.0f*radius)*hexantNormal;
            rootNode_->SetPosition(newPosition);
            new Flash(context_, masterControl_, newPosition); //Should be recycled
            flashSource_->Play(flashSample_);
        }
    }
}
