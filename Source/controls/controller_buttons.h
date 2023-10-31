#pragma once
// Unifies joystick, gamepad, and keyboard controller APIs.

#include <cstdint>
#include <functional>
#include <string_view>

namespace devilution {

// NOTE: A, B, X, Y refer to physical positions on an XBox 360 controller.
// A<->B and X<->Y are reversed on a Nintendo controller.
enum ControllerButton : uint8_t {
	ControllerButton_NONE,
	ControllerButton_IGNORE,
	ControllerButton_AXIS_TRIGGERLEFT,  // ZL (aka L2)
	ControllerButton_AXIS_TRIGGERRIGHT, // ZR (aka R2)
	ControllerButton_BUTTON_A,          // Bottom button
	ControllerButton_BUTTON_B,          // Right button
	ControllerButton_BUTTON_X,          // Left button
	ControllerButton_BUTTON_Y,          // TOP button
	ControllerButton_BUTTON_LEFTSTICK,
	ControllerButton_BUTTON_RIGHTSTICK,
	ControllerButton_BUTTON_LEFTSHOULDER,
	ControllerButton_BUTTON_RIGHTSHOULDER,
	ControllerButton_BUTTON_START,
	ControllerButton_BUTTON_BACK,
	ControllerButton_BUTTON_DPAD_UP,
	ControllerButton_BUTTON_DPAD_DOWN,
	ControllerButton_BUTTON_DPAD_LEFT,
	ControllerButton_BUTTON_DPAD_RIGHT,
	FIRST = ControllerButton_NONE,
	LAST = ControllerButton_BUTTON_DPAD_RIGHT
};

struct ControllerButtonCombo {
	constexpr ControllerButtonCombo()
	    : modifier(ControllerButton_NONE)
	    , button(ControllerButton_NONE)
	{
	}

	constexpr ControllerButtonCombo(ControllerButton button)
	    : modifier(ControllerButton_NONE)
	    , button(button)
	{
	}

	constexpr ControllerButtonCombo(ControllerButton modifier, ControllerButton button)
	    : modifier(modifier)
	    , button(button)
	{
	}

	ControllerButton modifier;
	ControllerButton button;
};

inline bool IsDPadButton(ControllerButton button)
{
	return button == ControllerButton_BUTTON_DPAD_UP
	    || button == ControllerButton_BUTTON_DPAD_DOWN
	    || button == ControllerButton_BUTTON_DPAD_LEFT
	    || button == ControllerButton_BUTTON_DPAD_RIGHT;
}

namespace controller_button_icon {
extern const std::string_view Playstation_Triangle;
extern const std::string_view Playstation_Square;
extern const std::string_view Playstation_X;
extern const std::string_view Playstation_Circle;
extern const std::string_view Playstation_Options;
extern const std::string_view Playstation_Share;
extern const std::string_view Playstation_L2;
extern const std::string_view Playstation_R2;
extern const std::string_view Playstation_L1;
extern const std::string_view Playstation_R1;
extern const std::string_view Playstation_DPad_Up;
extern const std::string_view Playstation_DPad_Right;
extern const std::string_view Playstation_DPad_Down;
extern const std::string_view Playstation_DPad_Left;
extern const std::string_view Playstation_LStick_NW;
extern const std::string_view Playstation_LStick_W;
extern const std::string_view Playstation_LStick_SW;
extern const std::string_view Playstation_LStick_N;
extern const std::string_view Playstation_LStick;
extern const std::string_view Playstation_LStick_S;
extern const std::string_view Playstation_LStick_NE;
extern const std::string_view Playstation_LStick_E;
extern const std::string_view Playstation_LStick_SE;
extern const std::string_view Playstation_L3;
extern const std::string_view Playstation_RStick_NW;
extern const std::string_view Playstation_RStick_W;
extern const std::string_view Playstation_RStick_SW;
extern const std::string_view Playstation_RStick_N;
extern const std::string_view Playstation_RStick;
extern const std::string_view Playstation_RStick_S;
extern const std::string_view Playstation_RStick_NE;
extern const std::string_view Playstation_RStick_E;
extern const std::string_view Playstation_RStick_SE;
extern const std::string_view Playstation_R3;
extern const std::string_view Playstation_Touchpad;
extern const std::string_view Nintendo_X;
extern const std::string_view Nintendo_Y;
extern const std::string_view Nintendo_B;
extern const std::string_view Nintendo_A;
extern const std::string_view Nintendo_Plus;
extern const std::string_view Nintendo_Minus;
extern const std::string_view Nintendo_ZL;
extern const std::string_view Nintendo_ZR;
extern const std::string_view Nintendo_L;
extern const std::string_view Nintendo_R;
extern const std::string_view Nintendo_DPad_Up;
extern const std::string_view Nintendo_DPad_Right;
extern const std::string_view Nintendo_DPad_Down;
extern const std::string_view Nintendo_DPad_Left;
extern const std::string_view Nintendo_LStick_NW;
extern const std::string_view Nintendo_LStick_W;
extern const std::string_view Nintendo_LStick_SW;
extern const std::string_view Nintendo_LStick_N;
extern const std::string_view Nintendo_LStick;
extern const std::string_view Nintendo_LStick_S;
extern const std::string_view Nintendo_LStick_NE;
extern const std::string_view Nintendo_LStick_E;
extern const std::string_view Nintendo_LStick_SE;
extern const std::string_view Nintendo_LStick_Click;
extern const std::string_view Nintendo_RStick_NW;
extern const std::string_view Nintendo_RStick_W;
extern const std::string_view Nintendo_RStick_SW;
extern const std::string_view Nintendo_RStick_N;
extern const std::string_view Nintendo_RStick;
extern const std::string_view Nintendo_RStick_S;
extern const std::string_view Nintendo_RStick_NE;
extern const std::string_view Nintendo_RStick_E;
extern const std::string_view Nintendo_RStick_SE;
extern const std::string_view Nintendo_RStick_Click;
extern const std::string_view Nintendo_Home;
extern const std::string_view Nintendo_Screenshot;
extern const std::string_view Nintendo_SL;
extern const std::string_view Nintendo_SR;
extern const std::string_view Xbox_Y;
extern const std::string_view Xbox_X;
extern const std::string_view Xbox_A;
extern const std::string_view Xbox_B;
extern const std::string_view Xbox_Menu;
extern const std::string_view Xbox_View;
extern const std::string_view Xbox_LT;
extern const std::string_view Xbox_RT;
extern const std::string_view Xbox_LB;
extern const std::string_view Xbox_RB;
extern const std::string_view Xbox_DPad_Up;
extern const std::string_view Xbox_DPad_Right;
extern const std::string_view Xbox_DPad_Down;
extern const std::string_view Xbox_DPad_Left;
extern const std::string_view Xbox_LStick_NW;
extern const std::string_view Xbox_LStick_W;
extern const std::string_view Xbox_LStick_SW;
extern const std::string_view Xbox_LStick_N;
extern const std::string_view Xbox_LStick;
extern const std::string_view Xbox_LStick_NE;
extern const std::string_view Xbox_LStick_E;
extern const std::string_view Xbox_LStick_SE;
extern const std::string_view Xbox_LStick_Click;
extern const std::string_view Xbox_RStick_NW;
extern const std::string_view Xbox_RStick_W;
extern const std::string_view Xbox_RStick_SW;
extern const std::string_view Xbox_RStick_N;
extern const std::string_view Xbox_RStick;
extern const std::string_view Xbox_RStick_S;
extern const std::string_view Xbox_RStick_NE;
extern const std::string_view Xbox_RStick_E;
extern const std::string_view Xbox_RStick_SE;
extern const std::string_view Xbox_RStick_Click;
extern const std::string_view Xbox_Xbox;
} // namespace controller_button_icon

std::string_view ToPlayStationIcon(ControllerButton button);
std::string_view ToNintendoIcon(ControllerButton button);
std::string_view ToXboxIcon(ControllerButton button);
std::string_view ToGenericButtonText(ControllerButton button);
std::string_view ToString(ControllerButton button);

} // namespace devilution
