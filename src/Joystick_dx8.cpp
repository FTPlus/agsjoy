/***********************************************************
 * Joystick interface -- platform specific implementation  *
 *                                                         *
 * Author: Ferry "Wyz" Timmers                             *
 *                                                         *
 * Date:                                                   *
 *                                                         *
 * Description: Joystick interface DirectX 8 version.      *
 ***********************************************************/

#include "Joystick.h"

#ifdef WIN_AUTO_VERSION
namespace AGSJoystickDX8 {
#else
namespace AGSJoystick {
#endif

using namespace AGSJoyAPI;

//------------------------------------------------------------------------------

//==============================================================================

void Initialize()
{
}

//------------------------------------------------------------------------------

void Update()
{
}

//------------------------------------------------------------------------------

void Terminate()
{
}

//==============================================================================

int AGSJoystick::Dispose(const char *address, bool force)
{
	return 1;
}

//------------------------------------------------------------------------------

int AGSJoystick::Serialize(const char *address, char *buffer, int bufsize)
{
	return 0;
}

//------------------------------------------------------------------------------

void AGSJoystick::Unserialize(int key, const char *serializedData, int dataSize)
{
}

//==============================================================================

long JoystickCount()
{
	return 0;
}

//------------------------------------------------------------------------------

long JoystickRescan()
{
	return 0;
}

//------------------------------------------------------------------------------

const char *JoystickName(long index)
{
	return "";
}

//==============================================================================

Joystick *Joystick_Open(long index)
{
	return NULL;
}

//------------------------------------------------------------------------------

long Joystick_IsOpen(long index)
{
	return 0;
}

//------------------------------------------------------------------------------

void Joystick_Click(long button)
{
}

//==============================================================================

void Joystick_Close(Joystick *)
{
}

//------------------------------------------------------------------------------

long Joystick_Valid(Joystick *)
{
	return 0;
}

//------------------------------------------------------------------------------

long Joystick_Unplugged(Joystick *)
{
	return 1;
}

//------------------------------------------------------------------------------

const char *Joystick_GetName(Joystick *)
{
	return "";
}

//------------------------------------------------------------------------------

long Joystick_GetAxis(Joystick *, long index)
{
	return 0;
}

//------------------------------------------------------------------------------

long Joystick_IsButtonDown(Joystick *, long button)
{
	return 0;
}

//------------------------------------------------------------------------------

void Joystick_Update(Joystick *)
{
}

//------------------------------------------------------------------------------

void Joystick_EnableEvents(Joystick *, long scope)
{
}

//------------------------------------------------------------------------------

void Joystick_DisableEvents(Joystick *)
{
}

//------------------------------------------------------------------------------

} /* namespace AGSJoystick(DX8) */

//..............................................................................
