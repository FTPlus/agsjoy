/***********************************************************
 * Joystick interface -- platform specific implementation  *
 *                                                         *
 * Author: Ferry Timmers                                   *
 *                                                         *
 * Date: 19-08-10 16:55                                    *
 * Refactored: 4-2-2014 17:31                              *
 *                                                         *
 * Description: Joystick interface Windows Multi Media     *
 *              version (the native API).                  *
 ***********************************************************/

#include "Joystick.h"

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <mmsystem.h>
#include <regstr.h>
#include <stdio.h>

#include <vector>
#include <set>

#include "version.h"

#ifdef WIN_AUTO_VERSION
namespace AGSJoystickMM {
#else
namespace AGSJoystick {
#endif

using namespace AGSJoyAPI;

//------------------------------------------------------------------------------

// MinGW support
#if defined(__GNUC__) && !defined(sprintf_s)
#define sprintf_s snprintf
#endif

#ifndef DEBUG
#define Dprintf(...) ((void) 0)
#else
#include <stdarg.h>

void Dprintf(const char *fmt, ...)
{
	char buffer[512];
	
	va_list args;
	va_start(args, fmt);
	
	vsprintf(buffer, fmt, args);
	engine->PrintDebugConsole(buffer);
	
	va_end(args);
}
#endif

//==============================================================================

struct JoyState;
struct Joystick;

int count = 0;                // Number of joysticks found
std::vector<int> map;         // Maps joystick ID to device ID
std::vector<long> hash;       // Maps joystick ID to a unique device hash
std::set<Joystick *> joyset;  // Keep opened joysticks
Joystick dummy;               // Fake joystick for fallback behaviour

// Invariant I: map.size() == count == hash.size()
// Invariant II: joy.id != INVALID_JOY <=> joy.state != NULL
// Invariant III: joy.id != INVALID_JOY => map[joy.id] exists

// Private methods
inline Joystick *Joystick_find(long index); // Find an open joystick instance (or NULL)
Joystick *Joystick_create(long index); // Create a new joystick instance
long Joystick_status(Joystick *);      // Device status: is it plugged in? etc.
void Joystick_update(Joystick *);      // Update axes, button and pov state
void Joystick_process(Joystick *);     // Process events (when enabled)
const char *Joystick_getname(long index);
long Joystick_hash(int id);

//------------------------------------------------------------------------------

struct JoyState
{
	long  x,  y,  z,  u,  v,  w;  // Used to store the last axis states
	long  pov;                    // Used to store the last pov state
	unsigned long  buttons;       // Used to store the last button states
	float fx, fy, fz, fu, fv, fw; // Used for callibration
	int   ox, oy, oz, ou, ov, ow; // idem
	
	JoyState (JOYCAPS &caps) : buttons(0)
	{		
		const long scale = 65535;
		const long min = -32768;
		
		fx = (float) scale / (caps.wXmax - caps.wXmin); ox = min - caps.wXmin;
		fy = (float) scale / (caps.wYmax - caps.wYmin); oy = min - caps.wYmin;
		fz = (float) scale / (caps.wZmax - caps.wZmin); oz = min - caps.wZmin;
		fu = (float) scale / (caps.wRmax - caps.wRmin); ou = min - caps.wRmin;
		fv = (float) scale / (caps.wUmax - caps.wUmin); ov = min - caps.wUmin;
		fw = (float) scale / (caps.wVmax - caps.wVmin); ow = min - caps.wVmin;
	}
	
	void update(Joystick *joy)
	{
		x = joy->x; y = joy->y; z = joy->z;
		u = joy->u; v = joy->v; w = joy->w;
		pov = joy->pov;
		buttons = joy->buttons;
	}
};

//==============================================================================

void Initialize()
{
	JOYCAPS joy;
	
	// Detect devices
	for (int i = 0, num = joyGetNumDevs(); i < num; ++i)
	{
		if (!joyGetDevCaps(i, &joy, sizeof (joy)))
		{
			hash.push_back(Joystick_hash(i));
			++count;
			map.push_back(i);
		}
	}
	
	// Set up fake joystick instance
	memset(&dummy, 0, sizeof (Joystick));
	dummy.id = INVALID_JOY;
}

//------------------------------------------------------------------------------

void Update()
{
	std::set<Joystick *>::iterator it;
	for (it = joyset.begin(); it != joyset.end(); ++it)
	{
		Joystick *joy = *it;
		if (joy->id == INVALID_JOY)
			continue;
		
		Joystick_update(joy);
		Joystick_process(joy);
	}
}

//------------------------------------------------------------------------------

void Terminate()
{
	joyset.clear();
	map.clear();
	count = 0;
}

//==============================================================================

int AGSJoystick::Dispose(const char *address, bool force)
{
	Joystick *joy = (Joystick *)address;
	
	// Never delete the fake joystick instance
	if (joy == &dummy)
		return 1;

	joyset.erase(joy);
	
	Dprintf("[Joystick] Deleted: #%d %p\n", joy->id, joy);
	if (joy->state)
			delete joy->state;
	delete joy;
	
	return 1;
}

//------------------------------------------------------------------------------

#pragma pack(push, 1)
struct AGSJoystickSerial
{
	long hash;
	long events;
};
#pragma pack(pop)

//------------------------------------------------------------------------------

int AGSJoystick::Serialize(const char *address, char *buffer, int bufsize)
{
	Joystick *joy = (Joystick *)address;
	
	if (joy->id == INVALID_JOY || sizeof (AGSJoystickSerial) > bufsize)
		return 0;
	
	AGSJoystickSerial serial = { hash[joy->id], joy->events };
	memcpy(buffer, &serial, sizeof (AGSJoystickSerial));
	
	return sizeof (AGSJoystickSerial);
}

//------------------------------------------------------------------------------

void AGSJoystick::Unserialize(int key, const char *serializedData, int dataSize)
{
	if (dataSize != sizeof (AGSJoystickSerial))
	{
		// Savefile incompatible, damaged or a fake joy
		AGS_RESTORE(Joystick, &dummy, key);
		return;
	}

	AGSJoystickSerial serial;
	memcpy(&serial, serializedData, sizeof (AGSJoystickSerial));
	
	// Find previous used device
	for (int i = 0; i < count; ++i)
	{
		if (serial.hash == hash[i]) // Found
		{
			// We do not return already open instances since this would probably
			// cause problems with AGS' garbage collector.
			// Simply create a new instance always
			Joystick *joy = Joystick_create(i);
			Dprintf("[Joystick] Created from savefile: #%d %p\n", joy->id, joy);
			joyset.insert(joy);
			
			AGS_RESTORE(Joystick, joy, key);
			return;
		}
	}
	
	// Device no longer present, invalidate joystick
	AGS_RESTORE(Joystick, &dummy, key);
}

//==============================================================================

long JoystickCount()
{
	return count;
}

//------------------------------------------------------------------------------

long JoystickRescan()
{
	JOYCAPS joy;
	long found = false;
	
	// Construct a set with already found devices (by device id)
	std::set<int> list;
	for (int i = 0; i < (int) map.size(); ++i)
		list.insert(map[i]);

	for (int i = 0, num = joyGetNumDevs(); i < num; ++i)
	{
		if (!joyGetDevCaps(i, &joy, sizeof (joy)))
		{
			if (list.count(i) < 1) // New (working) device found
			{
				hash.push_back(Joystick_hash(i));
				count++;
				map.push_back(i);
				found = true;
			}
		}
	}

	return found ? 1 : 0;
}

//------------------------------------------------------------------------------

const char *JoystickName(long index)
{
	// Debug information (undocumented)
	if (index == -2)
	{
		#ifdef WINMM_VERSION
		#	define VER_AUTO ""
		#else
		#	define VER_AUTO "auto: "
		#endif
		return AGS_STRING(PRODUCT_NAME " v" FILE_VERSION " " VER_AUTO "winmm");
	}
	
	if ((index < 0) || (index >= count))
		return AGS_STRING("");
	
	return AGS_STRING(Joystick_getname(index));
}

//==============================================================================

Joystick *Joystick_Open(long index)
{
	if (index == INVALID_JOY) // User requests a fake joystick instance
	{
		AGS_OBJECT(Joystick, &dummy);
		return &dummy;
	}

	if ((index < 0) || (index >= count))
		engine->AbortGame("!JoystickOpen: No device exists for specified index.");
	
	Joystick *joy;
	
	// Check if there is already an open instance, if so return it
	// Note: It might be a better idea to create duplicates. If it messes with
	// AGS's garbage collector this part should be removed.
	if (joy = Joystick_find(index))
		return joy;
	
	// Create a new joystick instance
	joy = Joystick_create(index);
	Dprintf("[Joystick] Created from scratch: #%d %p\n", joy->id, joy);
	
	AGS_OBJECT(Joystick, joy);
	joyset.insert(joy);
	return joy;
}

//------------------------------------------------------------------------------

long Joystick_IsOpen(long index)
{
	std::set<Joystick *>::iterator it;
	for (it = joyset.begin(); it != joyset.end(); ++it)
		if ((*it)->id == index)
			return 1;

	return 0;
}

//------------------------------------------------------------------------------

void Joystick_Click(long button)
{
	engine->SimulateMouseClick(button);
}

//==============================================================================

void Joystick_Close(Joystick *joy)
{
	joyset.erase(joy);
	if (!joy || joy->id == INVALID_JOY)
		return;
	
	if (joy->state)
		delete joy->state;

	memset(joy, 0, sizeof (Joystick));
	joy->id = INVALID_JOY;
}

//------------------------------------------------------------------------------

long Joystick_Valid(Joystick *joy)
{
	if (!joy || joy->id == INVALID_JOY)
		return 0;
	
	if (Joystick_status(joy) == MMSYSERR_NODRIVER)
		return 0;
	
	return 1;
}

//------------------------------------------------------------------------------

long Joystick_Unplugged(Joystick *joy)
{
	if (!joy || joy->id == INVALID_JOY)
		return 0;
	
	return (Joystick_status(joy) == JOYERR_UNPLUGGED) ? 1 : 0;
}

//------------------------------------------------------------------------------

const char *Joystick_GetName(Joystick *joy)
{
	if (joy->id == INVALID_JOY)
		return AGS_STRING("");
	
	return AGS_STRING(Joystick_getname(joy->id));
}

//------------------------------------------------------------------------------

long Joystick_GetAxis(Joystick *joy, long index)
{
	switch (index)
	{
		case 0: return (joy->x);
		case 1: return (joy->y);
		case 2: return (joy->z);
		case 3: return (joy->u);
		case 4: return (joy->v);
		case 5: return (joy->w);
		default:
			engine->AbortGame("!GetAxis: No axis exists for specified index.");
			return (0);
	}
}

//------------------------------------------------------------------------------

long Joystick_IsButtonDown(Joystick *joy, long button)
{
	return ((joy->buttons >> button) & 1);
}

//------------------------------------------------------------------------------

void Joystick_Update(Joystick *joy)
{
	if (Joystick_Valid(joy))
		Joystick_update(joy);
}

//------------------------------------------------------------------------------

void Joystick_EnableEvents(Joystick *joy, long scope)
{
	if (!joy || joy->id == INVALID_JOY)
		return;
	
	joy->state->update(joy);
	joy->events = scope ? 1 : 2;
}

//------------------------------------------------------------------------------

void Joystick_DisableEvents(Joystick *joy)
{
	if (!joy || joy->id == INVALID_JOY)
		return;
	
	joy->events = 0;
}

//==============================================================================

inline Joystick *Joystick_find(long index)
{
	std::set<Joystick *>::iterator it;
	for (it = joyset.begin(); it != joyset.end(); ++it)
		if ((*it)->id == index)
			return *it;
	
	return NULL;
}

//------------------------------------------------------------------------------

Joystick *Joystick_create(long index) // Pre: map[index] exists
{
	JOYCAPS caps;
	joyGetDevCaps(map[index], &caps, sizeof (caps));
	
	Joystick *joy = new Joystick;
	memset(joy, 0, sizeof (Joystick));
	
	joy->id = index; // joystick id, not device id
	joy->button_count = caps.wNumButtons;
	joy->axis_count = caps.wNumAxes;
	
	joy->state = new JoyState(caps);
	Joystick_update(joy);
	joy->state->update(joy);

	return joy;
}

//------------------------------------------------------------------------------

long Joystick_status(Joystick *joy) // Pre: joy->id != INVALID_JOY
{
	JOYINFOEX info;
	info.dwSize = sizeof (info);
	info.dwFlags = JOY_RETURNALL;
	
	return joyGetPosEx(map[joy->id], &info);
}

//------------------------------------------------------------------------------

void Joystick_update(Joystick *joy) // Pre: joy->id != INVALID_JOY
{
	JOYINFOEX info;
	info.dwSize = sizeof (info);
	info.dwFlags = JOY_RETURNALL & ~JOY_RETURNPOV;
	//if (Has a POV control) Will it fail otherwise?
		info.dwFlags |= JOY_RETURNPOVCTS;
	info.dwPOV = 0;
	
	if (joyGetPosEx(map[joy->id], &info))
	{
		// Error!
		Dprintf("[Joystick] Could not update joy: #%d %p\n", joy->id, joy);
		return;
	}
	
	JoyState &s = *joy->state;
	joy->x = (long) ((s.ox + ((float) info.dwXpos)) * s.fx);
	joy->y = (long) ((s.oy + ((float) info.dwYpos)) * s.fy);
	joy->z = (long) ((s.oz + ((float) info.dwZpos)) * s.fz);
	joy->u = (long) ((s.ou + ((float) info.dwRpos)) * s.fu);
	joy->v = (long) ((s.ov + ((float) info.dwUpos)) * s.fv);
	joy->w = (long) ((s.ow + ((float) info.dwVpos)) * s.fw);
	
	joy->buttons = info.dwButtons;
	
	long &pov = joy->pov;
	pov = 0;
	if (info.dwPOV == JOY_POVCENTERED)
		return;
	
	if (info.dwPOV > JOY_POVBACKWARD)
		pov |= 8; /* Left */
	else if ((info.dwPOV < JOY_POVBACKWARD) && (info.dwPOV > JOY_POVFORWARD))
		pov |= 2; /* Right */
	
	if ((info.dwPOV > JOY_POVRIGHT) && (info.dwPOV < JOY_POVLEFT))
		pov |= 4; /* Down */
	else if ((info.dwPOV < JOY_POVRIGHT) || (info.dwPOV > JOY_POVLEFT))
		pov |= 1; /* Up */
}

//------------------------------------------------------------------------------

#define JOY_START_AXIS_CHECK { int change;
#define JOY_AXIS_CHECK(a,i) change = joy->a - last->a; \
	if ((change > JOY_THRESHOLD) || (change < -JOY_THRESHOLD)) axes |= 1 << i;
#define JOY_END_AXIS_CHECK }

#define JOY_EVENT(e,v) \
	engine->QueueGameScriptFunction(e, joy->events - 1, 2, (long) joy, v);

void Joystick_process(Joystick *joy) // Pre: joy->id != INVALID_JOY
{
	if (!joy->events)
		return;
	
	JoyState *&last = joy->state;
	int axes = 0;
	long pressed;
	bool hat;

	JOY_START_AXIS_CHECK
		JOY_AXIS_CHECK(x, 0)
		JOY_AXIS_CHECK(y, 1)
		JOY_AXIS_CHECK(z, 2)
		JOY_AXIS_CHECK(u, 3)
		JOY_AXIS_CHECK(v, 4)
		JOY_AXIS_CHECK(w, 5)
	JOY_END_AXIS_CHECK

	pressed = joy->buttons ^ last->buttons;
	hat = joy->pov != last->pov;

	if (!(axes || pressed || hat))
		return;

	last->update(joy);

	{
		int axis = 0;

		while (axes)
		{
			if (axes & 1)
				JOY_EVENT("on_joy_move", axis);

			axes >>= 1;
			axis++;
		}
	}

	{	
		int button = 0;
		pressed &= joy->buttons; // ignore button releases

		while (pressed)
		{
			if (pressed & 1)
				JOY_EVENT("on_joy_press", button);
			
			pressed >>= 1;
			button++;
		}
	}
	
	if (hat)
		JOY_EVENT("on_joy_pov", joy->pov);
}

//------------------------------------------------------------------------------
// This code have been borrowed from SDL2; original by Eckhard Stolberg.

const char *Joystick_getname(long index) // Pre: map[index] exists
{
	const long id = map[index];
	static const char *unknown = "Unknown joystick";
	JOYCAPS caps;
	
	if (joyGetDevCaps(id, &caps, sizeof (caps)))
		return AGS_STRING("");
	
	char path[512];
	char value[512];
	size_t size;
	HKEY root;
	HKEY key;
	
	// Open joystick device registery
	if (sprintf_s(path, sizeof (path), "%s\\%s\\%s", REGSTR_PATH_JOYCONFIG, caps.szRegKey, REGSTR_KEY_JOYCURR) < 0
	|| (RegOpenKeyExA(root = HKEY_LOCAL_MACHINE, path, 0, KEY_READ, &key)
	&& RegOpenKeyExA(root = HKEY_CURRENT_USER, path, 0, KEY_READ, &key)))
		return AGS_STRING(unknown);
	
	// Get Joystick keyname in registery
	size = sizeof (value);
	if (sprintf_s(path, sizeof (path), "Joystick%d%s", id + 1, REGSTR_VAL_JOYOEMNAME) < 0
	|| RegQueryValueExA(key, path, 0, 0, (BYTE *) value, (DWORD *) &size))
	{
		RegCloseKey(key);
		return AGS_STRING(unknown);
	}
	RegCloseKey(key);

	// Get the value of the key
	if (sprintf_s(path, sizeof (path), "%s\\%s", REGSTR_PATH_JOYOEM, value) < 0
	|| RegOpenKeyExA(root, path, 0, KEY_READ, &key))
		return AGS_STRING(unknown);

	// Get the name
	size = sizeof (value);
	if (RegQueryValueExA(key, REGSTR_VAL_JOYOEMNAME, 0, 0, (BYTE *) value, (DWORD *) &size))
	{
		RegCloseKey(key);
		return AGS_STRING(unknown);
	}
	RegCloseKey(key);
	
	return AGS_STRING(value);
}

//------------------------------------------------------------------------------

inline void getoemname(int id, char *value, size_t size)
{
	*value = 0;
	
	JOYCAPS caps;
	if (joyGetDevCaps(id, &caps, sizeof (caps)))
		return;
	
	char path[512];
	HKEY root;
	HKEY key;
	
	// Open joystick device registery
	if (sprintf_s(path, sizeof (path), "%s\\%s\\%s", REGSTR_PATH_JOYCONFIG, caps.szRegKey, REGSTR_KEY_JOYCURR) < 0
	|| (RegOpenKeyExA(root = HKEY_LOCAL_MACHINE, path, 0, KEY_READ, &key)
	&& RegOpenKeyExA(root = HKEY_CURRENT_USER, path, 0, KEY_READ, &key)))
		return;
	
	// Get Joystick keyname in registery
	if (sprintf_s(path, sizeof (path), "Joystick%d%s", id + 1, REGSTR_VAL_JOYOEMNAME) < 0
	|| RegQueryValueExA(key, path, 0, 0, (BYTE *) value, (DWORD *) &size))
	{
		RegCloseKey(key);
		*value = 0;
		return;
	}
	RegCloseKey(key);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

long Joystick_hash(int id)
{
	char value[512];	
	getoemname(id, value, sizeof (value));
	
	// FNV-1a hash algorithm
	static const unsigned long basis = 2166136261UL;
	static const unsigned long prime = 16777619UL;
	
	unsigned long h = basis;
	const unsigned char *ptr = (const unsigned char *)value;
	
	while (*ptr)
		h = (*ptr++ ^ h) * prime;
	
	// Check for collisions (two devices with the same oem-name)
	for (int i = 0; i < count; ++i)
		if (hash[i] == h)
			h++;
	
	return (long) h;
}

//==============================================================================

} /* namespace AGSJoystick(MM) */

//..............................................................................
