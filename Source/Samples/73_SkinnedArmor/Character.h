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

#pragma once

#include <Urho3D/Input/Controls.h>
#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;
namespace Urho3D
{
class AnimationController;
}

//=============================================================================
//=============================================================================
const int CTRL_FORWARD = (1 << 0);
const int CTRL_BACK    = (1 << 1);
const int CTRL_LEFT    = (1 << 2);
const int CTRL_RIGHT   = (1 << 3);
const int CTRL_JUMP    = (1 << 4);
const int CTRL_EQUIP   = (1 << 5);

const int CTRL_LMB     = (1 << 6);
const int CTRL_RMB     = (1 << 7);

const float MOVE_FORCE = 0.8f;
const float INAIR_MOVE_FORCE = 0.02f;
const float BRAKE_FORCE = 0.2f;
const float JUMP_FORCE = 7.0f;
const float YAW_SENSITIVITY = 0.1f;
const float INAIR_THRESHOLD_TIME = 0.1f;

//=============================================================================
//=============================================================================
URHO3D_EVENT(E_WEAPONDMG, WeaponDmgEvent)
{
	URHO3D_PARAM(P_NODE, Node);
	URHO3D_PARAM(P_POS, Pos);
	URHO3D_PARAM(P_DIR, Dir);
}

//=============================================================================
// simple single key input queue
//=============================================================================
class QueInput
{
public:
    QueInput() : input_(M_MAX_UNSIGNED), holdTime_(1200) {}

    void SetInput(unsigned input)
    {
        input_ = input;
        queTimer_.Reset();
    }

    unsigned GetInput() const
    {
        return input_;
    }

    void Update()
    {
        if (queTimer_.GetMSec(false) >= holdTime_)
        {
            input_ = M_MAX_UNSIGNED;
        }
    }

    bool Empty() const
    {
        return (input_ == M_MAX_UNSIGNED);
    }

    void Reset()
    {
        input_ = M_MAX_UNSIGNED;
    }

protected:
    unsigned input_;

    unsigned holdTime_;
    Timer queTimer_;
};

//=============================================================================
//=============================================================================
class Character : public LogicComponent
{
    URHO3D_OBJECT(Character, LogicComponent);

public:
    /// Construct.
    Character(Context* context);
    
    /// Register object factory and attributes.
    static void RegisterObject(Context* context);
    
    virtual void DelayedStart();
    virtual void Start();
    /// Handle physics world update. Called by LogicComponent base class.
    virtual void FixedUpdate(float timeStep);
    
    /// Movement controls. Assigned by the main program each frame.
    Controls controls_;
    
private:
    void ProcessWeaponAction(bool equip, unsigned lMouseB);
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData);
    void HandleWeaponCollision(StringHash eventType, VariantMap& eventData);
    void HandleAnimationTrigger(StringHash eventType, VariantMap& eventData);
    void SendWeaponDmgEvent(Node *node);

    /// Grounded flag for movement.
    bool onGround_;
    /// Jump flag.
    bool okToJump_;
    /// In air timer. Due to possible physics inaccuracy, character can be off ground for max. 1/10 second and still be allowed to move.
    float inAirTimer_;
    bool jumpStarted_;

    // anim ctrl
    WeakPtr<AnimationController> animCtrl_;

    WeakPtr<Node> backLocatorNode_;
    WeakPtr<Node> rightHandLocatorNode_;
    WeakPtr<Node> weaponNode_;

    // weapon state
    unsigned weaponActionState_;
    String weaponActionAnim_;

    unsigned comboAnimsIdx_;
    Vector<String> weaponComboAnim_;
    QueInput queInput_;

    // weapon damage
    unsigned      weaponDmgState_;
    Vector<Node*> weaponDmgRecipientList_;


private:
    enum WeaponState    { Weapon_Invalid, Weapon_Unequipped, Weapon_Equipping, Weapon_Equipped, Weapon_UnEquipping, Weapon_AttackAnim };
    enum AnimLayerType  { NormalLayer, WeaponLayer };
    enum WeaponDmgState { WeaponDmg_OFF, WeaponDmg_ON };
};
