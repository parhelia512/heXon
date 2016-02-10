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

#include "inputmaster.h"

InputMaster::InputMaster(Context* context, MasterControl* masterControl) : Object(context)
{
    masterControl_ = masterControl;
    input_ = GetSubsystem<Input>();
    Init();
}

InputMaster::~InputMaster()
{
    _controllerActions.Clear();
    _controllerAxis.Clear();
    _controllerAxisActions.Clear();
}

void InputMaster::Init()
{
    BindControllerAction(Urho3D::CONTROLLER_BUTTON_LEFTSTICK, ACTION_CROUCH);
    BindControllerAction(Urho3D::CONTROLLER_BUTTON_A, ACTION_JUMP);
    BindControllerAction(Urho3D::CONTROLLER_BUTTON_RIGHTSHOULDER, ACTION_SWITCH_ITEM);
    BindControllerAction(Urho3D::CONTROLLER_BUTTON_START, ACTION_PAUSE);

    BindControllerAxis(Urho3D::CONTROLLER_AXIS_RIGHTX, AXIS_LOOK_X);
    BindControllerAxis(Urho3D::CONTROLLER_AXIS_RIGHTY, AXIS_LOOK_Y);
    SetJoystickSensitivity(Urho3D::CONTROLLER_AXIS_RIGHTX, 10.0f);
    SetJoystickSensitivity(Urho3D::CONTROLLER_AXIS_RIGHTY, 10.0f);

    //BindControllerAxisAction(Urho3D::CONTROLLER_AXIS_LEFTY, ACTION_MOVE_FORWARD, -1, 0.5f);
    BindControllerAxisAction(Urho3D::CONTROLLER_AXIS_LEFTY, ACTION_MOVE_FORWARD, ACTION_MOVE_BACKWARD, 0.5f);
    BindControllerAxisAction(Urho3D::CONTROLLER_AXIS_LEFTX, ACTION_STRAFE_LEFT, ACTION_STRAFE_RIGHT, 0.5f);

    BindControllerAxisAction(Urho3D::CONTROLLER_AXIS_TRIGGERLEFT, -1, ACTION_PRIMARY, 0.5f);
    BindControllerAxisAction(Urho3D::CONTROLLER_AXIS_TRIGGERRIGHT, -1, ACTION_SECONDARY, 0.5f);

    SubscribeToEvents();
}

void InputMaster::SetMouseSensitivity(const float sensitivity)
{
    _mouseSensitivity = sensitivity;
}

void InputMaster::SetJoystickSensitivity(const int axis, const float sensitivity)
{
    _joystickSensitivity[axis] = sensitivity;
}

void InputMaster::BindControllerAction(const int button, const int action)
{
    _controllerActions[button] = action;
}

void InputMaster::BindControllerAxis(const int axis, const int axisAction, const float threshold)
{
    _controllerAxis[axis] = axisAction;
}

void InputMaster::BindControllerAxisAction(const int axis, const int negative_action, const int positive_action, const float threshold)
{
    ControllerAxisAction definition;
    definition.negative_action = negative_action;
    definition.positive_action = positive_action;
    definition.threshold = threshold;
    definition.overThreshold = false;

    _controllerAxisActions[axis] = definition;
}

void InputMaster::SubscribeToEvents()
{
    SubscribeToEvent(Urho3D::E_MOUSEBUTTONDOWN, URHO3D_HANDLER(InputMaster, HandleMouseButtonDown));
    SubscribeToEvent(Urho3D::E_MOUSEBUTTONUP, URHO3D_HANDLER(InputMaster, HandleMouseButtonUp));
    SubscribeToEvent(Urho3D::E_MOUSEMOVE, URHO3D_HANDLER(InputMaster, HandleMouseMove));
    SubscribeToEvent(Urho3D::E_MOUSEWHEEL, URHO3D_HANDLER(InputMaster, HandleMouseWheel));
    SubscribeToEvent(Urho3D::E_KEYDOWN, URHO3D_HANDLER(InputMaster, HandleKeyDown));
    SubscribeToEvent(Urho3D::E_KEYUP, URHO3D_HANDLER(InputMaster, HandleKeyUp));
    SubscribeToEvent(Urho3D::E_JOYSTICKBUTTONDOWN, URHO3D_HANDLER(InputMaster, HandleJoystickButtonDown));
    SubscribeToEvent(Urho3D::E_JOYSTICKBUTTONUP, URHO3D_HANDLER(InputMaster, HandleJoystickButtonUp));
    SubscribeToEvent(Urho3D::E_JOYSTICKAXISMOVE, URHO3D_HANDLER(InputMaster, HandleJoystickAxisMove));
}

void InputMaster::HandleMouseButtonUp(StringHash eventType, VariantMap &eventData)
{
}

void InputMaster::HandleMouseButtonDown(StringHash eventType, VariantMap &eventData)
{
    using namespace MouseButtonDown;
    int button = eventData[P_BUTTON].GetInt();
    if (button == MOUSEB_LEFT){
        //Set tile type
    }
    else if (button == MOUSEB_RIGHT){
        //Clear tile
    }
}

void InputMaster::HandleMouseMove(StringHash eventType, VariantMap &eventData)
{
}

void InputMaster::HandleMouseWheel(StringHash eventType, VariantMap &eventData)
{
}

void InputMaster::HandleKeyUp(StringHash eventType, VariantMap &eventData)
{
}

void InputMaster::HandleMouseUp(StringHash eventType, VariantMap &eventData)
{
    using namespace MouseButtonUp;
    int button = eventData[P_BUTTON].GetInt();
}

void InputMaster::HandleKeyDown(StringHash eventType, VariantMap &eventData)
{
    //Get the triggering key
    int key = eventData[KeyDown::P_KEY].GetInt();

    switch (key){
    //Exit when ESC is pressed
    case KEY_ESC: EjectButtonPressed();
        break;
    //Take screenshot when 9 is pressed
    case KEY_9:
    {
        Graphics* graphics = GetSubsystem<Graphics>();
        Image screenshot(context_);
        graphics->TakeScreenShot(screenshot);
        //Here we save in the Data folder with date and time appended
        String fileName = GetSubsystem<FileSystem>()->GetProgramDir() + "Screenshots/Screenshot_" +
                Time::GetTimeStamp().Replaced(':', '_').Replaced('.', '_').Replaced(' ', '_')+".png";
        //Log::Write(1, fileName);
        screenshot.SavePNG(fileName);
    } break;
    //Pause/Unpause game on P or joystick Start
    case KEY_P:
    {
        PauseButtonPressed();
    } break;
    //Enter edit mode on E
    //Toggle music on M
    case KEY_M: masterControl_->musicSource_->SetGain(masterControl_->musicSource_->GetGain()==0.0f ? 0.32f : 0.0f);
        break;
    }
}

void InputMaster::HandleJoystickButtonDown(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    unsigned joystick = eventData[JoystickButtonDown::P_JOYSTICKID].GetInt();		//int
    int button = eventData[JoystickButtonDown::P_BUTTON].GetInt();		//int
    // Process game event
    switch (button){
    case JB_START: PauseButtonPressed();
        break;
    case JB_L2: case JB_R2: if (input_->GetJoystickByIndex(joystick)->GetButtonDown(JB_L2) &&
                                input_->GetJoystickByIndex(joystick)->GetButtonDown(JB_R2)) EjectButtonPressed();
        break;
    default: break;
    }
}
void InputMaster::PauseButtonPressed()
{
    switch (masterControl_->GetGameState()){
    case GS_INTRO: break;
    case GS_LOBBY: /*masterControl_->SetGameState(GS_PLAY);*/ break;
    case GS_PLAY: masterControl_->SetPaused(!masterControl_->GetPaused()); break;
    case GS_DEAD: masterControl_->SetGameState(GS_LOBBY); break;
    case GS_EDIT: break;
        default: break;
    }
}
void InputMaster::EjectButtonPressed()
{
    if (masterControl_->GetGameState()==GS_PLAY && !masterControl_->GetPaused()) {
        masterControl_->Eject();
    } else if (masterControl_->GetGameState()==GS_LOBBY) masterControl_->Exit();
}

void InputMaster::HandleJoystickButtonUp(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    int joyID = eventData[JoystickButtonUp::P_JOYSTICKID].GetInt();		//int
    int button = eventData[JoystickButtonUp::P_BUTTON].GetInt();		//int

    // Process game event
    if(_controllerActions.Contains(button))
    {
        //SendButtonUpEvent(_controllerActions[button]);
    }
}

void InputMaster::HandleJoystickAxisMove(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    eventData[JoystickAxisMove::P_JOYSTICKID];		//int
    int axis = eventData[JoystickAxisMove::P_AXIS].GetInt();			//int
    float pos = eventData[JoystickAxisMove::P_POSITION].GetFloat();		//float

        // Process game event
    if(_controllerAxisActions.Contains(axis))
    {
        int action = -1;
        float newPos = pos;
        if(pos > 0)
        {
            action = _controllerAxisActions[axis].positive_action;
        }
        else if(pos < 0)
        {
            newPos = -pos; // Put into [0,1]
            action = _controllerAxisActions[axis].negative_action;
        }

        // No action specified
        if(action == -1)
            return;

        if(newPos >= _controllerAxisActions[axis].threshold && !_controllerAxisActions[axis].overThreshold)
        {
            _controllerAxisActions[axis].overThreshold = true;
           // SendButtonDownEvent(action);
        }
        else if(newPos < _controllerAxisActions[axis].threshold && _controllerAxisActions[axis].overThreshold)
        {
            _controllerAxisActions[axis].overThreshold = false;
           // SendButtonUpEvent(action);
        }
    }

    if(_controllerAxis.Contains(axis)) {}
        //SendJoystickAxisMoveEvent(_controllerAxis[axis], pos * _mouseSensitivity, 0.25f);

}
