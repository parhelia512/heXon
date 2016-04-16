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

#include "spawnmaster.h"

#include "tilemaster.h"
#include "tile.h"
#include "player.h"

SpawnMaster::SpawnMaster(MasterControl *masterControl):
    Object(masterControl->GetContext()),
    spawning_{false},
    razorInterval_{2.0f},
    sinceRazorSpawn_{0.0f},
    spireInterval_{23.0f},
    sinceSpireSpawn_{0.0f},
    bubbleInterval_{0.23f},
    sinceBubbleSpawn_{bubbleInterval_}
{
    masterControl_ = masterControl;
    Audio* audio = GetSubsystem<Audio>();
    audio->SetMasterGain(SOUND_EFFECT, 0.0f);
    for (int r = 0; r < 23; ++r) {
        Razor* newRazor = new Razor(masterControl_);
        razors_[newRazor->rootNode_->GetID()] = SharedPtr<Razor>(newRazor);
    }
    for (int s = 0; s < 7; ++s) {
        Spire* newSpire = new Spire(masterControl_);
        spires_[newSpire->rootNode_->GetID()] = SharedPtr<Spire>(newSpire);
    }
    for (int m = 0; m < 8; ++m) {
        ChaoMine* newChaoMine = new ChaoMine(masterControl_);
        chaoMines_[newChaoMine->rootNode_->GetID()] = SharedPtr<ChaoMine>(newChaoMine);
    }

    for (int s = 0; s < 13; ++s) {
        Seeker* newSeeker = new Seeker(masterControl_);
        seekers_[newSeeker->rootNode_->GetID()] = SharedPtr<Seeker>(newSeeker);
    }
    for (int h = 0; h < 16; ++h) {
        HitFX* newHitFX = new HitFX(masterControl_);
        hitFXs_.Push(SharedPtr<HitFX>(newHitFX));
    }
    for (int e = 0; e < 9; ++e) {
        Explosion* newExplosion = new Explosion(masterControl_);
        explosions_.Push(SharedPtr<Explosion>(newExplosion));
    }
    for (int f = 0; f < 13; ++f) {
        Flash* newFlash= new Flash(masterControl_);
        flashes_.Push(SharedPtr<Flash>(newFlash));
    }
    for (int b = 0; b < 42; ++b) {
        Bubble* newBubble= new Bubble(masterControl_);
        bubbles_.Push(SharedPtr<Bubble>(newBubble));
    }
    for (int l = 0; l < 2048; ++l) {
        Line* newLine= new Line(masterControl_);
        lines_.Push(SharedPtr<Line>(newLine));
    }
    for (int z = 0; z < 8; ++z) {
        ChaoZap* newChaoZap = new ChaoZap(masterControl_);
        chaoZaps_.Push(SharedPtr<ChaoZap>(newChaoZap));
    }
    Clear();
    audio->SetMasterGain(SOUND_EFFECT, 1.0f);
}

void SpawnMaster::Activate()
{
    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(SpawnMaster, HandleSceneUpdate));
}
void SpawnMaster::Deactivate()
{
    UnsubscribeFromAllEvents();
}
void SpawnMaster::Clear()
{
    Vector<SharedPtr<Razor> > razors = razors_.Values();
    for (SharedPtr<Razor> r : razors){
        if (r->IsEnabled()) r->Disable();
    }
    Vector<SharedPtr<Spire> > spires = spires_.Values();
    for (SharedPtr<Spire> s : spires){
        if (s->IsEnabled()) s->Disable();
    }
    Vector<SharedPtr<ChaoMine> > chaoMines = chaoMines_.Values();
    for (SharedPtr<ChaoMine> c : chaoMines){
        if (c->IsEnabled()) c->Disable();
    }
    Vector<SharedPtr<Seeker> > seekers = seekers_.Values();
    for (SharedPtr<Seeker> s : seekers){
        if (s->IsEnabled()) s->Disable();
    }
    for (SharedPtr<HitFX> h : hitFXs_){
        if (h->IsEnabled()) h->Disable();
    }
    for (SharedPtr<Explosion> e : explosions_){
        if (e->IsEnabled()) e->Disable();
    }
    for (SharedPtr<Flash> f : flashes_){
        if (f->IsEnabled()) f->Disable();
    }
    for (SharedPtr<Bubble> b : bubbles_){
        if (b->IsEnabled()) b->Disable();
    }
    for (SharedPtr<Line> l : lines_){
        if (l->IsEnabled()) l->Disable();
    }
    for (SharedPtr<ChaoZap> c : chaoZaps_){
        if (c->IsEnabled()) c->Disable();
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
    Tile* randomTile = masterControl_->tileMaster_->GetRandomTile();
    if (randomTile) {
        Vector3 tilePosition = randomTile->rootNode_->GetPosition();
        return Vector3(tilePosition.x_, -23.0f, tilePosition.z_);
    }
    else return Vector3(Random(-5.0f, 5.0f), -42.0f, Random(-5.0f, 5.0f));
}

void SpawnMaster::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    const float timeStep = eventData[SceneUpdate::P_TIMESTEP].GetFloat();

    sinceRazorSpawn_ += timeStep;
    sinceSpireSpawn_ += timeStep;

    if (masterControl_->GetPlayer(1)->IsEnabled() || masterControl_->GetPlayer(2)->IsEnabled()){
        if (sinceRazorSpawn_ > razorInterval_ && CountActiveRazors() < 23)
            SpawnRazor(SpawnPoint());
        if (sinceSpireSpawn_ > spireInterval_ && CountActiveSpires() < 7)
            SpawnSpire(SpawnPoint());
    }

    sinceBubbleSpawn_ += timeStep;

    if (sinceBubbleSpawn_ > bubbleInterval_){
        SpawnBubble(BubbleSpawnPoint());
    }
}

int SpawnMaster::CountActiveRazors()
{
    int razorCount = 0;
    Vector<SharedPtr<Razor> > razors = razors_.Values();
    for (SharedPtr<Razor> r : razors){
        if (r->IsEnabled()) ++razorCount;
    }
    return razorCount;
}
int SpawnMaster::CountActiveSpires()
{
    int spireCount = 0;
    Vector<SharedPtr<Spire> > spires = spires_.Values();
    for (SharedPtr<Spire> s : spires){
        if (s->IsEnabled()) ++spireCount;
    }
    return spireCount;
}
int SpawnMaster::CountActiveLines()
{
    int lineCount = 0;
    for (SharedPtr<Line> l : lines_){
        if (l->IsEnabled()) ++lineCount;
    }
    return lineCount;
}

void SpawnMaster::SpawnRazor(const Vector3 &position)
{
    sinceRazorSpawn_ = 0.0f;
    if (!RespawnRazor(position)){
        Razor* newRazor = new Razor(masterControl_);
        newRazor->Set(position);
        razors_[newRazor->rootNode_->GetID()] = SharedPtr<Razor>(newRazor);
    }
    razorInterval_ = 7.0f * pow(0.95f, ((masterControl_->SinceLastReset()) + 10.0f) / 10.0f);
}
bool SpawnMaster::RespawnRazor(const Vector3 &position)
{
    Vector<SharedPtr<Razor> > razors = razors_.Values();
    for (SharedPtr<Razor> r : razors){
        if (!r->IsEnabled()){
            r->Set(position);
            return true;
        }
    }
    return false;
}

void SpawnMaster::SpawnSpire(const Vector3 &position)
{
    sinceSpireSpawn_ = 0.0f;
    if (!RespawnSpire(position)){
        Spire* newSpire = new Spire(masterControl_);
        newSpire->Set(position);
        spires_[newSpire->rootNode_->GetID()] = SharedPtr<Spire>(newSpire);
    }
    spireInterval_ = 23.0f * pow(0.95f, ((masterControl_->world.scene->GetElapsedTime() - masterControl_->world.lastReset) + 42.0f) / 42.0f);
}
bool SpawnMaster::RespawnSpire(const Vector3 &position)
{
    Vector<SharedPtr<Spire> > spires = spires_.Values();
    for (SharedPtr<Spire> s : spires){
        if (!s->IsEnabled()){
            s->Set(position);
            return true;
        }
    }
    return false;
}

void SpawnMaster::SpawnSeeker(const Vector3& position)
{
    if (!RespawnSeeker(position)){
        Seeker* newSeeker = new Seeker(masterControl_);
        newSeeker->Set(position);
        seekers_[newSeeker->rootNode_->GetID()] = SharedPtr<Seeker>(newSeeker);
    }
}
bool SpawnMaster::RespawnSeeker(const Vector3& position)
{
    Vector<SharedPtr<Seeker> > seekers = seekers_.Values();
    for (SharedPtr<Seeker> s : seekers){
        if (!s->IsEnabled()){
            s->Set(position);
            return true;
        }
    }
    return false;
}

void SpawnMaster::SpawnChaoMine(const Vector3& position, int playerID)
{
    if (!RespawnChaoMine(position, playerID)){
        ChaoMine* newChaoMine = new ChaoMine(masterControl_);
        newChaoMine->Set(position, playerID);
        chaoMines_[newChaoMine->rootNode_->GetID()] = SharedPtr<ChaoMine>(newChaoMine);
    }
}
bool SpawnMaster::RespawnChaoMine(const Vector3& position, int playerID)
{
    Vector<SharedPtr<ChaoMine> > chaoMines = chaoMines_.Values();
    for (SharedPtr<ChaoMine> c : chaoMines){
        if (!c->IsEnabled()){
            c->Set(position, playerID);
            return true;
        }
    }
    return false;
}
void SpawnMaster::SpawnChaoZap(const Vector3& position, int playerID)
{
    if (!RespawnChaoZap(position, playerID)){
        ChaoZap* newZap = new ChaoZap(masterControl_);
        newZap->Set(position, playerID);
        chaoZaps_.Push(SharedPtr<ChaoZap>(newZap));
    }
}
bool SpawnMaster::RespawnChaoZap(const Vector3& position, int playerID)
{
    for (SharedPtr<ChaoZap> c : chaoZaps_){
        if (!c->IsEnabled()){
            c->Set(position, playerID);
            return true;
        }
    }
    return false;
}

void SpawnMaster::SpawnHitFX(const Vector3 &position, int playerID, bool sound)
{
    if (!RespawnHitFX(position, playerID, sound)){
        HitFX* newHitFX = new HitFX(masterControl_);
        newHitFX->Set(position, playerID, sound);
        hitFXs_.Push(SharedPtr<HitFX>(newHitFX));
    }
}
bool SpawnMaster::RespawnHitFX(const Vector3& position, int playerID, bool sound)
{
    for (SharedPtr<HitFX> h : hitFXs_){
        if (!h->IsEnabled()){
            h->Set(position, playerID, sound);
            return true;
        }
    }
    return false;
}

void SpawnMaster::SpawnFlash(const Vector3& position)
{
    if (!RespawnFlash(position)){
        Flash* newFlash = new Flash(masterControl_);
        newFlash->Set(position);
        flashes_.Push(SharedPtr<Flash>(newFlash));
    }
}
bool SpawnMaster::RespawnFlash(const Vector3& position)
{
    for (SharedPtr<Flash> f : flashes_){
        if (!f->IsEnabled()){
            f->Set(position);
            return true;
        }
    }
    return false;
}

bool SpawnMaster::SpawnExplosion(const Vector3& position, const Color& color, float size, int playerID)
{
    if (!RespawnExplosion(position, color, size, playerID)){
        Explosion* explosion = new Explosion(masterControl_);
        explosion->Set(position, color, size, playerID);
        explosions_.Push(SharedPtr<Explosion>(explosion));
    }
    return false;
}
bool SpawnMaster::RespawnExplosion(const Vector3& position, const Color& color, float size, int playerID)
{
    for (WeakPtr<Explosion> e : explosions_){
        if (!e->IsEnabled()){
            e->Set(position, color, size, playerID);
            return true;
        }
    }
    return false;
}

void SpawnMaster::SpawnBubble(const Vector3& position)
{
    sinceBubbleSpawn_ = 0.0f;
    bubbleInterval_ = Random(23.0f, 42.0f) / (masterControl_->SinceLastReset() + 88);
    if (!RespawnBubble(position)){
        Bubble* newBubble = new Bubble(masterControl_);
        newBubble->Set(position);
        bubbles_.Push(SharedPtr<Bubble>(newBubble));
    }
}
bool SpawnMaster::RespawnBubble(const Vector3& position)
{
    for (SharedPtr<Bubble> b : bubbles_){
        if (!b->IsEnabled()){
            b->Set(position);
            return true;
        }
    }
    return false;
}
Vector3 SpawnMaster::BubbleSpawnPoint()
{
    return Quaternion(( Random(5) - 2 ) * 60.0f, Vector3::UP) *
            (Vector3::FORWARD * 21.0f + Vector3::RIGHT * Random(-10.0f, 10.0f))
            + Vector3::DOWN * 23.0f;
}

void SpawnMaster::SpawnLine(int playerID_)
{
    if (!RespawnLine(playerID_)){
        Line* newLine = new Line(masterControl_);
        newLine->Set(LineSpawnPoint(playerID_), playerID_);
        lines_.Push(SharedPtr<Line>(newLine));
    }
}
bool SpawnMaster::RespawnLine(int playerID)
{
    for (SharedPtr<Line> l : lines_){
        if (!l->IsEnabled()){
            l->Set(LineSpawnPoint(playerID), playerID);
            return true;
        }
    }
    return false;
}
Vector3 SpawnMaster::LineSpawnPoint(int playerID)
{
    return Quaternion(( Random(2) + (playerID == 2 ? 1 : -2) ) * 60.0f, Vector3::UP) *
            (Vector3::FORWARD * Random(23.0f, 42.0f) + Vector3::RIGHT * Random(-13.0f, 13.0f))
            + Vector3::DOWN * (23.0f + Random(46.0f));
}
