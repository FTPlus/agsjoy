/***************************************************************
 * Joystick interface -- See header file for more information. *
 ***************************************************************/

#include <stdlib.h>

#include "API.h"

#if defined(SDL_VERSION)
#	include "Joystick_sdl.cpp"
#elif defined(WINMM_VERSION)
#	include "Joystick_mm.cpp"
#elif defined(DX8_VERSION)
#	include "Joystick_dx8.cpp"
#elif defined(WIN_AUTO_VERSION)
// Set up so that it falls back to winmm when dx8 fails
#	include "Joystick_mm.cpp"
#	include "Joystick_dx8.cpp"
#elif defined(LINUX_VERSION)
#	include "Joystick_linux.cpp"
// Currently unsupported:
//#elif defined(MAC_VERSION)
//#	include "Joystick_osx.cpp"
//#elif defined(IOS_VERSION)
//#	include "Joystick_ios.cpp"
#else

// defaults to stub for compatibility

#warning Joystick plugin does not support this platform curently, a stub will be used.

#include "Joystick.h"

namespace AGSJOYSTICK {

//------------------------------------------------------------------------------

// Interface
void Initialize() {}
void Update() {}
void Terminate() {}

// Manager
int AGSJoystick::Dispose(const char *address, bool force) { return 1; }
int AGSJoystick::Serialize(const char *address, char *buffer, int bufsize) { return 0; }
void AGSJoystick::Unserialize(int key, const char *serializedData, int dataSize) {}

// Global
long JoystickCount() { return 0; }
long JoystickRescan() { return 0; }
const char *JoystickName(long index) { return ""; }

// Static
Joystick *Joystick_Open(long index) { return NULL; }
long Joystick_IsOpen(long index) { return 0; }
void Joystick_Click(long button) {}

// Members
void Joystick_Close(Joystick *) {}
long Joystick_Valid(Joystick *) { return 0; }
long Joystick_Unplugged(Joystick *) { return 1; }
const char *Joystick_GetName(Joystick *) { return ""; }
long Joystick_GetAxis(Joystick *, long index) { return 0; }
long Joystick_IsButtonDown(Joystick *, long button) { return 0; }
void Joystick_Update(Joystick *) {}
void Joystick_EnableEvents(Joystick *, long scope) {}
void Joystick_DisableEvents(Joystick *) {}

//------------------------------------------------------------------------------

} /* namespace AGSJoystick */

#endif // default platform

//..............................................................................
