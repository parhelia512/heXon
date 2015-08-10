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
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Container/HashMap.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Console.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/OctreeQuery.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/Resource.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>

#include "helper.h"

namespace Urho3D {
class Drawable;
class Node;
class Scene;
class Sprite;
}

using namespace Urho3D;

class heXoCam;
class InputMaster;
class TileMaster;
class SpawnMaster;
class Razor;
class Player;
class Apple;
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
        Vector<SharedPtr<Material> > skin;
        Vector<SharedPtr<Material> > cloth;
        Vector<SharedPtr<Material> > shoes;
        Vector<SharedPtr<Material> > hair;
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
StringHash const N_APPLE = StringHash("Apple");
StringHash const N_HEART = StringHash("Heart");
StringHash const N_RESET = StringHash("Reset");
StringHash const N_SEEKER = StringHash("Seeker");
StringHash const N_SPIRE = StringHash("Spire");
StringHash const N_RAZOR = StringHash("Razor");
}

enum GameState {GS_INTRO, GS_LOBBY, GS_PLAY, GS_DEAD, GS_EDIT};
enum JoyStickButton {JB_SELECT, JB_LEFTSTICK, JB_RIGHTSTICK, JB_START, JB_DPAD_UP, JB_DPAD_RIGHT, JB_DPAD_DOWN, JB_DPAD_LEFT, JB_L2, JB_R2, JB_L1, JB_R1, JB_TRIANGLE, JB_CIRCLE, JB_CROSS, JB_SQUARE};

class MasterControl : public Application
{
    /// Enable type information.
    OBJECT(MasterControl);
public:
    /// Constructor.
    MasterControl(Context* context);
    GameWorld world;
    Resources resources;
    SharedPtr<PhysicsWorld> physicsWorld_;
    SharedPtr<ResourceCache> cache_;
    SharedPtr<Graphics> graphics_;
    SharedPtr<UI> ui_;
    SharedPtr<Renderer> renderer_;
    SharedPtr<XMLFile> defaultStyle_;
    SharedPtr<TileMaster> tileMaster_;
    SharedPtr<InputMaster> inputMaster_;
    SharedPtr<SpawnMaster> spawnMaster_;

    SharedPtr<Player> player_;
    SharedPtr<Apple> apple_;
    SharedPtr<Heart> heart_;
    SharedPtr<Node> lobbyNode_;

    /// Setup before engine initialization. Modifies the engine paramaters.
    virtual void Setup();
    /// Setup after engine initialization.
    virtual void Start();
    /// Cleanup after the main loop. Called by Application.
    virtual void Stop();
    void Exit();

    void CreateSineLookupTable();
    float Sine(float x) { return sine_[(int)round(sine_.Size() * heXon::Cycle((float)(x/M_PI), 0.0f, 1.0f))%sine_.Size()]; }
    float Sine(float freq, float min, float max, float shift = 0.0f);

    void SetGameState(GameState newState);
    GameState GetGameState(){ return currentState_; }
    bool GetPaused() { return paused_; }
    void SetPaused(bool paused) { paused_ = paused; world.scene->SetUpdateEnabled(!paused);}
    void Pause() { SetPaused(true);}
    void Unpause() { SetPaused(false); }
    bool editMode_;
    bool PhysicsRayCast(PODVector<PhysicsRaycastResult> &hitResults, Urho3D::Ray ray, float distance, unsigned collisionMask);
    bool PhysicsSphereCast(PODVector<RigidBody *> &hitResults, Vector3 center, float radius, unsigned collisionMask);
    void StartGame();

    ///Get resources
    SharedPtr<Material> GetRandomSkin() { return resources.materials.skin[Random((int)resources.materials.skin.Size())]; }
    SharedPtr<Material> GetRandomCloth() { return resources.materials.cloth[Random((int)resources.materials.cloth.Size())]; }
    SharedPtr<Material> GetRandomShoes() { return resources.materials.shoes[Random((int)resources.materials.shoes.Size())]; }
    SharedPtr<Material> GetRandomHair() { return resources.materials.hair[Random((int)resources.materials.hair.Size())]; }
    void Eject();
private:
    Vector<double> sine_;

    bool paused_;
    GameState currentState_;

    Sound* menuMusic_;
    Sound* gameMusic_;
    SoundSource* musicSource_;
    Light* lobbySpotLight_;

    void SetWindowTitleAndIcon();
    void CreateConsoleAndDebugHud();

    void CreateScene();
    void CreateUI();
    void SubscribeToEvents();

    void HandleSceneUpdate(StringHash eventType, VariantMap& eventData);
    void HandlePlayTrigger(StringHash otherNode, VariantMap &eventData){ SetGameState(GS_PLAY);}

    void UpdateCursor(double timeStep);
    bool CursorRayCast(double maxDistance, PODVector<RayQueryResult> &hitResults);

    void LeaveGameState();
    void EnterGameState();
    void LoadResources();
};

#endif // MASTERCONTROL_H
