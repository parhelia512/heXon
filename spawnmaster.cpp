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

#include "Urho3D/Audio/Audio.h"

#include "spawnmaster.h"
#include "tilemaster.h"
#include "tile.h"
#include "player.h"

SpawnMaster::SpawnMaster(Context *context, MasterControl *masterControl):
    Object(context),
    spawning_{false},
    razorInterval_{2.0f},
    sinceRazorSpawn_{0.0f},
    spireInterval_{23.0f},
    sinceSpireSpawn_{0.0f}
{
    masterControl_ = masterControl;

    Audio* audio = GetSubsystem<Audio>();
    audio->SetMasterGain(SOUND_EFFECT, 0.0f);
    for (int r = 0; r < 23; r++) {
        Razor* newRazor = new Razor(context_, masterControl_);
        razors_[newRazor->rootNode_->GetID()] = SharedPtr<Razor>(newRazor);
    }
    for (int s = 0; s < 7; s++) {
        Spire* newSpire = new Spire(context_, masterControl_);
        spires_[newSpire->rootNode_->GetID()] = SharedPtr<Spire>(newSpire);
    }
    for (int m = 0; m < 5; m++) {
        ChaoMine* newChaoMine = new ChaoMine(context_, masterControl_);
        chaoMines_[newChaoMine->rootNode_->GetID()] = SharedPtr<ChaoMine>(newChaoMine);
    }

    for (int s = 0; s < 20; s++) {
        Seeker* newSeeker = new Seeker(context_, masterControl_);
        seekers_[newSeeker->rootNode_->GetID()] = SharedPtr<Seeker>(newSeeker);
    }
    for (int h = 0; h < 15; h++) {
        HitFX* newHitFX = new HitFX(context_, masterControl_);
        hitFXs_.Push(SharedPtr<HitFX>(newHitFX));
    }
    for (int e = 0; e < 5; e++) {
        Explosion* newExplosion = new Explosion(context_, masterControl_);
        explosions_.Push(SharedPtr<Explosion>(newExplosion));
    }
    for (int f = 0; f < 10; f++) {
        Flash* newFlash= new Flash(context_, masterControl_);
        flashes_.Push(SharedPtr<Flash>(newFlash));
    }
    for (int z = 0; z < 5; z++) {
        ChaoZap* newChaoZap= new ChaoZap(context_, masterControl_);
        chaoZaps_.Push(SharedPtr<ChaoZap>(newChaoZap));
    }
    Clear();
    audio->SetMasterGain(SOUND_EFFECT, 1.0f);
}

void SpawnMaster::Activate()
{
    SubscribeToEvent(E_SCENEUPDATE, HANDLER(SpawnMaster, HandleSceneUpdate));
}
void SpawnMaster::Deactivate()
{
    UnsubscribeFromAllEvents();
}
void SpawnMaster::Clear()
{
    Vector<SharedPtr<Razor> > razors = razors_.Values();
    for (unsigned r = 0; r < razors.Size(); r++){
        if (razors[r]->IsEnabled())
            razors[r]->Disable();
    }
    Vector<SharedPtr<Spire> > spires = spires_.Values();
    for (unsigned s = 0; s < spires.Size(); s++){
        if (spires[s]->IsEnabled())
            spires[s]->Disable();
    }
    Vector<SharedPtr<ChaoMine> > chaoMines = chaoMines_.Values();
    for (unsigned m = 0; m < chaoMines.Size(); m++){
        if (chaoMines[m]->IsEnabled())
            chaoMines[m]->Disable();
    }
    Vector<SharedPtr<Seeker> > seekers = seekers_.Values();
    for (unsigned m = 0; m < seekers.Size(); m++){
        if (seekers[m]->IsEnabled())
            seekers[m]->Disable();
    }
    for (unsigned h = 0; h < hitFXs_.Size(); h++){
        if (hitFXs_[h]->IsEnabled())
            hitFXs_[h]->Disable();
    }
    for (unsigned e = 0; e < explosions_.Size(); e++){
        if (explosions_[e]->IsEnabled())
            explosions_[e]->Disable();
    }
    for (unsigned f = 0; f < flashes_.Size(); f++){
        if (flashes_[f]->IsEnabled())
            flashes_[f]->Disable();
    }
    for (unsigned z = 0; z < chaoZaps_.Size(); z++){
        if (chaoZaps_[z]->IsEnabled())
            chaoZaps_[z]->Disable();
    }

}

void SpawnMaster::Restart()
{
    Clear();
    razorInterval_ = 2.0f;
    sinceRazorSpawn_ = 0.0f;
    spireInterval_ = 23.0f;
    sinceSpireSpawn_  = 0.0f;
    Activate();
}

Vector3 SpawnMaster::SpawnPoint()
{
    WeakPtr<Tile> randomTile = masterControl_->tileMaster_->GetRandomTile();
    if (randomTile) {
        Vector3 tilePosition = randomTile->rootNode_->GetPosition();
        return Vector3(tilePosition.x_, -23.0f, tilePosition.z_);
    }
    else return Vector3(Random(-5.0f, 5.0f), -42.0f, Random(-5.0f, 5.0f));
}

void SpawnMaster::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    using namespace SceneUpdate;

    double timeStep = eventData[P_TIMESTEP].GetFloat();

    sinceRazorSpawn_ += timeStep;
    sinceSpireSpawn_ += timeStep;

    if (masterControl_->player_->IsEnabled()){
        if (sinceRazorSpawn_ > razorInterval_)// && CountActiveRazors() < 23)
            SpawnRazor(SpawnPoint());
        if (sinceSpireSpawn_ > spireInterval_)// && CountActiveSpires() < 7)
            SpawnSpire(SpawnPoint());
    }
}

int SpawnMaster::CountActiveRazors() //Crash
{
    int razorCount = 0;
    for (unsigned r = 0; r < razors_.Values().Size(); r++){
        if (razors_[r]->rootNode_!=nullptr && razors_[r]->IsEnabled()) ++razorCount;
    }
    return razorCount;
}
int SpawnMaster::CountActiveSpires() //Crash
{
    int spireCount = 0;
    for (unsigned s = 0; s < spires_.Values().Size(); s++){
        if (spires_[s]->IsEnabled()) ++spireCount;
    }
    return spireCount;
}

void SpawnMaster::SpawnRazor(const Vector3 &position)
{
    sinceRazorSpawn_ = 0.0f;
    if (!RespawnRazor(position)){
        Razor* newRazor = new Razor(context_, masterControl_);
        newRazor->Set(position);
        razors_[newRazor->rootNode_->GetID()] = SharedPtr<Razor>(newRazor);
    }
    razorInterval_ = 7.0f * pow(0.95f, ((masterControl_->world.scene->GetElapsedTime() - masterControl_->world.lastReset) + 10.0f) / 10.0f);
}
bool SpawnMaster::RespawnRazor(const Vector3 &position)
{
    Vector<SharedPtr<Razor> > razors = razors_.Values();
    for (unsigned r = 0; r < razors.Size(); r++){
        if (!razors[r]->IsEnabled()){
            SharedPtr<Razor> razor = razors[r];
            razor->Set(position);
            return true;
        }
    }
    return false;
}

void SpawnMaster::SpawnSpire(const Vector3 &position)
{
    sinceSpireSpawn_ = 0.0f;
    if (!RespawnSpire(position)){
        Spire* newSpire = new Spire(context_, masterControl_);
        newSpire->Set(position);
        spires_[newSpire->rootNode_->GetID()] = SharedPtr<Spire>(newSpire);
    }
    spireInterval_ = 23.0f * pow(0.95f, ((masterControl_->world.scene->GetElapsedTime() - masterControl_->world.lastReset) + 42.0f) / 42.0f);
}
bool SpawnMaster::RespawnSpire(const Vector3 &position)
{
    Vector<SharedPtr<Spire> > spires = spires_.Values();
    for (unsigned s = 0; s < spires.Size(); s++){
        if (!spires[s]->IsEnabled()){
            SharedPtr<Spire> spire = spires[s];
            spire->Set(position);
            return true;
        }
    }
    return false;
}

void SpawnMaster::SpawnSeeker(const Vector3& position)
{
    if (!RespawnSeeker(position)){
        Seeker* newSeeker = new Seeker(context_, masterControl_);
        newSeeker->Set(position);
        seekers_[newSeeker->rootNode_->GetID()] = SharedPtr<Seeker>(newSeeker);
    }
}
bool SpawnMaster::RespawnSeeker(const Vector3& position)
{
    Vector<SharedPtr<Seeker> > seekers = seekers_.Values();
    for (unsigned s = 0; s < seekers.Size(); s++){
        if (!seekers[s]->IsEnabled()){
            SharedPtr<Seeker> seeker = seekers[s];
            seeker->Set(position);
            return true;
        }
    }
    return false;
}

void SpawnMaster::SpawnChaoMine(const Vector3& position)
{
    if (!RespawnChaoMine(position)){
        ChaoMine* newChaoMine = new ChaoMine(context_, masterControl_);
        newChaoMine->Set(position);
        chaoMines_[newChaoMine->rootNode_->GetID()] = SharedPtr<ChaoMine>(newChaoMine);
    }
}
bool SpawnMaster::RespawnChaoMine(const Vector3& position)
{
    Vector<SharedPtr<ChaoMine> > chaoMines = chaoMines_.Values();
    for (unsigned r = 0; r < chaoMines.Size(); r++){
        if (!chaoMines[r]->IsEnabled()){
            SharedPtr<ChaoMine> chaoMine = chaoMines[r];
            chaoMine->Set(position);
            return true;
        }
    }
    return false;
}
void SpawnMaster::SpawnChaoZap(const Vector3& position)
{
    if (!RespawnChaoZap(position)){
        ChaoZap* newZap = new ChaoZap(context_, masterControl_);
        newZap->Set(position);
        chaoZaps_.Push(SharedPtr<ChaoZap>(newZap));
    }
}
bool SpawnMaster::RespawnChaoZap(const Vector3& position)
{
    for (unsigned r = 0; r < chaoZaps_.Size(); r++){
        if (!chaoZaps_[r]->IsEnabled()){
            SharedPtr<ChaoZap> chaoZap = chaoZaps_[r];
            chaoZap->Set(position);
            return true;
        }
    }
    return false;
}

void SpawnMaster::SpawnHitFX(const Vector3 &position, bool sound)
{
    if (!RespawnHitFX(position, sound)){
        HitFX* newHitFX = new HitFX(context_, masterControl_);
        newHitFX->Set(position, sound);
        hitFXs_.Push(SharedPtr<HitFX>(newHitFX));
    }
}
bool SpawnMaster::RespawnHitFX(const Vector3& position, bool sound)
{
    for (unsigned h = 0; h < hitFXs_.Size(); h++){
        if (!hitFXs_[h]->IsEnabled()){
            SharedPtr<HitFX> hitFX = hitFXs_[h];
            hitFX->Set(position, sound);
            return true;
        }
    }
    return false;
}

void SpawnMaster::SpawnFlash(const Vector3& position)
{
    if (!RespawnFlash(position)){
        Flash* newFlash = new Flash(context_, masterControl_);
        newFlash->Set(position);
        flashes_.Push(SharedPtr<Flash>(newFlash));
    }
}
bool SpawnMaster::RespawnFlash(const Vector3& position)
{
    for (unsigned f = 0; f < flashes_.Size(); f++){
        if (!flashes_[f]->IsEnabled()){
            SharedPtr<Flash> flash = flashes_[f];
            flash->Set(position);
            return true;
        }
    }
    return false;
}

bool SpawnMaster::SpawnExplosion(const Vector3& position, const Color& color, float size)
{
    if (!RespawnExplosion(position, color, size)){
        Explosion* explosion = new Explosion(context_, masterControl_);
        explosion->Set(position, color, size);
        explosions_.Push(SharedPtr<Explosion>(explosion));
    }
    return false;
}
bool SpawnMaster::RespawnExplosion(const Vector3& position, const Color& color, float size)
{
    for (unsigned e = 0; e < explosions_.Size(); e++){
        if (!explosions_[e]->IsEnabled()){
            WeakPtr<Explosion> explosion = explosions_[e];
            explosion->Set(position, color, size);
            return true;
        }
    }
    return false;
}
