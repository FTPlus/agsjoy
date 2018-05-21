/*******************************************************
 * Joystick interface -- header file                   *
 *                                                     *
 * Author: Ferry "Wyz" Timmers                         *
 *                                                     *
 * Date: 14:10 4-2-2014                                *
 *                                                     *
 * Description: Implements a joystick object and       *
 *              exposes game controller functionality. *
 *******************************************************/

#ifndef _JOYSTICK_H

#include "API.h"

#ifndef AGSJOYSTICK
#	ifdef WIN_AUTO_VERSION
#		define AGSJOYSTICK AGSJoystickMM
#		include "Joystick.h"
#		undef AGSJOYSTICK
#		define AGSJOYSTICK AGSJoystickDX8
#	else
#		define AGSJOYSTICK AGSJoystick
#	endif
#endif

/// Joystick plugin
namespace AGSJOYSTICK {

//------------------------------------------------------------------------------

void Initialize(); ///< Initializes the interface so it is ready to be used
void Update();     ///< Updates the interface state
void Terminate();  ///< Resets the interface to its initial state

//------------------------------------------------------------------------------

/// Platform dependent interface state representation
struct JoyState;

/// Joystick instance
struct Joystick
{
	// Exposed: <<<DO NOT CHANGE THE ORDER!!!>>>
	long id;
	long button_count;
	long axis_count;
	long x, y, z, u, v, w;
	long pov;
	unsigned long buttons;
	
	// Internal:
	int events;
	JoyState *state;
};

AGS_DEFINE_CLASS(Joystick)

//------------------------------------------------------------------------------

// Global
long JoystickCount();
long JoystickRescan();
const char *JoystickName(long index);

// Static
Joystick *Joystick_Open(long index);
long Joystick_IsOpen(long index);
void Joystick_Click(long button);

// Members
void Joystick_Close(Joystick *);
long Joystick_Valid(Joystick *);
long Joystick_Unplugged(Joystick *);
const char *Joystick_GetName(Joystick *);
long Joystick_GetAxis(Joystick *, long index);
long Joystick_IsButtonDown(Joystick *, long button);
void Joystick_Update(Joystick *);
void Joystick_EnableEvents(Joystick *, long scope);
void Joystick_DisableEvents(Joystick *);

//------------------------------------------------------------------------------

} /* namespace AGSJoystick */

//------------------------------------------------------------------------------

#ifndef JOYSTICK_HEADER
#define JOYSTICK_HEADER \
	"/// Returns the number of gamecontrollers found.\r\n" \
	"import int JoystickCount ();\r\n" \
	"/// Scans for newly installed gamecontrollers. Retuns true when found.\r\n" \
	"import bool JoystickRescan ();\r\n" \
	"/// Returns the name of the specified gamecontroller. (0-15)\r\n" \
	"import String JoystickName (int ID);\r\n" \
	"\r\n" \
	"enum JoystickPOV {\r\n" \
	"	ePOVCenter = 0,\r\n" \
	"	ePOVUp = 1,\r\n" \
	"	ePOVRight = 2,\r\n" \
	"	ePOVUpRight = 3,\r\n" \
	"	ePOVDown = 4,\r\n" \
	"	ePOVDownRight = 6,\r\n" \
	"	ePOVLeft = 8,\r\n" \
	"	ePOVUpLeft = 9,\r\n" \
	"	ePOVDownLeft = 12\r\n" \
	"};\r\n" \
	"\r\n" \
	"#define JOY_RANGE 32768\r\n" \
	"managed struct Joystick {\r\n" \
	"	readonly int ID;\r\n" \
	"	readonly int ButtonCount;\r\n" \
	"	readonly int AxesCount;\r\n" \
	"	readonly int x;\r\n" \
	"	readonly int y;\r\n" \
	"	readonly int z;\r\n" \
	"	readonly int u;\r\n" \
	"	readonly int v;\r\n" \
	"	readonly int w;\r\n" \
	"	readonly JoystickPOV POV;\r\n" \
	"	readonly int buttons; // $AUTOCOMPLETEIGNORE$\r\n" \
	"	\r\n" \
	"/// Opens specified controller. (0-15)\r\n" \
	"	import static Joystick* Open (int ID); // $AUTOCOMPLETESTATICONLY$\r\n" \
	"/// Checks if specified controller has been opened. (0-15)\r\n" \
	"	import static bool IsOpen (int ID); // $AUTOCOMPLETESTATICONLY$\r\n" \
	"/// Simulates a mouseclick at the current cursor position.\r\n" \
	"	import static void Click (MouseButton button);\r\n" \
	"/// Closes controller.\r\n" \
	"	import void Close ();\r\n" \
	"/// Returns whether the controller is valid. (use this when loading save games)\r\n" \
	"	import bool Valid ();\r\n" \
	"/// Returns if the controller is currently unplugged.\r\n" \
	"	import bool Unplugged ();\r\n" \
	"/// Returns the controller name.\r\n" \
	"	import String GetName ();\r\n" \
	"/// Returns axis value by number. (0-5)\r\n" \
	"	import int GetAxis (int axis);\r\n" \
	"/// Returns true when the specified button is currently down. (0-31)\r\n" \
	"	import bool IsButtonDown (int button);\r\n" \
	"/// Forces an update of the controller axis, button and pov state.\r\n" \
	"	import void Update ();\r\n" \
	"/// Enables events on axis move, button press and pov. (0 = global (default), 1 = room)\r\n" \
	"	import void EnableEvents (int scope = 0);\r\n" \
	"/// Disable events. (disabled by default)\r\n" \
	"	import void DisableEvents ();\r\n" \
	"};\r\n";
#endif

#ifndef JOYSTICK_ENTRY
#define JOYSTICK_ENTRY                           \
	AGS_FUNCTION(JoystickCount)                  \
	AGS_FUNCTION(JoystickRescan)                 \
	AGS_FUNCTION(JoystickName)                   \
	AGS_CLASS   (Joystick)                       \
	AGS_METHOD  (Joystick, Open, 1)              \
	AGS_METHOD  (Joystick, IsOpen, 1)            \
	AGS_METHOD  (Joystick, Click, 1)             \
	AGS_METHOD  (Joystick, Close, 0)             \
	AGS_METHOD  (Joystick, Valid, 0)             \
	AGS_METHOD  (Joystick, Unplugged, 0)         \
	AGS_METHOD  (Joystick, GetName, 0)           \
	AGS_METHOD  (Joystick, GetAxis, 1)           \
	AGS_METHOD  (Joystick, IsButtonDown, 1)      \
	AGS_METHOD  (Joystick, Update, 0)            \
	AGS_METHOD  (Joystick, EnableEvents, 1)      \
	AGS_METHOD  (Joystick, DisableEvents, 0)
#endif

//------------------------------------------------------------------------------

#define _JOYSTICK_H
#endif /* _JOYSTICK_H */

//..............................................................................
