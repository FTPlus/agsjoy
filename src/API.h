/*******************************************************
 * Plugin API -- header file                           *
 *                                                     *
 * Author: Ferry "Wyz" Timmers                         *
 *                                                     *
 * Date: 13:30 4-2-2014                                *
 *                                                     *
 * Description: Provides an abstraction layer for the  *
 *              AGS plugin interface.                  *
 *******************************************************/

#ifndef _API_H
#define _API_H

// windows.h must be included before agsplugin.h
// for other platform includes it's not required
#if defined(_WIN32) || defined(_WINDOWS_)
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	ifndef _WINDOWS_
#		define _WINDOWS_
#	endif
#endif

//------------------------------------------------------------------------------
// Platform detection

#ifndef _WINDOWS_  // Note: WINDOWS_VERSION will be set by agsplugin.h later
#	if defined(linux) || defined (__linux__)
#		ifndef LINUX_VERSION
#			define LINUX_VERSION
#		endif
#	elif defined(__APPLE__)
#		include "TargetConditionals.h"
#		if defined(TARGET_OS_MAC)
#			ifndef MAC_VERSION
#				define MAC_VERSION
#			endif
#		elif defined(TARGET_OS_IPHONE)
#			ifndef IOS_VERSION
#				define IOS_VERSION
#			endif
#		endif
#	endif
#else
#	if !defined(WINMM_VERSION) && !defined(DX8_VERSION) && !defined(SDL_VERSION)
#		define WIN_AUTO_VERSION
#	endif
#endif

// Include AGS plugin interface header

#include "agsplugin.h"

//------------------------------------------------------------------------------
// Global (platform independed) defines
// Changing these would most likely tick off game developers

#define JOY_THRESHOLD 256
#define INVALID_JOY -1

//------------------------------------------------------------------------------
// Macros

#define STRINGIFY(s) STRINGIFY_X(s)
#define STRINGIFY_X(s) #s

#define AGS_STRING(x)     AGSJoyAPI::engine->CreateScriptString(x)
#define AGS_OBJECT(c,x)   AGSJoyAPI::engine->RegisterManagedObject((void *) (x), &ags ## c)
#define AGS_RESTORE(c,x,i)AGSJoyAPI::engine->RegisterUnserializedObject((i), (void *) (x), &ags ## c)
#define AGS_HOLD(x)       AGSJoyAPI::engine->IncrementManagedObjectRefCount((const char *) (x))
#define AGS_RELEASE(x)    AGSJoyAPI::engine->DecrementManagedObjectRefCount((const char *) (x))
#define AGS_TO_KEY(x)     AGSJoyAPI::engine->GetManagedObjectKeyByAddress((const char *) (x))
#define AGS_FROM_KEY(c,x) ((c *) AGSJoyAPI::engine->GetManagedObjectAddressByKey(x))

#define AGS_FUNCTION(x)   engine->RegisterScriptFunction(#x, (void *) (x));
#define AGS_METHOD(c,x,a) engine->RegisterScriptFunction(#c "::" #x "^" #a, (void *) (c ## _ ## x));
#define AGS_MEMBER(c,x)   engine->RegisterScriptFunction(#c "::get_" #x, (void *) (c ## _get_ ## x)); \
                          engine->RegisterScriptFunction(#c "::set_" #x, (void *) (c ## _set_ ## x));
#define AGS_READONLY(c,x) engine->RegisterScriptFunction(#c "::get_" #x, (void *) (c ## _get_ ## x));
#define AGS_ARRAY(c,x)    engine->RegisterScriptFunction(#c "::geti_" #x, (void *) (c ## _geti_ ## x)); \
                          engine->RegisterScriptFunction(#c "::seti_" #x, (void *) (c ## _seti_ ## x));
#define AGS_CLASS(c)      engine->AddManagedObjectReader(#c, &ags ## c);


#ifndef AGSMAIN
	#define AGSMAIN extern
#endif

#define AGS_DEFINE_CLASS(c)                                                      \
struct AGS ## c : public IAGSScriptManagedObject,                                \
                  public IAGSManagedObjectReader                                 \
{                                                                                \
	virtual const char *GetType() { return #c; }                                 \
	virtual int Dispose(const char *address, bool force);                        \
	virtual int Serialize(const char *address, char *buffer, int bufsize);       \
	virtual void Unserialize(int key, const char *serializedData, int dataSize); \
} AGSMAIN ags ## c;

#ifdef WIN_AUTO_VERSION
#define FALLBACK(c,a)                            \
if (c) { using namespace AGSJoystickDX8; a; }    \
else { using namespace AGSJoystickMM; a; }
#else
#define FALLBACK(c,a)                            \
{ using namespace AGSJoystick; a; }
#endif

//------------------------------------------------------------------------------
// Globals

/// Wrapper for the plugin interface
namespace AGSJoyAPI {

extern IAGSEngine *engine;

} /* namespace AGSJoyAPI */

#endif /* _API_H */

//..............................................................................
