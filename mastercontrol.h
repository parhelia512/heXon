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

#ifndef MASTERCONTROL_H
#define MASTERCONTROL_H

#include <Urho3D/Urho3D.h>

#include "luckey.h"
#include "player.h"

namespace Urho3D {
class Node;
class Scene;
}

class heXoCam;
class InputMaster;
class Arena;
class SpawnMaster;
class Razor;
//class Player;
class Door;
class SplatterPillar;
class Apple;
class ChaoBall;
class Heart;
class Lobby;

typedef struct GameWorld
{
    SharedPtr<heXoCam> camera;
    float lastReset;
    SharedPtr<Octree> octree;
    SharedPtr<Node> backdropNode;
    SharedPtr<Node> voidNode;
    struct {
        SharedPtr<Node> sceneCursor;
        SharedPtr<Cursor> uiCursor;
        PODVector<RayQueryResult> hitResults;
    } cursor;
} GameWorld;

typedef struct HitInfo
{
    Vector3 position_;
    Vector3 hitNormal_;
    Node* hitNode_;
    Drawable* drawable_;
} HitInfo;

namespace {
StringHash const N_VOID = StringHash("Void");
StringHash const N_ARENAEDGE = StringHash("ArenaEdge");
StringHash const N_CURSOR = StringHash("Cursor");
StringHash const N_TILE = StringHash("Tile");
StringHash const N_PLAYER = StringHash("Player");
StringHash const N_BULLET = StringHash("Bullet");
StringHash const N_APPLE = StringHash("Apple");
StringHash const N_HEART = StringHash("Heart");
StringHash const N_CHAOBALL = StringHash("ChaoBall");
StringHash const N_CHAOMINE = StringHash("ChaoMine");
StringHash const N_RESET = StringHash("Reset");
StringHash const N_SEEKER = StringHash("Seeker");
StringHash const N_SPIRE = StringHash("Spire");
StringHash const N_RAZOR = StringHash("Razor");
}

enum GameState {GS_INTRO, GS_LOBBY, GS_PLAY, GS_DEAD, GS_EDIT};
enum PickupType {PT_RESET, PT_APPLE, PT_HEART, PT_CHAOBALL};

#define MC MasterControl::GetInstance()

class MasterControl : public Application
{
    URHO3D_OBJECT(MasterControl, Application);
public:
    MasterControl(Context* context);
    static MasterControl* GetInstance();

    GameWorld world;
    Scene* scene_;
    PhysicsWorld* physicsWorld_;
    SoundSource* musicSource_;
    SharedPtr<XMLFile> defaultStyle_;
    Lobby* lobby_;
    Arena* arena_;

    Vector< SharedPtr<Player> > players_;

    Apple* apple_;
    Heart* heart_;
    ChaoBall* chaoBall_;

    // Setup before engine initialization. Modifies the engine paramaters.
    virtual void Setup();
    // Setup after engine initialization.
    virtual void Start();
    // Cleanup after the main loop. Called by Application.
    virtual void Stop();
    void Exit();

    Material* GetMaterial(String name) const { return CACHE->GetResource<Material>("Materials/"+name+".xml"); }
    Model* GetModel(String name) const { return CACHE->GetResource<Model>("Models/"+name+".mdl"); }
    Texture* GetTexture(String name) const { return CACHE->GetResource<Texture>("Textures/"+name+".png"); }
    Sound* GetMusic(String name) const;
    Sound* GetSample(String name) const;

    Player* GetPlayer(int playerID) const;
    Player*GetNearestPlayer(Vector3 pos);
    Vector< SharedPtr<Player> > GetPlayers() { return players_; }

    float SinceLastReset() const { return scene_->GetElapsedTime() - world.lastReset; }
    void SetGameState(GameState newState);
    GameState GetGameState(){ return currentState_; }
    GameState GetPreviousGameState(){ return previousState_; }
    float GetAspectRatio() const noexcept { return aspectRatio_; }
    bool IsPaused() { return paused_; }
    void SetPaused(bool paused) { paused_ = paused; scene_->SetUpdateEnabled(!paused);}
    void Pause() { SetPaused(true);}
    void Unpause() { SetPaused(false); }
    float GetSinceStateChange() const noexcept { return sinceStateChange_; }

    bool PhysicsRayCast(PODVector<PhysicsRaycastResult> &hitResults, Urho3D::Ray ray, const float distance, const unsigned collisionMask = M_MAX_UNSIGNED);
    bool PhysicsSphereCast(PODVector<RigidBody*> &hitResults, Vector3 center, const float radius, const unsigned collisionMask = M_MAX_UNSIGNED);
    void StartGame();

    void Eject();
    bool NoHumans();
    void LoadHighest();

    float Sine(const float freq, const float min, const float max, const float shift = 0.0f);
    float Cosine(const float freq, const float min, const float max, const float shift = 0.0f);
private:
    static MasterControl* instance_;

    Vector<double> sine_;

    float aspectRatio_;
    bool paused_;
    GameState previousState_;
    GameState currentState_;
    float sinceStateChange_;

    SharedPtr<Sound> menuMusic_;
    SharedPtr<Sound> gameMusic_;

    Light* lobbySpotLight_;

    Material* lobbyGlowGreen_;
    Material* lobbyGlowPurple_;

    void SetWindowTitleAndIcon();
    void CreateConsoleAndDebugHud();

    void CreateScene();
    void CreateUI();
    void SubscribeToEvents();

    void HandleSceneUpdate(StringHash eventType, VariantMap &eventData);

    void UpdateCursor(const float timeStep);
    bool CursorRayCast(const float maxDistance, PODVector<RayQueryResult> &hitResults);

    void LeaveGameState();
    void EnterGameState();

    float secondsPerFrame_;
    float sinceFrameRateReport_;
    float SinePhase(float freq, float shift);
};

#endif // MASTERCONTROL_H
