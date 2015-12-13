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

#ifndef MASTERCONTROL_H
#define MASTERCONTROL_H

#include <Urho3D/Urho3D.h>

#include "luckey.h"

namespace Urho3D {
class Node;
class Scene;
}

using namespace Urho3D;

class heXoCam;
class InputMaster;
class TileMaster;
class SpawnMaster;
class Razor;
class Player;
class Apple;
class MultiX;
class ChaoBall;
class Heart;

typedef struct GameWorld
{
    SharedPtr<heXoCam> camera;
    SharedPtr<Scene> scene;
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

//heXon's resource tree
typedef struct Resources
{
    struct {
        struct {
            SharedPtr<Model> male;
            SharedPtr<Model> female;
        } pilots;
        struct {
            SharedPtr<Model> swift;
        } ships;
        struct {
            SharedPtr<Model> center;
            struct {
                SharedPtr<Model> top;
                SharedPtr<Model> bottom;
            } razor;
            struct {
                SharedPtr<Model> top;
                SharedPtr<Model> bottom;
            } spire;
        } enemies;
        struct {
            SharedPtr<Model> backgroundTile;
            SharedPtr<Model> obstacle;
        } arenaElements;
    } models;
    struct {
        SharedPtr<Material> basic;
        SharedPtr<Material> shipPrimary;
        SharedPtr<Material> shipSecondary;
    } materials;
} Resources;

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
StringHash const N_MULTIX = StringHash("MultiX");
StringHash const N_CHAOBALL = StringHash("ChaoBall");
StringHash const N_RESET = StringHash("Reset");
StringHash const N_SEEKER = StringHash("Seeker");
StringHash const N_SPIRE = StringHash("Spire");
StringHash const N_RAZOR = StringHash("Razor");
}

enum GameState {GS_INTRO, GS_LOBBY, GS_PLAY, GS_DEAD, GS_EDIT};
enum JoyStickButton {JB_SELECT, JB_LEFTSTICK, JB_RIGHTSTICK, JB_START, JB_DPAD_UP, JB_DPAD_RIGHT, JB_DPAD_DOWN, JB_DPAD_LEFT, JB_L2, JB_R2, JB_L1, JB_R1, JB_TRIANGLE, JB_CIRCLE, JB_CROSS, JB_SQUARE};
enum PickupType {PT_RESET, PT_APPLE, PT_HEART, PT_MULTIX, PT_CHAOBALL};

class MasterControl : public Application
{
    URHO3D_OBJECT(MasterControl, Application);
public:
    MasterControl(Context* context);
    GameWorld world;
    Resources resources;
    SharedPtr<PhysicsWorld> physicsWorld_;
    WeakPtr<ResourceCache> cache_;
    WeakPtr<Graphics> graphics_;
    SharedPtr<SoundSource> musicSource_;
    SharedPtr<UI> ui_;
    SharedPtr<Renderer> renderer_;
    SharedPtr<XMLFile> defaultStyle_;
    SharedPtr<TileMaster> tileMaster_;
    SharedPtr<InputMaster> inputMaster_;
    SharedPtr<SpawnMaster> spawnMaster_;

    SharedPtr<Player> player_;
    SharedPtr<Apple> apple_;
    SharedPtr<Heart> heart_;
    SharedPtr<MultiX> multiX_;
    SharedPtr<ChaoBall> chaoBall_;
    SharedPtr<Node> lobbyNode_;

    // Setup before engine initialization. Modifies the engine paramaters.
    virtual void Setup();
    // Setup after engine initialization.
    virtual void Start();
    // Cleanup after the main loop. Called by Application.
    virtual void Stop();
    void Exit();

    void CreateSineLookupTable();
    float Sine(float x) { return sine_[(int)round(sine_.Size() * LucKey::Cycle((float)(x/(2.0f*M_PI)), 0.0f, 1.0f))%sine_.Size()]; }
    float Sine(float freq, float min, float max, float shift = 0.0f);
    float Cosine(float x) { return Sine(x+(0.5f*M_PI)); }
    float Cosine(float freq, float min, float max, float shift = 0.0f){ return Sine(freq, min, max, shift+0.5f*M_PI); }

    void SetGameState(GameState newState);
    GameState GetGameState(){ return currentState_; }
    bool GetPaused() { return paused_; }
    void SetPaused(bool paused) { paused_ = paused; world.scene->SetUpdateEnabled(!paused);}
    void Pause() { SetPaused(true);}
    void Unpause() { SetPaused(false); }
    bool editMode_;
    bool PhysicsRayCast(PODVector<PhysicsRaycastResult> &hitResults, Urho3D::Ray ray, float distance, unsigned collisionMask);
    bool PhysicsSphereCast(PODVector<RigidBody*> &hitResults, Vector3 center, float radius, unsigned collisionMask);
    void StartGame();

    void Eject();
private:
    Vector<double> sine_;

    bool paused_;
    GameState currentState_;

    SharedPtr<Sound> menuMusic_;
    SharedPtr<Sound> gameMusic_;

    Light* lobbySpotLight_;

    void SetWindowTitleAndIcon();
    void CreateConsoleAndDebugHud();

    void CreateScene();
    void CreateUI();
    void SubscribeToEvents();

    void HandleSceneUpdate(StringHash eventType, VariantMap &eventData);
    void HandlePlayTrigger(StringHash otherNode, VariantMap &eventData){ SetGameState(GS_PLAY); }
    void HandleExitTrigger(StringHash otherNode, VariantMap &eventData){ Exit(); }

    void UpdateCursor(const float timeStep);
    bool CursorRayCast(const float maxDistance, PODVector<RayQueryResult> &hitResults);

    void LeaveGameState();
    void EnterGameState();
    void LoadResources();
};

#endif // MASTERCONTROL_H
