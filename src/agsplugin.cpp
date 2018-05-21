/***********************************************************
 * AGS Joystick                                            *
 *                                                         *
 * Author: Ferry Timmers                                   *
 *                                                         *
 * Date: 19-08-10 16:55                                    *
 *                                                         *
 * Description: Joystick (game controller really) plugin   *
 *              for Adventure Game Studio.                 *
 *                                                         *
 ***********************************************************/

#define MIN_EDITOR_VERSION 1
#define MIN_ENGINE_VERSION 18

#define AGSMAIN

//#include "agsplugin.h" // Included by API.h
#include "API.h"
#include "Joystick.h"

DLLEXPORT int AGS_PluginV2() { return 1; }

//==============================================================================
// The standard Windows DLL entry point
/*
BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
	switch(reason)
	{
		case DLL_PROCESS_ATTACH:
			break;
		
		case DLL_PROCESS_DETACH:
			break;
		
		case DLL_THREAD_ATTACH:
			break;
		
		case DLL_THREAD_DETACH:
			break;
	}
	
	return TRUE;
}
*/
//==============================================================================

// ***** Design time *****

IAGSEditor *editor; // Editor interface

const char *ourScriptHeader = JOYSTICK_HEADER;

//------------------------------------------------------------------------------

const char *AGS_GetPluginName()
{
	return ("Joystick plugin");
}

//------------------------------------------------------------------------------

int AGS_EditorStartup(IAGSEditor *lpEditor)
{
	// User has checked the plugin to use it in their game
	
	// If it's an earlier version than what we need, abort.
	if (lpEditor->version < MIN_EDITOR_VERSION)
		return (-1);
	
	editor = lpEditor;
	editor->RegisterScriptHeader(ourScriptHeader);
	
	return (0);
}

//------------------------------------------------------------------------------

void AGS_EditorShutdown()
{
	// User has un-checked the plugin from their game
	editor->UnregisterScriptHeader(ourScriptHeader);
}

//------------------------------------------------------------------------------

void AGS_EditorProperties(HWND parent)
{
	#ifdef WINDOWS_VERSION
	MessageBox(parent, "Joystick and game controller plugin, by Wyz August 2010",
	           "About", MB_OK | MB_ICONINFORMATION);
	#endif
}

/*
int AGS_EditorSaveGame(char *buffer, int bufsize) { return (0); }
void AGS_EditorLoadGame(char *buffer, int bufsize) {}
*/
//==============================================================================

// ***** Run time *****

#ifdef WIN_AUTO_VERSION
bool fallbackstate = false; // TODO: Add DirectX 8 detection here
#endif

//------------------------------------------------------------------------------

void AGS_EngineStartup(IAGSEngine *lpEngine)
{
	using namespace AGSJoyAPI;
	
	engine = lpEngine;
	
	// Make sure it's got the version with the features we need
	if (engine->version < MIN_ENGINE_VERSION)
		engine->AbortGame("Plugin needs engine version " STRINGIFY(MIN_ENGINE_VERSION) " or newer.");
	
	// Initialize plugin
	FALLBACK(fallbackstate, Initialize());
	
	// Script bindings
	FALLBACK(fallbackstate, JOYSTICK_ENTRY);
	
	// Request event hooks
	engine->RequestEventHook(AGSE_PRERENDER);
}

//------------------------------------------------------------------------------

void AGS_EngineShutdown()
{
	// Terminate plugin
	FALLBACK(fallbackstate, Terminate());
}

//------------------------------------------------------------------------------

int AGS_EngineOnEvent(int event, int data)
{
	switch (event)
	{
		// Note: Is this really the best moment to update? Maybe we should do it
		// AFTER drawing so it would be closer to the next game logic update.
		// Try AGSE_FINALSCREENDRAW instead.
		case AGSE_PRERENDER:
			FALLBACK(fallbackstate, Update());
			break;

		default:
			break;
	}
	
	// Return 1 to stop event from processing further (when needed)
	return (0);
}

//------------------------------------------------------------------------------
/*
int AGS_EngineDebugHook(const char *scriptName, int lineNum, int reserved) {}
void AGS_EngineInitGfx(const char *driverID, void *data) {}
*/
//..............................................................................
