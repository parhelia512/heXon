#include "chaomine.h"
#include "player.h"

ChaoMine::ChaoMine(Context *context, MasterControl *masterControl): Enemy(context, masterControl),
    playerID_{0}
{
    rootNode_->SetName("ChaoMine");

    rigidBody_->SetMass(0.5f);
    //Overrides Enemy values
    meleeDamage_ = 0.1f;
    initialHealth_ = 0.05f;
    worth_ = 1;

    countDown_ = Random(1.0f, 5.0f);
    innerNode_ = rootNode_->CreateChild();
    innerModel_ = innerNode_->CreateComponent<StaticModel>();
    innerModel_->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/MineInner.mdl"));
    innerModel_->SetMaterial(0, masterControl_->resources.materials.ship1Primary);

    outerNode_ = rootNode_->CreateChild();
    outerModel_ = outerNode_->CreateComponent<StaticModel>();
    outerModel_->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/MineOuter.mdl"));
    outerModel_->SetMaterial(0, masterControl_->resources.materials.ship1Secondary);
    outerModel_->SetMaterial(1, masterControl_->resources.materials.ship1Primary);

}

void ChaoMine::Set(const Vector3 position, int playerID)
{
    playerID_ = playerID;

    if (playerID_ == 1){
        innerModel_->SetMaterial(0, masterControl_->resources.materials.ship1Primary);
        outerModel_->SetMaterial(0, masterControl_->resources.materials.ship1Secondary);
        outerModel_->SetMaterial(1, masterControl_->resources.materials.ship1Primary);
    } else if (playerID_ == 2){
        innerModel_->SetMaterial(0, masterControl_->resources.materials.ship2Primary);
        outerModel_->SetMaterial(0, masterControl_->resources.materials.ship2Secondary);
        outerModel_->SetMaterial(1, masterControl_->resources.materials.ship2Primary);
    }


    Enemy::Set(position);
    SubscribeToEvent(E_SCENEPOSTUPDATE, URHO3D_HANDLER(ChaoMine, HandleMineUpdate));
}

void ChaoMine::HandleMineUpdate(StringHash eventType, VariantMap &eventData)
{
    float timeStep = eventData[ScenePostUpdate::P_TIMESTEP].GetFloat();

    //Spin
    innerNode_->Rotate(Quaternion(50.0f*timeStep, 80.0f*timeStep, 92.0f*timeStep));
    outerNode_->Rotate(Quaternion(-60.0f*timeStep,-101.0f*timeStep, -95.0f*timeStep));
}

void ChaoMine::CheckHealth()
{
    if (rootNode_->IsEnabled() && health_ <= 0 || panicTime_ > 23.0f){
        masterControl_->spawnMaster_->SpawnChaoZap(GetPosition(), playerID_);
        Disable();
    }
}

void ChaoMine::HandleCollision(StringHash eventType, VariantMap &eventData)
{
    PODVector<RigidBody*> collidingBodies;
    rigidBody_->GetCollidingBodies(collidingBodies);

    for (unsigned b = 0; b < collidingBodies.Size(); ++b) {
        StringHash colliderNodeNameHash = collidingBodies[b]->GetNode()->GetNameHash();
        if (    colliderNodeNameHash == N_RAZOR ||
                colliderNodeNameHash == N_SPIRE   ) {
            SetHealth(0.0f);
        }
    }
}
