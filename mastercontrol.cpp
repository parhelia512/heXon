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

#include <fstream>

#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Core/Context.h>

#include "hexocam.h"
#include "inputmaster.h"
#include "spawnmaster.h"
#include "player.h"
#include "apple.h"
#include "heart.h"
#include "multix.h"
#include "chaoball.h"
#include "bullet.h"
#include "seeker.h"
#include "flash.h"
#include "hitfx.h"
#include "explosion.h"
#include "muzzle.h"
#include "chaomine.h"
#include "mastercontrol.h"

URHO3D_DEFINE_APPLICATION_MAIN(MasterControl);

MasterControl::MasterControl(Context *context):
    Application(context),
    currentState_{GS_INTRO},
    paused_{false},
    editMode_{false},
    sinceStateChange_{0.0f}
{
}

void MasterControl::Setup()
{
    // Modify engine startup parameters.
    //Set custom window title and icon.
    engineParameters_["WindowTitle"] = "heXon";
    engineParameters_["LogName"] = GetSubsystem<FileSystem>()->GetAppPreferencesDir("urho3d", "logs")+"heXon.log";
//    engineParameters_["VSync"] = true;
//    engineParameters_["FullScreen"] = false;
//    engineParameters_["Headless"] = false;
//    engineParameters_["WindowWidth"] = 960;
//    engineParameters_["WindowHeight"] = 900;
//    engineParameters_["RenderPath"] = "RenderPaths/DeferredHWDepth.xml";
}
void MasterControl::Start()
{
    TailGenerator::RegisterObject(context_);

    new InputMaster(context_, this);
    cache_ = GetSubsystem<ResourceCache>();
    graphics_ = GetSubsystem<Graphics>();
    renderer_ = GetSubsystem<Renderer>();
    CreateSineLookupTable();

    LoadResources();

    // Get default style
    defaultStyle_ = cache_->GetResource<XMLFile>("UI/DefaultStyle.xml");
    //Create console and debug HUD.
    CreateConsoleAndDebugHud();
    //Create the scene content
    CreateScene();
    //Create the UI content
    CreateUI();
    //Hook up to the frame update and render post-update events

//    menuMusic_ = cache_->GetResource<Sound>("Resources/Music/Eddy J - Webbed Gloves in Neon Brights.ogg");
//    menuMusic_->SetLooped(true);
    gameMusic_ = cache_->GetResource<Sound>("Resources/Music/Alien Chaos - Disorder.ogg");
    gameMusic_->SetLooped(true);
    Node* musicNode = world.scene->CreateChild("Music");
    musicSource_ = musicNode->CreateComponent<SoundSource>();
    musicSource_->SetGain(1.0f);
    musicSource_->SetSoundType(SOUND_MUSIC);
//    musicSource_->Play(menuMusic_);

    SetGameState(GS_LOBBY);

    SubscribeToEvents();
}
void MasterControl::Stop()
{
    engine_->DumpResources(true);
}

void MasterControl::SubscribeToEvents()
{
    //Subscribe scene update event.
    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(MasterControl, HandleSceneUpdate));
}

void MasterControl::CreateConsoleAndDebugHud()
{
    // Create console
    Console* console = engine_->CreateConsole();
    console->SetDefaultStyle(defaultStyle_);
    console->GetBackground()->SetOpacity(0.8f);

    // Create debug HUD.
    DebugHud* debugHud = engine_->CreateDebugHud();
    debugHud->SetDefaultStyle(defaultStyle_);
}

void MasterControl::CreateUI()
{
    ui_ = GetSubsystem<UI>();

    //Create a Cursor UI element because we want to be able to hide and show it at will. When hidden, the mouse cursor will control the camera
    world.cursor.uiCursor = new Cursor(context_);
    world.cursor.uiCursor->SetVisible(true);
    ui_->SetCursor(world.cursor.uiCursor);

    //Set starting position of the cursor at the rendering window center
    world.cursor.uiCursor->SetPosition(graphics_->GetWidth()/2, graphics_->GetHeight()/2);
}

void MasterControl::LoadResources()
{
    //Load models
    resources.models.pilots.male = cache_->GetResource<Model>("Resources/Models/Male.mdl");
    resources.models.pilots.female = cache_->GetResource<Model>("Resources/Models/Female.mdl");
    resources.models.pilots.hairStyles.Push(SharedPtr<Model>(cache_->GetResource<Model>("Resources/Models/Mohawk.mdl")));
    resources.models.pilots.hairStyles.Push(SharedPtr<Model>(cache_->GetResource<Model>("Resources/Models/Seagull.mdl")));
    resources.models.pilots.hairStyles.Push(SharedPtr<Model>(cache_->GetResource<Model>("Resources/Models/Mustain.mdl")));

    resources.models.ships.swift = cache_->GetResource<Model>("Resources/Models/KlåMk10.mdl");

    resources.models.arenaElements.backgroundTile = cache_->GetResource<Model>("Resources/Models/BackgroundTile.mdl");
    resources.models.arenaElements.obstacle = cache_->GetResource<Model>("Resources/Models/Obstacle.mdl");

    //Load materials
    resources.materials.basic = SharedPtr<Material>(cache_->GetResource<Material>("Resources/Materials/Basic.xml"));
    resources.materials.basic->SetShaderParameter("MatDiffColor", Color(0.05f, 0.05f, 0.05f, 1.0));

    resources.materials.ship1Primary = cache_->GetResource<Material>("Resources/Materials/GreenEnvmap.xml");
    resources.materials.ship1Secondary = cache_->GetResource<Material>("Resources/Materials/GreenGlowEnvmap.xml");
    resources.materials.ship2Primary = cache_->GetResource<Material>("Resources/Materials/PurpleEnvmap.xml");
    resources.materials.ship2Secondary = cache_->GetResource<Material>("Resources/Materials/PurpleGlowEnvmap.xml");
}

void MasterControl::CreateScene()
{
    world.scene = new Scene(context_);

    world.octree = world.scene->CreateComponent<Octree>();
    physicsWorld_ = world.scene->CreateComponent<PhysicsWorld>();
    physicsWorld_->SetGravity(Vector3::ZERO);
    world.scene->CreateComponent<DebugRenderer>();

    //Create a Zone component for fog control
    Node* zoneNode = world.scene->CreateChild("Zone");
    Zone* zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(Vector3(-100.0f, -50.0f, -100.0f),Vector3(100.0f, 5.0f, 100.0f)));
    zone->SetFogColor(Color(0.0f, 0.0f, 0.0f));
    zone->SetFogStart(56.8f);
    zone->SetFogEnd(61.8f);

    //Create cursor
    world.cursor.sceneCursor = world.scene->CreateChild("Cursor");
    StaticModel* cursorObject = world.cursor.sceneCursor->CreateComponent<StaticModel>();
    cursorObject->SetModel(cache_->GetResource<Model>("Resources/Models/Hexagon.mdl"));
    cursorObject->SetMaterial(cache_->GetResource<Material>("Resources/Materials/Glow.xml"));
    world.cursor.sceneCursor->SetEnabled(false);

    //Create an invisible plane for mouse raycasting
    world.voidNode = world.scene->CreateChild("MouseHitPlane");
    //Location is set in update since the plane moves with the camera.
    world.voidNode->SetScale(Vector3(1000.0f, 1.0f, 1000.0f));
    StaticModel* planeObject = world.voidNode->CreateComponent<StaticModel>();
    planeObject->SetModel(cache_->GetResource<Model>("Models/Plane.mdl"));
    planeObject->SetMaterial(cache_->GetResource<Material>("Resources/Materials/Invisible.xml"));

    //Create camera
    world.camera = new heXoCam(context_, this);

    //Create arena
    tileMaster_ = new TileMaster(context_, this);

    //Construct lobby
    lobbyNode_ = world.scene->CreateChild("Lobby");
    lobbyNode_->Rotate(Quaternion(0.0f, 180.0f, 0.0f));
    Node* chamberNode = lobbyNode_->CreateChild("Chamber");
    StaticModel* chamberModel = chamberNode->CreateComponent<StaticModel>();
    chamberModel->SetModel(cache_->GetResource<Model>("Resources/Models/Chamber.mdl"));
    chamberModel->SetMaterial(0, resources.materials.basic);
    chamberModel->SetMaterial(1, cache_->GetResource<Material>("Resources/Materials/PitchBlack.xml"));
    chamberModel->SetCastShadows(true);

    //Create player 1 ship
    Node* ship1Node = lobbyNode_->CreateChild("Ship");
    ship1Node->SetWorldPosition(Vector3(-4.2f, 0.6f, 0.0f));
    ship1Node->Rotate(Quaternion(270.0f, Vector3::UP));
    StaticModel* ship1 = ship1Node->CreateComponent<StaticModel>();
    ship1->SetModel(resources.models.ships.swift);
    ship1->SetMaterial(0, cache_->GetResource<Material>("Resources/Materials/GreenGlow.xml"));
    ship1->SetMaterial(1, cache_->GetResource<Material>("Resources/Materials/Green.xml"));
    ship1->SetCastShadows(true);
    RigidBody* ship1Body = ship1Node->CreateComponent<RigidBody>();
    ship1Body->SetTrigger(true);
    ship1Node->CreateComponent<CollisionShape>()->SetBox(Vector3::ONE * 2.23f);
    SubscribeToEvent(ship1Node, E_NODECOLLISIONSTART, URHO3D_HANDLER(MasterControl, HandlePlayTrigger1));


    //Create player 2 ship
    Node* ship2Node = lobbyNode_->CreateChild("Ship");
    ship2Node->SetWorldPosition(Vector3(4.2f, 0.6f, 0.0f));
    ship2Node->Rotate(Quaternion(90.0f, Vector3::UP));
    StaticModel* ship2 = ship2Node->CreateComponent<StaticModel>();
    ship2->SetModel(resources.models.ships.swift);
    ship2->SetMaterial(0, cache_->GetResource<Material>("Resources/Materials/PurpleGlow.xml"));
    ship2->SetMaterial(1, cache_->GetResource<Material>("Resources/Materials/Purple.xml"));
    ship2->SetCastShadows(true);
    RigidBody* ship2Body = ship2Node->CreateComponent<RigidBody>();
    ship2Body->SetTrigger(true);
    ship2Node->CreateComponent<CollisionShape>()->SetBox(Vector3::ONE * 2.23f);
    SubscribeToEvent(ship2Node, E_NODECOLLISIONSTART, URHO3D_HANDLER(MasterControl, HandlePlayTrigger2));

    lobbyNode_->CreateComponent<RigidBody>();
    lobbyNode_->CreateComponent<CollisionShape>()->SetConvexHull(cache_->GetResource<Model>("Resources/Models/CC_Center.mdl"));
    lobbyNode_->CreateComponent<CollisionShape>()->SetConvexHull(cache_->GetResource<Model>("Resources/Models/CC_CentralBox.mdl"));
    lobbyNode_->CreateComponent<CollisionShape>()->SetConvexHull(cache_->GetResource<Model>("Resources/Models/CC_Edge1.mdl"));
    lobbyNode_->CreateComponent<CollisionShape>()->SetConvexHull(cache_->GetResource<Model>("Resources/Models/CC_Edge2.mdl"));
    lobbyNode_->CreateComponent<CollisionShape>()->SetConvexHull(cache_->GetResource<Model>("Resources/Models/CC_Side1.mdl"));
    lobbyNode_->CreateComponent<CollisionShape>()->SetConvexHull(cache_->GetResource<Model>("Resources/Models/CC_Side2.mdl"));

    CollisionShape* edge2Shape = lobbyNode_->CreateComponent<CollisionShape>();
    edge2Shape->SetConvexHull(cache_->GetResource<Model>("Resources/Models/CC_Edge2.mdl"));
    edge2Shape->SetRotation(Quaternion(180.0f, Vector3::UP));
    CollisionShape* side1Shape = lobbyNode_->CreateComponent<CollisionShape>();
    side1Shape->SetConvexHull(cache_->GetResource<Model>("Resources/Models/CC_Side1.mdl"));
    side1Shape->SetRotation(Quaternion(180.0f, Vector3::UP));
    CollisionShape* side2Shape = lobbyNode_->CreateComponent<CollisionShape>();
    side2Shape->SetConvexHull(cache_->GetResource<Model>("Resources/Models/CC_Side2.mdl"));
    side2Shape->SetRotation(Quaternion(180.0f, Vector3::UP));

    Node* leftPointLightNode1 = lobbyNode_->CreateChild("PointLight");
    leftPointLightNode1->SetPosition(Vector3(2.3f, 2.3f, 3.2f));
    Light* leftPointLight1 = leftPointLightNode1->CreateComponent<Light>();
    leftPointLight1->SetLightType(LIGHT_POINT);
    leftPointLight1->SetBrightness(1.0f);
    leftPointLight1->SetColor(Color(0.42f, 1.0f, 0.1f));
    leftPointLight1->SetRange(10.0f);
    leftPointLight1->SetCastShadows(true);
    leftPointLight1->SetShadowBias(BiasParameters(0.0001f, 0.1f));

    Node* leftPointLightNode2 = lobbyNode_->CreateChild("PointLight");
    leftPointLightNode2->SetPosition(Vector3(2.3f, 2.3f, -3.2f));
    Light* leftPointLight2 = leftPointLightNode2->CreateComponent<Light>();
    leftPointLight2->SetLightType(LIGHT_POINT);
    leftPointLight2->SetBrightness(1.0f);
    leftPointLight2->SetColor(Color(0.42f, 1.0f, 0.1f));
    leftPointLight2->SetRange(10.0f);
    leftPointLight2->SetCastShadows(true);
    leftPointLight2->SetShadowBias(BiasParameters(0.0001f, 0.1f));

    Node* rightPointLightNode1 = lobbyNode_->CreateChild("PointLight");
    rightPointLightNode1->SetPosition(Vector3(-2.3f, 2.3f, 3.2f));
    Light* rightPointLight1 = rightPointLightNode1->CreateComponent<Light>();
    rightPointLight1->SetLightType(LIGHT_POINT);
    rightPointLight1->SetBrightness(1.0f);
    rightPointLight1->SetColor(Color(0.42f, 0.1f, 1.0f));
    rightPointLight1->SetRange(10.0f);
    rightPointLight1->SetCastShadows(true);
    rightPointLight1->SetShadowBias(BiasParameters(0.0001f, 0.1f));

    Node* rightPointLightNode2 = lobbyNode_->CreateChild("PointLight");
    rightPointLightNode2->SetPosition(Vector3(-2.3f, 2.3f, -3.2f));
    Light* rightPointLight2 = rightPointLightNode2->CreateComponent<Light>();
    rightPointLight2->SetLightType(LIGHT_POINT);
    rightPointLight2->SetBrightness(1.0f);
    rightPointLight2->SetColor(Color(0.42f, 0.1f, 1.0f));
    rightPointLight2->SetRange(10.0f);
    rightPointLight2->SetCastShadows(true);
    rightPointLight2->SetShadowBias(BiasParameters(0.0001f, 0.1f));

//    for (int i = 0; i < 6; i++){
//        Node* edgeNode = floorNode->CreateChild("LobbyEdge");
//        edgeNode->Rotate(Quaternion(0.0f, (60.0f * i), 0.0f));
//        Model* model = cache_->GetResource<Model>("Resources/Models/ArenaEdgeSegment.mdl");
//        edgeNode->CreateComponent<RigidBody>();
//        CollisionShape* collider = edgeNode->CreateComponent<CollisionShape>();
//        collider->SetConvexHull(model);
//        SubscribeToEvent(edgeNode, E_NODECOLLISIONSTART, URHO3D_HANDLER(MasterControl, HandleExitTrigger));
//    }
    //Create game elements
    spawnMaster_ = new SpawnMaster(context_, this);
    player1_ = new Player(context_, this, 1);
    player2_ = new Player(context_, this, 2);
    apple_ = new Apple(context_, this);
    heart_ = new Heart(context_, this);
    multiX_ = new MultiX(context_, this);
    chaoBall_ = new ChaoBall(context_, this);
}

void MasterControl::SetGameState(const GameState newState)
{
    if (newState != currentState_){
        LeaveGameState();
        currentState_ = newState;
        sinceStateChange_ = 0.0f;
        EnterGameState();
    }
}
void MasterControl::LeaveGameState()
{
    switch (currentState_){
    case GS_INTRO : break;
    case GS_LOBBY : {
        lobbyNode_->SetEnabledRecursive(false);
    } break;
    case GS_PLAY : {
        spawnMaster_->Deactivate();
    } break; //Eject when alive
    case GS_DEAD : {
        player1_->CreateNewPilot();
        player1_->ResetScore();
        player2_->CreateNewPilot();
        player2_->ResetScore();
        world.camera->SetGreyScale(false);
    } break;
    case GS_EDIT : break; //Disable EditMaster
    default: break;
    }
}
void MasterControl::EnterGameState()
{
    switch (currentState_){
    case GS_INTRO : break;
    case GS_LOBBY : {
//        musicSource_->Play(menuMusic_);
        musicSource_->Stop();
        lobbyNode_->SetEnabledRecursive(true);
        player1_->EnterLobby();
        player2_->EnterLobby();
        world.camera->EnterLobby();
        spawnMaster_->Clear();
        tileMaster_->EnterLobbyState();

        apple_->Disable();
        heart_->Disable();
        multiX_->Disable();
        chaoBall_->Disable();
    } break;
    case GS_PLAY : {
        musicSource_->Play(gameMusic_);
        player1_->EnterPlay();
        player2_->EnterPlay();
        world.camera->EnterPlay();

        apple_->Respawn(true);
        heart_->Respawn(true);
        multiX_->Deactivate();
        chaoBall_->Deactivate();

        world.lastReset = world.scene->GetElapsedTime();
        spawnMaster_->Restart();
        tileMaster_->EnterPlayState();
    } break;
    case GS_DEAD : {
        spawnMaster_->Deactivate();
        world.camera->SetGreyScale(true);
    } break;
    default: break;
    }
}

void MasterControl::Eject()
{
    SetGameState(GS_LOBBY);
}

void MasterControl::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
    double timeStep = eventData[Update::P_TIMESTEP].GetFloat();
    sinceStateChange_ += timeStep;
    UpdateCursor(timeStep);

    resources.materials.basic->SetShaderParameter("MatDiffColor", resources.materials.basic->GetShaderParameter("MatDiffColor").GetColor().Lerp(
                                          GetGameState() == GS_LOBBY
                                          ? Color(0.142f, 0.132f, 0.13f) * Sine(5.0f, 0.75f, 1.0f, 1.23f) * Min(2.3f * sinceStateChange_, 1.0f)
                                          : Color(0.0f, 0.0f, 0.0f, 0.0f), timeStep));

    switch (currentState_){
    case GS_LOBBY: {
//        musicSource_->SetGain(Max(0.0f, 1.0f - sinceStateChange_));
        lobbyNode_->SetPosition((Vector3::DOWN * 23.0f) / (128.0f * sinceStateChange_+23.0f));
    }
        break;
    case GS_PLAY: {
//        musicSource_->SetGain(Min(0.666f, sinceStateChange_ * 0.023f));
        lobbyNode_->SetPosition((Vector3::DOWN * 23.0f) * Min(1.0f, sinceStateChange_));
    } break;
    }
}

void MasterControl::UpdateCursor(const float timeStep)
{
    world.cursor.sceneCursor->Rotate(Quaternion(0.0f,100.0f*timeStep,0.0f));
    //world.cursor.sceneCursor->SetScale((world.cursor.sceneCursor->GetWorldPosition() - world.camera->GetWorldPosition()).Length()*0.05f);
    if (CursorRayCast(250.0f, world.cursor.hitResults))
    {
        for (unsigned i = 0; i < world.cursor.hitResults.Size(); i++)
        {
            if (world.cursor.hitResults[i].node_->GetNameHash() == N_TILE)
            {
                world.cursor.sceneCursor->SetWorldPosition(world.cursor.hitResults[i].node_->GetPosition()+Vector3::UP);
                /*Vector3 camHitDifference = world.camera->rootNode_->GetWorldPosition() - world.cursor.hitResults[i].position_;
                camHitDifference /= world.camera->rootNode_->GetWorldPosition().y_ - world.voidNode->GetPosition().y_;
                camHitDifference *= world.camera->rootNode_->GetWorldPosition().y_;
                world.cursor.sceneCursor->SetWorldPosition(world.camera->rootNode_->GetWorldPosition()-camHitDifference);*/
            }
        }
    }
}

bool MasterControl::CursorRayCast(const float maxDistance, PODVector<RayQueryResult> &hitResults)
{
    IntVector2 mousePos = world.cursor.uiCursor->GetPosition();
    Ray cameraRay = world.camera->camera_->GetScreenRay((float)mousePos.x_/graphics_->GetWidth(),
                                                        (float)mousePos.y_/graphics_->GetHeight());
    RayOctreeQuery query(hitResults, cameraRay, RAY_TRIANGLE, maxDistance, DRAWABLE_GEOMETRY);
    world.scene->GetComponent<Octree>()->Raycast(query);
    if (hitResults.Size()) return true;
    else return false;
}

bool MasterControl::PhysicsRayCast(PODVector<PhysicsRaycastResult> &hitResults, const Ray ray,
                                   const float distance, const unsigned collisionMask = M_MAX_UNSIGNED)
{
    if (distance > 1.0e-9)
        physicsWorld_->Raycast(hitResults, ray, distance, collisionMask);

    return (hitResults.Size() > 0);
}

bool MasterControl::PhysicsSphereCast(PODVector<RigidBody*> &hitResults, const Vector3 center,
                                      const float radius, const unsigned collisionMask = M_MAX_UNSIGNED)
{
    physicsWorld_->GetRigidBodies(hitResults, Sphere(center, radius), collisionMask);
    if (hitResults.Size()) return true;
    else return false;
}

void MasterControl::Exit()
{

    //Save player1 pilot to file
    std::ofstream fPilot1;
    fPilot1.open("Resources/Pilot1.lkp");
    fPilot1 << player1_->pilot_.male_ << '\n';
    fPilot1 << player1_->pilot_.hairStyle_ << '\n';
    for (unsigned c = 0; c < player1_->pilot_.colors_.Size(); c++){
        fPilot1 << player1_->pilot_.colors_[c].r_ << ' '
               << player1_->pilot_.colors_[c].g_ << ' '
               << player1_->pilot_.colors_[c].b_ << ' '
               << '\n';
    }
    fPilot1 << player1_->GetScore();

    //Save player2 pilot to file
    std::ofstream fPilot2;
    fPilot2.open("Resources/Pilot2.lkp");
    fPilot2 << player2_->pilot_.male_ << '\n';
    fPilot2 << player2_->pilot_.hairStyle_ << '\n';
    for (unsigned c = 0; c < player1_->pilot_.colors_.Size(); c++){
        fPilot2 << player2_->pilot_.colors_[c].r_ << ' '
               << player2_->pilot_.colors_[c].g_ << ' '
               << player2_->pilot_.colors_[c].b_ << ' '
               << '\n';
    }
    fPilot2 << player2_->GetScore();

    //And exit
    engine_->Exit();
}

void MasterControl::CreateSineLookupTable()
{
    //Populate sine lookup vector
    int resolution = 4096;
    for (int i = 0; i < resolution; i++){
        sine_.Push(sin(((float)i/resolution)*2.0*M_PI));
    }
}

float MasterControl::Sine(const float freq, const float min, const float max, const float shift)
{
    float phase = freq * world.scene->GetElapsedTime() + shift;
    float add = 0.5f*(min+max);
    return Sine(phase) * 0.5f * (max - min) + add;
}
