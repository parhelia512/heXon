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

    for (int r = 0; r < 23; r++) SpawnRazor(SpawnPoint());
    for (int s = 0; s < 7; s++) SpawnSpire(SpawnPoint());
    for (int s = 0; s < 20; s++) SpawnSeeker(SpawnPoint());
    for (int h = 0; h < 15; h++) SpawnHitFX(SpawnPoint(), false);
    for (int e = 0; e < 5; e++) SpawnExplosion(SpawnPoint(), Color::WHITE, 1.0f);
    for (int f = 0; f < 10; f++) SpawnFlash(SpawnPoint());
    Clear();
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
    for (unsigned s = 0; s < seekers_.Size(); s++){
        if (seekers_[s]->IsEnabled())
            seekers_[s]->Disable();
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
        if (razors_[r]->rootNode_!=nullptr && razors_[r]->rootNode_->IsEnabled()) ++razorCount;
    }
    return razorCount;
}
int SpawnMaster::CountActiveSpires() //Crash
{
    int spireCount = 0;
    for (unsigned s = 0; s < spires_.Values().Size(); s++){
        if (spires_[s]->rootNode_->IsEnabled()) ++spireCount;
    }
    return spireCount;
}

void SpawnMaster::SpawnRazor(Vector3 position)
{
    sinceRazorSpawn_ = 0.0f;
    if (!RespawnRazor(position)){
        Razor* newRazor = new Razor(context_, masterControl_, position);
        razors_[newRazor->rootNode_->GetID()] = SharedPtr<Razor>(newRazor);
    }
    razorInterval_ = 7.0f * pow(0.95f, ((masterControl_->world.scene->GetElapsedTime() - masterControl_->world.lastReset) + 10.0f) / 10.0f);
}
bool SpawnMaster::RespawnRazor(Vector3 position)
{
    Vector<SharedPtr<Razor> > razors = razors_.Values();
    for (unsigned r = 0; r < razors.Size(); r++){
        if (!razors[r]->rootNode_->IsEnabled()){
            SharedPtr<Razor> razor = razors[r];
            razor->Set(position);
            return true;
        }
    }
    return false;
}

void SpawnMaster::SpawnSpire(Vector3 position)
{
    sinceSpireSpawn_ = 0.0f;
    if (!RespawnSpire(position)){
        Spire* newSpire = new Spire(context_, masterControl_, position);
        spires_[newSpire->rootNode_->GetID()] = SharedPtr<Spire>(newSpire);
    }
    spireInterval_ = 23.0f * pow(0.95f, ((masterControl_->world.scene->GetElapsedTime() - masterControl_->world.lastReset) + 42.0f) / 42.0f);
}
bool SpawnMaster::RespawnSpire(Vector3 position)
{
    Vector<SharedPtr<Spire> > spires = spires_.Values();
    for (unsigned s = 0; s < spires.Size(); s++){
        if (!spires[s]->rootNode_->IsEnabled()){
            SharedPtr<Spire> spire = spires[s];
            spire->Set(position);
            return true;
        }
    }
    return false;
}

void SpawnMaster::SpawnSeeker(Vector3 position)
{
    if (!RespawnSeeker(position)){
        Seeker* newSeeker = new Seeker(context_, masterControl_, position);
        seekers_.Push(SharedPtr<Seeker>(newSeeker));
    }
}
bool SpawnMaster::RespawnSeeker(Vector3 position)
{
    for (unsigned s = 0; s < seekers_.Size(); s++){
        if (!seekers_[s]->rootNode_->IsEnabled()){
            SharedPtr<Seeker> seeker = seekers_[s];
            seeker->Set(position);
            return true;
        }
    }
    return false;
}

void SpawnMaster::SpawnHitFX(Vector3 position, bool sound)
{
    if (!RespawnHitFX(position, sound)){
        HitFX* newHitFX = new HitFX(context_, masterControl_, position, sound);
        hitFXs_.Push(SharedPtr<HitFX>(newHitFX));
    }
}
bool SpawnMaster::RespawnHitFX(Vector3 position, bool sound)
{
    for (unsigned h = 0; h < hitFXs_.Size(); h++){
        if (!hitFXs_[h]->rootNode_->IsEnabled()){
            SharedPtr<HitFX> hitFX = hitFXs_[h];
            hitFX->Set(position, sound);
            return true;
        }
    }
    return false;
}

void SpawnMaster::SpawnFlash(Vector3 position)
{
    if (!RespawnFlash(position)){
        Flash* newFlash = new Flash(context_, masterControl_, position);
        flashes_.Push(SharedPtr<Flash>(newFlash));
    }
}
bool SpawnMaster::RespawnFlash(Vector3 position)
{
    for (unsigned f = 0; f < flashes_.Size(); f++){
        if (!flashes_[f]->rootNode_->IsEnabled()){
            SharedPtr<Flash> flash = flashes_[f];
            flash->Set(position);
            return true;
        }
    }
    return false;
}

bool SpawnMaster::SpawnExplosion(Vector3 position, Color color, float size)
{
    if (!RespawnExplosion(position, color, size)){
        Explosion* explosion = new Explosion(context_, masterControl_, position, color, size);
        explosions_.Push(SharedPtr<Explosion>(explosion));
    }
    return false;
}
bool SpawnMaster::RespawnExplosion(Vector3 position, Color color, float size)
{
    for (unsigned e = 0; e < explosions_.Size(); e++){
        if (!explosions_[e]->rootNode_->IsEnabled()){
            WeakPtr<Explosion> explosion = explosions_[e];
            explosion->Set(position, color, size);
            return true;
        }
    }
    return false;
}
