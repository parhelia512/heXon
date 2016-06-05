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

#include "mastercontrol.h"

#include <fstream>
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Core/Context.h>
#include "TailGenerator.h"

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
#include "pilot.h"
#include "splatterpillar.h"
#include "door.h"

URHO3D_DEFINE_APPLICATION_MAIN(MasterControl);

MasterControl* MasterControl::instance_ = NULL;

MasterControl* MasterControl::GetInstance()
{
    return MasterControl::instance_;
}

MasterControl::MasterControl(Context *context):
    Application(context),
    aspectRatio_{},
    paused_{false},
    currentState_{GS_INTRO},
    sinceStateChange_{0.0f},
    sinceFrameRateReport_{0.0f}
{
    instance_ = this;
}

void MasterControl::Setup()
{
    SetRandomSeed(GetSubsystem<Time>()->GetSystemTime());

    FileSystem* fs{GetSubsystem<FileSystem>()};
    String resourcePath{"/usr/local/share/hexon"};
    if (!fs->DirExists(resourcePath)){
        resourcePath = "Resources";
    }

    if (fs->DirExists(resourcePath))
        engineParameters_["ResourcePaths"] = resourcePath;

    // Modify engine startup parameters.
    //Set custom window title and icon.
    engineParameters_["WindowTitle"] = "heXon";
    engineParameters_["LogName"] = GetSubsystem<FileSystem>()->GetAppPreferencesDir("urho3d", "logs")+"heXon.log";
    engineParameters_["WindowIcon"] = "icon.png";

//    engineParameters_["VSync"] = true;
//    engineParameters_["FullScreen"] = false;
//    engineParameters_["Headless"] = true;
//    engineParameters_["WindowWidth"] = 1280;
//    engineParameters_["WindowHeight"] = 1024;
//    engineParameters_["RenderPath"] = "RenderPaths/ForwardOutline.xml";
}
void MasterControl::Start()
{
    ENGINE->SetMaxFps(100);
    aspectRatio_ = static_cast<float>(GRAPHICS->GetWidth()) / GRAPHICS->GetHeight();

    TailGenerator::RegisterObject(context_);

    new InputMaster();
    renderer_ = GetSubsystem<Renderer>();

    LoadHair();

    // Get default style
    defaultStyle_ = CACHE->GetResource<XMLFile>("UI/DefaultStyle.xml");
    //Create console and debug HUD.
    CreateConsoleAndDebugHud();
    //Create the scene content
    CreateScene();
    //Create the UI content
    CreateUI();
    //Hook up to the frame update and render post-update events

    menuMusic_ = GetMusic("Modanung - BulletProof MagiRex");
    menuMusic_->SetLooped(true);
    gameMusic_ = GetMusic("Alien Chaos - Disorder");
    gameMusic_->SetLooped(true);
    Node* musicNode{world.scene->CreateChild("Music")};
    musicSource_ = musicNode->CreateComponent<SoundSource>();
    musicSource_->SetGain(0.32f);
    musicSource_->SetSoundType(SOUND_MUSIC);

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
    Console* console{engine_->CreateConsole()};
    console->SetDefaultStyle(defaultStyle_);
    console->GetBackground()->SetOpacity(0.8f);

    // Create debug HUD.
    DebugHud* debugHud{engine_->CreateDebugHud()};
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
    world.cursor.uiCursor->SetPosition(GRAPHICS->GetWidth()/2, GRAPHICS->GetHeight()/2);
}

Sound* MasterControl::GetMusic(String name) const {
    Sound* song{CACHE->GetResource<Sound>("Music/"+name+".ogg")};
    song->SetLooped(true);
    return song;
}
Sound*MasterControl::GetSample(String name) const {
    Sound* sample{CACHE->GetResource<Sound>("Samples/"+name+".ogg")};
    sample->SetLooped(false);
    return sample;
}

void MasterControl::LoadHair()
{
    //Load hairmodels
    hairStyles_.Push(SharedPtr<Model>(GetModel("Mohawk")));
    hairStyles_.Push(SharedPtr<Model>(GetModel("Seagull")));
    hairStyles_.Push(SharedPtr<Model>(GetModel("Mustain")));
    hairStyles_.Push(SharedPtr<Model>(GetModel("Frotoad")));
}

void MasterControl::LoadHighest()
{
    highestPilot_->Load("Resources/.Pilot0.lkp", highestScore_);
    if (highestScore_ == 0){
        highestNode_->SetEnabledRecursive(false);
        highestScoreText_->SetColor(Color{0.0f, 0.0f, 0.0f, 0.0f});

    } else {
        podiumNode_->SetRotation(Quaternion::IDENTITY);
        podiumNode_->Rotate(Quaternion(LucKey::RandomSign()*30.0f, Vector3::UP));
        highestNode_->SetEnabledRecursive(true);
        highestScoreText_->SetText(String(highestScore_));
        highestScoreText_->SetColor(Color(0.23f, 0.75f, 1.0f, 0.75f));
    }
}

void MasterControl::CreateScene()
{
    world.scene = new Scene(context_);
    world.scene->SetTimeScale(1.23f);

    world.octree = world.scene->CreateComponent<Octree>();
    physicsWorld_ = world.scene->CreateComponent<PhysicsWorld>();
    physicsWorld_->SetGravity(Vector3::ZERO);
    world.scene->CreateComponent<DebugRenderer>();

    //Create a Zone component for fog control
    Node* zoneNode{world.scene->CreateChild("Zone")};
    Zone* zone{zoneNode->CreateComponent<Zone>()};
    zone->SetBoundingBox(BoundingBox(Vector3(-100.0f, -100.0f, -100.0f),Vector3(100.0f, 5.0f, 100.0f)));
    zone->SetFogColor(Color(0.0f, 0.0f, 0.0f));
    zone->SetFogStart(56.8f);
    zone->SetFogEnd(61.8f);
    zone->SetHeightFog(true);
    zone->SetFogHeight(-10.0f);
    zone->SetFogHeightScale(0.23f);

    //Create cursor
    world.cursor.sceneCursor = world.scene->CreateChild("Cursor");
    StaticModel* cursorObject{world.cursor.sceneCursor->CreateComponent<StaticModel>()};
    cursorObject->SetModel(GetModel("Hexagon"));
    cursorObject->SetMaterial(GetMaterial("Glow"));
    world.cursor.sceneCursor->SetEnabled(false);

    //Create an solid black plane to hide everything beyond full fog
    world.voidNode = world.scene->CreateChild("Void");
    world.voidNode->SetPosition(Vector3::DOWN * 10.0f);
    world.voidNode->SetScale(Vector3(1000.0f, 1.0f, 1000.0f));
    StaticModel* planeObject{world.voidNode->CreateComponent<StaticModel>()};
    planeObject->SetModel(GetModel("Plane"));
    planeObject->SetMaterial(GetMaterial("PitchBlack"));

    //Create camera
    world.camera = new heXoCam();

    //Create arena
    tileMaster_ = new TileMaster();

    //Construct lobby
    lobbyNode_ = world.scene->CreateChild("Lobby");
    lobbyNode_->Rotate(Quaternion(0.0f, 0.0f, 0.0f));
    Node* chamberNode{lobbyNode_->CreateChild("Chamber")};
    StaticModel* chamberModel{chamberNode->CreateComponent<StaticModel>()};
    chamberModel->SetModel(GetModel("Chamber"));
    chamberModel->SetMaterial(0, GetMaterial("Basic"));
    chamberModel->SetMaterial(1, GetMaterial("PitchBlack"));
    chamberModel->SetMaterial(4, GetMaterial("Drain"));
    lobbyGlowGreen_ = GetMaterial("GreenGlow");
    chamberModel->SetMaterial(2, lobbyGlowGreen_);
    lobbyGlowPurple_ = GetMaterial("PurpleGlow");
    chamberModel->SetMaterial(3, lobbyGlowPurple_);
    chamberModel->SetCastShadows(true);

    //Create player 1 lobby ship
    Node* ship1Node{lobbyNode_->CreateChild("Ship")};
    ship1Node->SetWorldPosition(Vector3(-4.2f, 0.6f, 0.0f));
    ship1Node->Rotate(Quaternion(90.0f, Vector3::UP));
    StaticModel* ship1{ship1Node->CreateComponent<StaticModel>()};
    ship1->SetModel(GetModel("KlåMk10"));
    ship1->SetMaterial(0, GetMaterial("GreenGlow"));
    ship1->SetMaterial(1, GetMaterial("Green"));
    ship1->SetCastShadows(true);
    RigidBody* ship1Body{ship1Node->CreateComponent<RigidBody>()};
    ship1Body->SetTrigger(true);
    ship1Node->CreateComponent<CollisionShape>()->SetBox(Vector3::ONE * 2.23f);
    SubscribeToEvent(ship1Node, E_NODECOLLISIONSTART, URHO3D_HANDLER(MasterControl, HandlePlayTrigger1));

    //Create player 2 lobby ship
    Node* ship2Node{lobbyNode_->CreateChild("Ship")};
    ship2Node->SetWorldPosition(Vector3(4.2f, 0.6f, 0.0f));
    ship2Node->Rotate(Quaternion(270.0f, Vector3::UP));
    StaticModel* ship2{ship2Node->CreateComponent<StaticModel>()};
    ship2->SetModel(GetModel("KlåMk10"));
    ship2->SetMaterial(0, GetMaterial("PurpleGlow"));
    ship2->SetMaterial(1, GetMaterial("Purple"));
    ship2->SetCastShadows(true);
    RigidBody* ship2Body{ship2Node->CreateComponent<RigidBody>()};
    ship2Body->SetTrigger(true);
    ship2Node->CreateComponent<CollisionShape>()->SetBox(Vector3::ONE * 2.23f);
    SubscribeToEvent(ship2Node, E_NODECOLLISIONSTART, URHO3D_HANDLER(MasterControl, HandlePlayTrigger2));

    //Create highest
    highestNode_ = lobbyNode_->CreateChild("Highest");
    highestNode_->Translate(Vector3(0.0f, 3.2f, -5.0f));
    highestNode_->Rotate(Quaternion(45.0f, Vector3::RIGHT));
    highestNode_->Rotate(Quaternion(180.0f, Vector3::UP));
    podiumNode_ = highestNode_->CreateChild("Podium");
    podiumNode_->SetScale(0.5f);
    StaticModel* hexPodium{podiumNode_->CreateComponent<StaticModel>()};
    hexPodium->SetModel(GetModel("Hexagon"));
    hexPodium->SetMaterial(GetMaterial("BackgroundTile")->Clone());
    Node* highestPilotNode{podiumNode_->CreateChild("HighestPilot")};
    highestPilotNode->SetScale(2.0f);


    Node* spotNode{highestNode_->CreateChild("HighestSpot")};
    spotNode->Translate(Vector3(0.0f, -2.0f, 3.0f));
    spotNode->LookAt(highestNode_->GetWorldPosition());
    Light* highestSpot{spotNode->CreateComponent<Light>()};
    highestSpot->SetLightType(LIGHT_SPOT);
    highestSpot->SetRange(5.0f);
    highestSpot->SetFov(23.5f);
    highestSpot->SetColor(Color(0.6f, 0.7f, 1.0f));
    highestSpot->SetBrightness(5.0f);
    highestSpot->SetSpecularIntensity(0.23f);

    UI* ui{GetSubsystem<UI>()};
    highestScoreText_ = ui->GetRoot()->CreateChild<Text>();
    highestScoreText_->SetName("HighestScore");
    highestScoreText_->SetText("0");
    highestScoreText_->SetFont(CACHE->GetResource<Font>("Fonts/skirmishergrad.ttf"), 23);
    highestScoreText_->SetColor(Color(0.23f, 0.75f, 1.0f, 0.75f));
    highestScoreText_->SetHorizontalAlignment(HA_CENTER);
    highestScoreText_->SetVerticalAlignment(VA_CENTER);
    highestScoreText_->SetPosition(0, ui->GetRoot()->GetHeight()/2.13f);

    highestPilot_ = new Pilot(highestPilotNode);
    LoadHighest();

    lobbyNode_->CreateComponent<RigidBody>();
    lobbyNode_->CreateComponent<CollisionShape>()->SetConvexHull(GetModel("CC_Center"));
    lobbyNode_->CreateComponent<CollisionShape>()->SetConvexHull(GetModel("CC_CentralBox"));
    lobbyNode_->CreateComponent<CollisionShape>()->SetConvexHull(GetModel("CC_Edge1"));
    lobbyNode_->CreateComponent<CollisionShape>()->SetConvexHull(GetModel("CC_Edge2"));
    lobbyNode_->CreateComponent<CollisionShape>()->SetConvexHull(GetModel("CC_Side1"));
    lobbyNode_->CreateComponent<CollisionShape>()->SetConvexHull(GetModel("CC_Side2"));

    CollisionShape* edge1Shape{lobbyNode_->CreateComponent<CollisionShape>()};
    edge1Shape->SetConvexHull(GetModel("CC_Edge1"));
    edge1Shape->SetPosition(Vector3::FORWARD * 1.23f);
    edge1Shape->SetRotation(Quaternion(180.0f, Vector3::UP));
    CollisionShape* edge2Shape{lobbyNode_->CreateComponent<CollisionShape>()};
    edge2Shape->SetConvexHull(GetModel("CC_Edge2"));
    edge2Shape->SetRotation(Quaternion(180.0f, Vector3::UP));
    CollisionShape* side1Shape{lobbyNode_->CreateComponent<CollisionShape>()};
    side1Shape->SetConvexHull(GetModel("CC_Side1"));
    side1Shape->SetRotation(Quaternion(180.0f, Vector3::UP));
    CollisionShape* side2Shape{lobbyNode_->CreateComponent<CollisionShape>()};
    side2Shape->SetConvexHull(GetModel("CC_Side2"));
    side2Shape->SetRotation(Quaternion(180.0f, Vector3::UP));


    Node* leftPointLightNode1{lobbyNode_->CreateChild("PointLight")};
    leftPointLightNode1->SetPosition(Vector3(-2.3f, 2.3f, 3.0f));
    leftPointLight1_ = leftPointLightNode1->CreateComponent<Light>();
    leftPointLight1_->SetLightType(LIGHT_POINT);
    leftPointLight1_->SetBrightness(0.83f);
    leftPointLight1_->SetColor(Color(0.42f, 1.0f, 0.1f));
    leftPointLight1_->SetRange(13.0f);
    leftPointLight1_->SetCastShadows(true);
    leftPointLight1_->SetShadowBias(BiasParameters(0.0001f, 0.1f));

    Node* leftPointLightNode2{lobbyNode_->CreateChild("PointLight")};
    leftPointLightNode2->SetPosition(Vector3(-2.3f, 2.3f, -3.0f));
    leftPointLight2_ = leftPointLightNode2->CreateComponent<Light>();
    leftPointLight2_->SetLightType(LIGHT_POINT);
    leftPointLight2_->SetBrightness(0.83f);
    leftPointLight2_->SetColor(Color(0.42f, 1.0f, 0.1f));
    leftPointLight2_->SetRange(13.0f);
    leftPointLight2_->SetCastShadows(true);
    leftPointLight2_->SetShadowBias(BiasParameters(0.0001f, 0.1f));

    Node* rightPointLightNode1{lobbyNode_->CreateChild("PointLight")};
    rightPointLightNode1->SetPosition(Vector3(2.3f, 2.3f, 3.0f));
    rightPointLight1_ = rightPointLightNode1->CreateComponent<Light>();
    rightPointLight1_->SetLightType(LIGHT_POINT);
    rightPointLight1_->SetBrightness(1.0f);
    rightPointLight1_->SetColor(Color(0.42f, 0.1f, 1.0f));
    rightPointLight1_->SetRange(13.0f);
    rightPointLight1_->SetCastShadows(true);
    rightPointLight1_->SetShadowBias(BiasParameters(0.0001f, 0.1f));

    Node* rightPointLightNode2{lobbyNode_->CreateChild("PointLight")};
    rightPointLightNode2->SetPosition(Vector3(2.3f, 2.3f, -3.0f));
    rightPointLight2_ = rightPointLightNode2->CreateComponent<Light>();
    rightPointLight2_->SetLightType(LIGHT_POINT);
    rightPointLight2_->SetBrightness(1.0f);
    rightPointLight2_->SetColor(Color(0.42f, 0.1f, 1.0f));
    rightPointLight2_->SetRange(13.0f);
    rightPointLight2_->SetCastShadows(true);
    rightPointLight2_->SetShadowBias(BiasParameters(0.0001f, 0.1f));

    //Create game elements
    spawnMaster_ = new SpawnMaster();

    player1_ = new Player(1);
    player2_ = new Player(2);
    players_[player1_->GetRootNodeID()] = player1_;
    players_[player2_->GetRootNodeID()] = player2_;


    door1_ = new Door(false);
    door2_ = new Door(true);

    splatterPillar1_ = new SplatterPillar(false);
    splatterPillar2_ = new SplatterPillar(true);

    apple_ = new Apple();
    heart_ = new Heart();
    multiX_ = new MultiX();
    chaoBall_ = new ChaoBall();
}

void MasterControl::SetGameState(const GameState newState)
{
    if (newState != currentState_) {
        LeaveGameState();
        previousState_ = currentState_;
        currentState_ = newState;
        sinceStateChange_ = 0.0f;
        EnterGameState();
    }
}
void MasterControl::LeaveGameState()
{
    switch (currentState_) {
    case GS_INTRO : break;
    case GS_LOBBY : {
        lobbyNode_->SetEnabledRecursive(false);
        if (highestScore_ != 0)
            highestScoreText_->SetColor(Color(0.23f, 0.75f, 1.0f, 0.23f));
    } break;
    case GS_PLAY : {
        spawnMaster_->Deactivate();
    } break; //Eject when alive
    case GS_DEAD : {
        world.camera->SetGreyScale(false);
    } break;
    case GS_EDIT : break; //Disable EditMaster
    default: break;
    }
}
void MasterControl::EnterGameState()
{
    switch (currentState_) {
    case GS_INTRO : break;
    case GS_LOBBY : {
        GetPlayer(1)->EnterLobby();
        GetPlayer(2)->EnterLobby();
        musicSource_->Play(menuMusic_);
        lobbyNode_->SetEnabledRecursive(true);
        world.camera->EnterLobby();
        spawnMaster_->Clear();
        tileMaster_->EnterLobbyState();
        highestNode_->SetEnabledRecursive(highestScore_ != 0);
        highestScoreText_->SetColor(Color(0.23f, 0.75f, 1.0f, 0.75f) * static_cast<float>(highestScore_ != 0));

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
        musicSource_->SetGain(musicSource_->GetGain() * 0.666f);
    } break;
    default: break;
    }
}

void MasterControl::Eject()
{
    SetGameState(GS_LOBBY);
}

void MasterControl::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{ (void)eventType;

    float t{eventData[Update::P_TIMESTEP].GetFloat()};

/*
    //Output FPS
    secondsPerFrame_ *= 1.0f - t;
    secondsPerFrame_ += t * t;
    sinceFrameRateReport_ += t;
    if (sinceFrameRateReport_ >= 1.0f) {
        Log::Write(1, String(1.0f / secondsPerFrame_));
        sinceFrameRateReport_ = 0.0f;
    }
*/
    sinceStateChange_ += t;
    UpdateCursor(t);

    switch (currentState_) {
    case GS_LOBBY: {
        lobbyNode_->SetPosition((Vector3::DOWN * 23.0f) / (128.0f * sinceStateChange_+23.0f));

        leftPointLight1_->SetBrightness(Sine(1.0f, 0.666f, 0.95f));
        leftPointLight2_->SetBrightness(Sine(1.0f, 0.666f, 0.95f, M_PI));
        rightPointLight1_->SetBrightness(Sine(1.0f, 0.95f, 1.23f));
        rightPointLight2_->SetBrightness(Sine(1.0f, 0.95f, 1.23f, M_PI));

        if (door1_->HidesPlayer() > 0.5f && door2_->HidesPlayer() > 0.5f){
                Exit();
        }
    } break;
    case GS_PLAY: {
        lobbyNode_->SetPosition((Vector3::DOWN * 23.0f) * Min(1.0f, sinceStateChange_));
    } break;
    case GS_DEAD: {
        if (sinceStateChange_ > 5.0f && NoHumans())
            SetGameState(GS_LOBBY);
    }
    default: break;
    }
}

void MasterControl::UpdateCursor(const float timeStep)
{ (void)timeStep;/*
    world.cursor.sceneCursor->Rotate(Quaternion(0.0f,100.0f*timeStep,0.0f));
    //world.cursor.sceneCursor->SetScale((world.cursor.sceneCursor->GetWorldPosition() - world.camera->GetWorldPosition()).Length()*0.05f);
    if (CursorRayCast(250.0f, world.cursor.hitResults))
        for (RayQueryResult r : world.cursor.hitResults)
            if (r.node_->GetNameHash() == N_TILE)
                world.cursor.sceneCursor->SetWorldPosition(r.node_->GetPosition()+Vector3::UP);
                Vector3 camHitDifference = world.camera->rootNode_->GetWorldPosition() - world.cursor.hitResults[i].position_;
                camHitDifference /= world.camera->rootNode_->GetWorldPosition().y_ - world.voidNode->GetPosition().y_;
                camHitDifference *= world.camera->rootNode_->GetWorldPosition().y_;
                world.cursor.sceneCursor->SetWorldPosition(world.camera->rootNode_->GetWorldPosition()-camHitDifference);*/
}

bool MasterControl::CursorRayCast(const float maxDistance, PODVector<RayQueryResult> &hitResults)
{
    IntVector2 mousePos{world.cursor.uiCursor->GetPosition()};
    Ray cameraRay{world.camera->camera_->GetScreenRay((float)mousePos.x_/GRAPHICS->GetWidth(),
                                                        (float)mousePos.y_/GRAPHICS->GetHeight())};
    RayOctreeQuery query{hitResults, cameraRay, RAY_TRIANGLE, maxDistance, DRAWABLE_GEOMETRY};
    world.scene->GetComponent<Octree>()->Raycast(query);

    if (hitResults.Size())
        return true;
    else
        return false;
}

bool MasterControl::PhysicsRayCast(PODVector<PhysicsRaycastResult> &hitResults, const Ray ray,
                                   const float distance, const unsigned collisionMask)
{
    if (distance > 1.0e-9)
        physicsWorld_->Raycast(hitResults, ray, distance, collisionMask);

    return (hitResults.Size() > 0);
}

bool MasterControl::PhysicsSphereCast(PODVector<RigidBody*> &hitResults, const Vector3 center,
                                      const float radius, const unsigned collisionMask)
{
    physicsWorld_->GetRigidBodies(hitResults, Sphere(center, radius), collisionMask);
    if (hitResults.Size()) return true;
    else return false;
}

void MasterControl::Exit()
{
    //Save pilots and their scores
    player1_->SavePilot();
    player2_->SavePilot();

    //...and exit to the left
    engine_->Exit();
}

float MasterControl::Sine(const float freq, const float min, const float max, const float shift)
{
    float phase{freq * world.scene->GetElapsedTime() + shift};
    float add{0.5f * (min + max)};
    return LucKey::Sine(phase) * 0.5f * (max - min) + add;
}
float MasterControl::Cosine(const float freq, const float min, const float max, const float shift)
{
    float phase{freq * world.scene->GetElapsedTime() + shift};
    float add{0.5f * (min + max)};
    return LucKey::Cosine(phase) * 0.5f * (max - min) + add;
}

Player* MasterControl::GetPlayer(int playerID, bool other) const
{
    if (!other)
        return playerID == 1 ? player1_.Get() : player2_.Get();
    else
        return playerID != 1 ? player1_.Get() : player2_.Get();
}

bool MasterControl::NoHumans()
{
    return !GetPlayer(1)->IsHuman() && !GetPlayer(2)->IsHuman();
}
