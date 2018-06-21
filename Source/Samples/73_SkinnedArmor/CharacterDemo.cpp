//
// Copyright (c) 2008-2016 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Graphics/DebugRenderer.h>

#include "CharacterDemo.h"
#include "Character.h"
#include "CollisionLayer.h"

#include <Urho3D/DebugNew.h>
//=============================================================================
//=============================================================================
const float CAMERA_MIN_DIST = 1.0f;
const float CAMERA_INITIAL_DIST = 4.0f;
const float CAMERA_MAX_DIST = 15.0f;

//=============================================================================
//=============================================================================
URHO3D_DEFINE_APPLICATION_MAIN(CharacterDemo)

//=============================================================================
//=============================================================================
CharacterDemo::CharacterDemo(Context* context) :
    Sample(context),
    firstPerson_(false),
    drawDebug_(false)
{
    // Register factory and attributes for the Character component so it can be created via CreateComponent, and loaded / saved
    Character::RegisterObject(context);
}

CharacterDemo::~CharacterDemo()
{
}

void CharacterDemo::Setup()
{
    engineParameters_["WindowTitle"]  = GetTypeName();
    engineParameters_["LogName"]      = GetSubsystem<FileSystem>()->GetProgramDir() + "skinnedArmor.log";
    engineParameters_["FullScreen"]   = false;
    engineParameters_["Headless"]     = false;
    engineParameters_["WindowWidth"]  = 1280; 
    engineParameters_["WindowHeight"] = 720;
}

void CharacterDemo::Start()
{
    // Execute base class startup
    Sample::Start();

    ChangeDebugHudText();

    // Create static scene content
    CreateScene();

    // Create the controllable character
    CreateCharacter();

    // Create the UI content
    CreateInstructions();

    // Subscribe to necessary events
    SubscribeToEvents();

    // Set the mouse mode to use in the sample
    Sample::InitMouseMode(MM_RELATIVE);
}

void CharacterDemo::ChangeDebugHudText()
{
    // change profiler text
    if (GetSubsystem<DebugHud>())
    {
        Text *dbgText = GetSubsystem<DebugHud>()->GetProfilerText();
        dbgText->SetColor(Color::CYAN);
        dbgText->SetTextEffect(TE_NONE);

        dbgText = GetSubsystem<DebugHud>()->GetStatsText();
        dbgText->SetColor(Color::CYAN);
        dbgText->SetTextEffect(TE_NONE);

        dbgText = GetSubsystem<DebugHud>()->GetMemoryText();
        dbgText->SetColor(Color::CYAN);
        dbgText->SetTextEffect(TE_NONE);

        dbgText = GetSubsystem<DebugHud>()->GetModeText();
        dbgText->SetColor(Color::CYAN);
        dbgText->SetTextEffect(TE_NONE);
    }
}

void CharacterDemo::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);

    // Create camera and define viewport. We will be doing load / save, so it's convenient to create the camera outside the scene,
    // so that it won't be destroyed and recreated, and we don't have to redefine the viewport on load
    cameraNode_ = new Node(context_);
    Camera* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(350.0f);

    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, camera));
    GetSubsystem<Renderer>()->SetViewport(0, viewport);

    // load scene
    XMLFile *xmlLevel = cache->GetResource<XMLFile>("SkinnedArmor/Scene/LevelScene.xml");
    scene_->LoadXML(xmlLevel->GetRoot());

    dummyNode_ = scene_->GetChild("Dummy", true);
}

void CharacterDemo::CreateCharacter()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // spin node
    Node *spawnNode = scene_->GetChild("playerSpawn");
    Node* objectNode = scene_->CreateChild("Player");
    objectNode->SetPosition(spawnNode->GetPosition());

    Node* adjustNode = objectNode->CreateChild("AdjNode");
    adjustNode->SetRotation(Quaternion(180, Vector3(0,1,0)));

    // model
    AnimatedModel* object = adjustNode->CreateComponent<AnimatedModel>();
    SharedPtr<Model> model(cache->GetResource<Model>("SkinnedArmor/Girlbot/Girlbot.mdl")->Clone());
    Model *modelArmor = cache->GetResource<Model>("SkinnedArmor/Maria/Armor.mdl");
    model->SetGeometry(4, 0, modelArmor->GetGeometry(0, 0));
    model->SetGeometry(5, 0, modelArmor->GetGeometry(1, 0));
    model->SetGeometry(6, 0, modelArmor->GetGeometry(2, 0));
    model->SetGeometry(7, 0, modelArmor->GetGeometry(3, 0));
    model->SetGeometry(8, 0, modelArmor->GetGeometry(4, 0));

    // set model
    object->SetModel(model);

    object->SetMaterial(0, cache->GetResource<Material>("SkinnedArmor/Girlbot/Materials/BetaBodyMat1.xml"));
    object->SetMaterial(1, cache->GetResource<Material>("SkinnedArmor/Girlbot/Materials/BetaBodyMat1.xml"));
    object->SetMaterial(2, cache->GetResource<Material>("SkinnedArmor/Girlbot/Materials/BetaBodyMat1.xml"));
    object->SetMaterial(3, cache->GetResource<Material>("SkinnedArmor/Girlbot/Materials/BetaJointsMAT1.xml"));

    object->SetMaterial(4, cache->GetResource<Material>("SkinnedArmor/Maria/Materials/MariaMat1.xml"));
    object->SetMaterial(5, cache->GetResource<Material>("SkinnedArmor/Maria/Materials/MariaMat1.xml"));
    object->SetMaterial(6, cache->GetResource<Material>("SkinnedArmor/Maria/Materials/MariaMat1.xml"));
    object->SetMaterial(7, cache->GetResource<Material>("SkinnedArmor/Maria/Materials/MariaMat1.xml"));
    object->SetMaterial(8, cache->GetResource<Material>("SkinnedArmor/Maria/Materials/MariaMat1.xml"));

    object->SetCastShadows(true);

    // anim ctrl
    adjustNode->CreateComponent<AnimationController>();

    // Set the head bone for manual control
    object->GetSkeleton().GetBone("Head")->animated_ = false;

    // rigidbody
    RigidBody* body = objectNode->CreateComponent<RigidBody>();
    body->SetCollisionLayer(ColLayer_Character);
    body->SetCollisionMask(ColMask_Character);
    body->SetMass(1.0f);
    body->SetAngularFactor(Vector3::ZERO);
    body->SetCollisionEventMode(COLLISION_ALWAYS);

    CollisionShape* shape = objectNode->CreateComponent<CollisionShape>();
    shape->SetCapsule(0.7f, 1.8f, Vector3(0.0f, 0.9f, 0.0f));
    character_ = objectNode->CreateComponent<Character>();

    // back locator
    XMLFile *xmlDat = cache->GetResource<XMLFile>("SkinnedArmor/XMLData/BackLocator.xml");
    Node *loadNode = scene_->InstantiateXML(xmlDat->GetRoot(), Vector3::ZERO, Quaternion::IDENTITY);
    Node *mntNode = adjustNode->GetChild(loadNode->GetName(), true);

    if (mntNode)
    {
        Node *gsLocator = loadNode->GetChild("GreatswordLocator");
        if (gsLocator)
        {
            mntNode->AddChild(gsLocator);
            greatswordNode_ = mntNode->GetChild("Weapon", true);
        }
    }
    scene_->RemoveChild(loadNode);

}

void CharacterDemo::CreateInstructions()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();

    // Construct new Text object, set string to display and font to use
    Text* instructionText = ui->GetRoot()->CreateChild<Text>();
    instructionText->SetText(
        "Q - equip sword, LMB - combo attack, F5 - dbg collision"
    );
    instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 12);
    instructionText->SetColor(Color::CYAN);
    // The text has multiple rows. Center them in relation to each other
    instructionText->SetTextAlignment(HA_CENTER);

    // Position the text relative to the screen center
    instructionText->SetHorizontalAlignment(HA_CENTER);
    //instructionText->SetVerticalAlignment(VA_CENTER);
    instructionText->SetPosition(0, 20);
}

void CharacterDemo::SubscribeToEvents()
{
    // Subscribe to Update event for setting the character controls before physics simulation
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CharacterDemo, HandleUpdate));

    // Subscribe to PostUpdate event for updating the camera position after physics simulation
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(CharacterDemo, HandlePostUpdate));

    // Unsubscribe the SceneUpdate event from base class as the camera node is being controlled in HandlePostUpdate() in this sample
    UnsubscribeFromEvent(E_SCENEUPDATE);

    // weapon dmg
    SubscribeToEvent(E_WEAPONDMG, URHO3D_HANDLER(CharacterDemo, HandleWeaponDmgEvent));
}

void CharacterDemo::HandleWeaponDmgEvent(StringHash eventType, VariantMap& eventData)
{
    using namespace WeaponDmgEvent;

    Node *node = (Node*)eventData[P_NODE].GetVoidPtr();

    DmgRecipient dmgRecipientData;
    dmgRecipientList_.Push(dmgRecipientData);
    DmgRecipient &dmgRecipient = dmgRecipientList_[dmgRecipientList_.Size()-1];

    // for this demo, we only look for staticModel type
    dmgRecipient.node_ = node;
    Material *mat = node->GetComponent<StaticModel>()->GetMaterial();
    dmgRecipient.dmgColor_ = Color::RED;
    dmgRecipient.origColor_ = mat->GetShaderParameter("MatDiffColor").GetColor();
    mat->SetShaderParameter("MatDiffColor", dmgRecipient.dmgColor_);

    dmgRecipient.flashTimer_.Reset();
}

void CharacterDemo::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    // dmg recipient
    for ( unsigned i = 0; i < dmgRecipientList_.Size(); ++i)
    {
        if (dmgRecipientList_[i].flashTimer_.GetMSec(false) > 400)
        {
            Material *mat = dmgRecipientList_[i].node_->GetComponent<StaticModel>()->GetMaterial();

            mat->SetShaderParameter("MatDiffColor", dmgRecipientList_[i].origColor_);
            dmgRecipientList_.Erase(i--);
        }
    }

    Input* input = GetSubsystem<Input>();

    if (character_)
    {
        // Clear previous controls
        character_->controls_.Set(CTRL_FORWARD | CTRL_BACK | CTRL_LEFT | CTRL_RIGHT | CTRL_JUMP, false);

        // Update controls using keys
        UI* ui = GetSubsystem<UI>();
        if (!ui->GetFocusElement())
        {
            character_->controls_.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
            character_->controls_.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
            character_->controls_.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
            character_->controls_.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
            character_->controls_.Set(CTRL_EQUIP, input->GetKeyDown(KEY_Q));

            if (input->GetMouseButtonPress(MOUSEB_LEFT))
                character_->controls_.Set(CTRL_LMB, true);
            if (input->GetMouseButtonPress(MOUSEB_RIGHT))
                character_->controls_.Set(CTRL_RMB, true);

            character_->controls_.Set(CTRL_JUMP, input->GetKeyDown(KEY_SPACE));
            character_->controls_.yaw_ += (float)input->GetMouseMoveX() * YAW_SENSITIVITY;
            character_->controls_.pitch_ += (float)input->GetMouseMoveY() * YAW_SENSITIVITY;
            // Limit pitch
            character_->controls_.pitch_ = Clamp(character_->controls_.pitch_, -80.0f, 80.0f);
            // Set rotation already here so that it's updated every rendering frame instead of every physics frame
            character_->GetNode()->SetRotation(Quaternion(character_->controls_.yaw_, Vector3::UP));

            // Switch between 1st and 3rd person
            if (input->GetKeyPress(KEY_F))
                firstPerson_ = !firstPerson_;
        }
    }

    if (input->GetKeyPress(KEY_F5))
    {
        if (debounceTimer_.GetMSec(false) > 250)
        {
            drawDebug_ = !drawDebug_;
            debounceTimer_.Reset();
        }
    }

}

void CharacterDemo::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    if (!character_)
        return;

    Node* characterNode = character_->GetNode();

    // Get camera lookat dir from character yaw + pitch
    Quaternion rot = characterNode->GetRotation();
    Quaternion dir = rot * Quaternion(character_->controls_.pitch_, Vector3::RIGHT);

    // Turn head to camera pitch, but limit to avoid unnatural animation
    Node* headNode = characterNode->GetChild("Head", true);
    float limitPitch = Clamp(character_->controls_.pitch_, -45.0f, 45.0f);
    Quaternion headDir = rot * Quaternion(limitPitch, Vector3(1.0f, 0.0f, 0.0f));
    // This could be expanded to look at an arbitrary target, now just look at a point in front
    Vector3 headWorldTarget = headNode->GetWorldPosition() + headDir * Vector3(0.0f, 0.0f, -1.0f);
    headNode->LookAt(headWorldTarget, Vector3(0.0f, 1.0f, 0.0f));

    //if (firstPerson_)
    //{
    //    cameraNode_->SetPosition(headNode->GetWorldPosition() + rot * Vector3(0.0f, 0.15f, 0.2f));
    //    cameraNode_->SetRotation(dir);
    //}
    //else
    {
        // Third person camera: position behind the character
        Vector3 aimPoint = characterNode->GetPosition() + rot * Vector3(0.0f, 1.7f, 0.0f);

        // Collide camera ray with static physics objects (layer bitmask 2) to ensure we see the character properly
        Vector3 rayDir = dir * Vector3::BACK;
        float rayDistance = CAMERA_INITIAL_DIST;
        PhysicsRaycastResult result;
        scene_->GetComponent<PhysicsWorld>()->RaycastSingle(result, Ray(aimPoint, rayDir), rayDistance, ColMask_Camera);
        if (result.body_)
            rayDistance = Min(rayDistance, result.distance_);
        rayDistance = Clamp(rayDistance, CAMERA_MIN_DIST, CAMERA_MAX_DIST);

        cameraNode_->SetPosition(aimPoint + rayDir * rayDistance);
        cameraNode_->SetRotation(dir);
    }

    if (drawDebug_)
    {
        DebugRenderer *dbgRenderer = scene_->GetComponent<DebugRenderer>();
        if (dummyNode_ && dummyNode_->GetComponent<RigidBody>())
        {
            dummyNode_->GetComponent<RigidBody>()->DrawDebugGeometry(dbgRenderer, true);
        }
        if (greatswordNode_ && greatswordNode_->GetComponent<RigidBody>())
        {
            greatswordNode_->GetComponent<RigidBody>()->DrawDebugGeometry(dbgRenderer, true);
        }
    }

}

