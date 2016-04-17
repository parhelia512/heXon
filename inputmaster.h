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

#ifndef INPUTMASTER_H
#define INPUTMASTER_H

#include <Urho3D/Urho3D.h>

#include "mastercontrol.h"

// Define events to be bound
//URHO3D_EVENT(MENU_BUTTON_DOWN, MenuButtonDown) // MouseDown, KeyDown, ControllerButtonDown, MouseWheel
//{
//    URHO3D_PARAM(P_ACTION, Action);	// int
//}
//URHO3D_EVENT(MENU_BUTTON_UP, MenuButtonUp) // MouseUp, KeyUp, ControllerButtonUp, ControllerAxisMove, MouseWheel
//{
//    URHO3D_PARAM(P_ACTION, Action);	// int
//}
//URHO3D_EVENT(BUTTON_DOWN, ButtonDown) // MouseDown, KeyDown, ControllerButtonDown, MouseWheel
//{
//    URHO3D_PARAM(P_ACTION, Action);	// int
//}
//URHO3D_EVENT(BUTTON_UP, ButtonUp) // MouseUp, KeyUp, ControllerButtonUp, MouseWheel
//{
//    URHO3D_PARAM(P_ACTION, Action);	// int
//}
//URHO3D_EVENT(MOUSE_AXIS_MOVE, MouseAxisMove) // MouseMove
//{
//    URHO3D_PARAM(P_AXIS, Axis);	// int
//    URHO3D_PARAM(P_DELTA, Delta);	// float
//}

class InputMaster : public Object
{
    URHO3D_OBJECT(InputMaster, Object);
public:
    InputMaster(MasterControl* masterControl);

    void Init();

    void SubscribeToEvents();

    void HandleMouseButtonDown(StringHash eventType, VariantMap &eventData);
    void HandleKeyDown(StringHash eventType, VariantMap &eventData);
    void HandleMouseUp(StringHash eventType, VariantMap &eventData);
    void HandleJoystickButtonDown(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleJoystickButtonUp(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleJoystickAxisMove(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleMouseMove(StringHash eventType, VariantMap &eventData);
    void HandleMouseWheel(StringHash eventType, VariantMap &eventData);
    void HandleKeyUp(StringHash eventType, VariantMap &eventData);
    void HandleMouseButtonUp(StringHash eventType, VariantMap &eventData);
    void Screenshot();

private:
    MasterControl* masterControl_;

    Input* input_;

    void PauseButtonPressed();
    void EjectButtonPressed();
};

#endif // INPUTMASTER_H
