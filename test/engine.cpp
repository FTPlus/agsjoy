/*************************************************************
 * engine emulation -- See header file for more information. *
 *************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <map>
#include <set>
#include <vector>

#include "engine.h"

#define LIST(x,y) LIST ## x(y)
#define LIST0(x)
#define LIST1(x) x(0)
#define LIST2(x) LIST1(x), x(1)
#define LIST3(x) LIST2(x), x(2)
#define LIST4(x) LIST3(x), x(3)
#define LIST5(x) LIST4(x), x(4)
#define LIST6(x) LIST5(x), x(5)
#define LIST7(x) LIST6(x), x(6)
#define LIST8(x) LIST7(x), x(7)
#define LIST9(x) LIST8(x), x(8)

//------------------------------------------------------------------------------

#undef AGSIFUNC
#ifdef WINDOWS_VERSION
#	define AGSIFUNC(type) type __stdcall
#	define DLLIMPORT extern "C" __declspec(dllimport)
#	define strdup _strdup
#else
#	define AGSIFUNC(type) type
#	define DLLIMPORT extern "C"
#endif

DLLIMPORT void AGS_EngineStartup(IAGSEngine *);
DLLIMPORT void AGS_EngineShutdown(void);
DLLIMPORT int  AGS_EngineOnEvent(int, int);

IAGSEngine engine;

//------------------------------------------------------------------------------

struct Type
{
	std::string name;
};
std::map<std::string,Type> types;

struct Function
{
	std::string name;
	std::string type;
	int params;
	void *addr;
};
std::map<std::string,Function> functions;

std::set<long> events;

struct Object
{
	long key;
	IAGSScriptManagedObject *manager;
	long refcount;
	void *addr;
	
	long operator ++();
	long operator --();
	
	static long find(void *);
	static void clear();
};
std::vector<Object> objects;

//------------------------------------------------------------------------------

long Object::operator ++()
{
	return ++refcount;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

long Object::operator --()
{
	if (!refcount)
		return 0;
	long count = --refcount;
	if (!count)
	{
		if (manager->Dispose((const char *) addr, 0))
			addr = NULL;
	}
	return count;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

long Object::find(void *addr)
{
	for (size_t i = 0; i < objects.size(); ++i)
		if (objects[i].addr == addr)
			return i;
	return -1;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void Object::clear()
{
	for (size_t i = 0; i < objects.size(); ++i)
		if (objects[i].addr)
			if (objects[i].manager->Dispose((const char *) objects[i].addr, 1))
				objects[i].addr = NULL;
}

//==============================================================================

namespace Engine {

void Initialize()
{
	engine.version = 18;
	AGS_EngineStartup(&engine);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

bool Trigger(long event, long data)
{
	if (!events.count(event))
		return false;
	return !!AGS_EngineOnEvent(event, data);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void Terminate()
{
	Object::clear();
	objects.clear();
	AGS_EngineShutdown();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

Value Call(const char *name, int argc, Value *argv)
{
	if (!functions.count(name))
	{
		printf("Call to '%s' failed: function was not registered.\n", name);
		return Value();
	}
	Function func = functions[name];
	
	#define LONG(x) long
	#define ARGV(x) argv[x]
	#define CALL(x) {                                                          \
		long (*call) (LIST(x, LONG));                                          \
		call = (long (*) (LIST(x, LONG))) (func.addr);                         \
		return Value((long) call(LIST(x, ARGV)));                              \
	}
	
	switch (argc)
	{
		case 0: CALL(0);
		case 1: CALL(1);
		case 2: CALL(2);
		case 3: CALL(3);
		case 4: CALL(4);
		case 5: CALL(5);
		case 6: CALL(6);
		case 7: CALL(7);
		case 8: CALL(8);
		case 9: CALL(9);
		default:
			printf("Call to '%s' failed: too many parameters.\n", name);
	}
	
	#undef LONG
	#undef ARGV
	#undef CALL
}

//------------------------------------------------------------------------------

bool Save(const char *filename)
{
	FILE *fp = fopen(filename, "wb");
	if (!fp)
		return false;
	char buffer[4096];
	for (size_t i = 0; i < objects.size(); ++i)
	{
		size_t size = objects[i].manager->Serialize
			((const char *) objects[i].addr, buffer, sizeof (buffer));
		fwrite(&size, sizeof (size_t), 1, fp);
		fwrite(buffer, size, 1, fp);
	}
	size_t size = 0;
	fwrite(&size, sizeof (size_t), 1, fp);
	fclose(fp);
	return true;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

bool Load(const char *filename)
{
	Object::clear();
	objects.clear();
	
	FILE *fp = fopen(filename, "wb");
	if (!fp)
		return false;
	fseek(fp, 0, SEEK_END);
	long bufsize = ftell(fp);
	char *buffer = (char *) malloc(bufsize);
	rewind(fp);
	if (fread(buffer, bufsize, 1, fp) == bufsize)
	{
		fclose(fp);
		return false;
	}
	fclose(fp);
	
	for (const char *ptr = buffer;;)
	{
		size_t size = *((size_t *)ptr);
		ptr += sizeof (size_t);
		
	}
	
	return true;
}

//------------------------------------------------------------------------------

HandleBase::HandleBase(void *obj) : key(Object::find(obj))
{
	if (key >= 0)
		++objects[key];
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

HandleBase::HandleBase(const HandleBase &other) : key(other.key)
{
	if (key >= 0)
		++objects[key];
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

HandleBase::~HandleBase()
{
	if (key >= 0)
		--objects[key];
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

HandleBase &HandleBase::operator =(const HandleBase &other)
{
	if (key >= 0)
		--objects[key];
	key = other.key;
	if (key >= 0)
		++objects[key];
	return *this;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

HandleBase::operator void *()
{
	if (key >= 0)
		return objects[key].addr;
	return NULL;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const char *HandleBase::type()
{
	if (key >= 0)
		return objects[key].manager->GetType();
	return NULL;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

Value HandleBase::call(const char *name, int argc, Value *argv)
{
	if (key < 0)
	{
		printf("Call to '%s' failed: handle is empty.\n", name);
		return Value();
	}
	
	Function func;
	std::string fullname = std::string(type()) + "::" + name;
	if (functions.count(fullname))
		func = functions[fullname];
	else
	{
		if (!functions.count(name))
		{
			printf("Call to '%s' failed: function was not registered.\n", name);
			return Value();
		}
		func = functions[name];
	}
	
	#define LONG(x) long
	#define ARGV(x) argv[x]
	#define CALL(x) {                                                          \
		long (*call) (void *, LIST(x, LONG));                                  \
		call = (long (*) (void *, LIST(x, LONG))) (func.addr);                 \
		return Value((long) call(objects[key].addr, LIST(x, ARGV)));           \
	}
	
	switch (argc)
	{
		case 0: return Value((long) ((long(*) (void *)) (func.addr))(objects[key].addr));
		case 1: CALL(1);
		case 2: CALL(2);
		case 3: CALL(3);
		case 4: CALL(4);
		case 5: CALL(5);
		case 6: CALL(6);
		case 7: CALL(7);
		case 8: CALL(8);
		case 9: CALL(9);
		default:
			printf("Call to '%s' failed: too many parameters.\n", name);
	}
	
	#undef LONG
	#undef ARGV
	#undef CALL
}

//------------------------------------------------------------------------------

} /* namespace Engine */

//==============================================================================

AGSIFUNC(const char*) IAGSEngine::CreateScriptString(const char *fromText)
{
	return strdup(fromText);
	// This creates a memleak which we'll simply ignore:
	// this is for testing purposes only!
}

//------------------------------------------------------------------------------

AGSIFUNC(void) IAGSEngine::AbortGame (const char *reason)
{
	printf("Fatal error: %s\n", reason);
	abort();
}

//------------------------------------------------------------------------------

AGSIFUNC(void) IAGSEngine::RegisterScriptFunction(const char *name, void *address)
{
	Function func;
	const char *ptr;
	
	if (ptr = strchr(name, ':'))
	{
		func.type = std::string(name, ptr - name);
		name = ptr + 2;
	}
	
	if (ptr = strchr(name, '^'))
	{
		func.name = std::string(name, ptr - name);
		func.params = atoi(ptr + 1);
	}
	else
	{
		func.name = std::string(name);
		func.params = -1;
	}
	
	func.addr = address;
	
	functions[func.name] = func;
	functions[func.type + "::" + func.name] = func;
}

//------------------------------------------------------------------------------

AGSIFUNC(void) IAGSEngine::RequestEventHook(int32 event)
{
	events.insert(event);
}

//------------------------------------------------------------------------------

AGSIFUNC(void) IAGSEngine::UnrequestEventHook(int32 event)
{
	events.erase(event);
}

//------------------------------------------------------------------------------

AGSIFUNC(void) IAGSEngine::PrintDebugConsole(const char *text)
{
	printf("[log] %s\n", text);
}

//------------------------------------------------------------------------------

AGSIFUNC(int) IAGSEngine::CanRunScriptFunctionNow()
{
	return 1;
}

//------------------------------------------------------------------------------

AGSIFUNC(int) IAGSEngine::CallGameScriptFunction(const char *name,
	int32 globalScript, int32 numArgs, int32 arg1, int32 arg2, int32 arg3)
{
	const char *mode = globalScript ? "global call" : "room call";
	switch (numArgs)
	{
		case 0: printf("[%s] %s()", mode, name); break;
		case 1: printf("[%s] %s(%p)", mode, name, arg1); break;
		case 2: printf("[%s] %s(%p, %p)", mode, name, arg1, arg2); break;
		case 3: printf("[%s] %s(%p, %p, %p)", mode, name, arg1, arg2, arg3); break;
	}
	return 1;
}

//------------------------------------------------------------------------------

AGSIFUNC(void) IAGSEngine::QueueGameScriptFunction(const char *name,
	int32 globalScript, int32 numArgs, int32 arg1, int32 arg2)
{
	IAGSEngine::CallGameScriptFunction(name, globalScript, numArgs, arg1, arg2, 0);
}

//------------------------------------------------------------------------------

AGSIFUNC(int) IAGSEngine::RegisterManagedObject(const void *object,
	IAGSScriptManagedObject *callback)
{
	Object obj;
	obj.key = objects.size();
	obj.manager = callback;
	obj.refcount = 0;
	obj.addr = (void *) object;
	
	objects.push_back(obj);
	return obj.key;
}

//------------------------------------------------------------------------------

AGSIFUNC(void) IAGSEngine::AddManagedObjectReader(const char *typeName,
	IAGSManagedObjectReader *reader)
{
}

//------------------------------------------------------------------------------

AGSIFUNC(void) IAGSEngine::RegisterUnserializedObject(int key,
	const void *object, IAGSScriptManagedObject *callback)
{
}

//------------------------------------------------------------------------------

AGSIFUNC(void*) IAGSEngine::GetManagedObjectAddressByKey(int key)
{
	if (key < 0 || key >= objects.size())
		return NULL;
	return objects[key].addr;
}

//------------------------------------------------------------------------------

AGSIFUNC(int) IAGSEngine::GetManagedObjectKeyByAddress(const char *address)
{
	return Object::find((void *) address);
}

//------------------------------------------------------------------------------

AGSIFUNC(int) IAGSEngine::IncrementManagedObjectRefCount(const char *address)
{
	long key = Object::find((void *) address);
	if (key >= 0)
		return ++objects[key];
	return 0;
}

//------------------------------------------------------------------------------

AGSIFUNC(int) IAGSEngine::DecrementManagedObjectRefCount(const char *address)
{
	long key = Object::find((void *) address);
	if (key >= 0)
		return --objects[key];
	return 0;
}

//------------------------------------------------------------------------------

// Unemulated functions, just return bogus:

AGSIFUNC(const char*) IAGSEngine::GetEngineVersion () { return NULL; }
#ifdef WINDOWS_VERSION
  AGSIFUNC(HWND) IAGSEngine::GetWindowHandle() { return 0; }
  AGSIFUNC(LPDIRECTDRAW2) IAGSEngine::GetDirectDraw2 () { return NULL; }
  AGSIFUNC(LPDIRECTDRAWSURFACE2) IAGSEngine::GetBitmapSurface (BITMAP *) { return NULL; }
#endif
AGSIFUNC(BITMAP *) IAGSEngine::GetScreen () { return NULL; }

AGSIFUNC(int) IAGSEngine::GetSavedData (char *buffer, int32 bufsize) { return 0; }

AGSIFUNC(BITMAP *) IAGSEngine::GetVirtualScreen () { return NULL; }
AGSIFUNC(void) IAGSEngine::DrawText (int32 x, int32 y, int32 font, int32 color, char *text) {}
AGSIFUNC(void) IAGSEngine::GetScreenDimensions (int32 *width, int32 *height, int32 *coldepth) {}
AGSIFUNC(unsigned char**) IAGSEngine::GetRawBitmapSurface (BITMAP *) { return NULL; }
AGSIFUNC(void) IAGSEngine::ReleaseBitmapSurface (BITMAP *) {}
AGSIFUNC(void) IAGSEngine::GetMousePosition (int32 *x, int32 *y) {}

AGSIFUNC(int) IAGSEngine::GetCurrentRoom () { return 0; }
AGSIFUNC(int) IAGSEngine::GetNumBackgrounds () { return 0; }
AGSIFUNC(int) IAGSEngine::GetCurrentBackground () { return 0; }
AGSIFUNC(BITMAP *) IAGSEngine::GetBackgroundScene (int32) { return NULL; }
AGSIFUNC(void) IAGSEngine::GetBitmapDimensions (BITMAP *bmp, int32 *width, int32 *height, int32 *coldepth) {}

AGSIFUNC(int) IAGSEngine::FWrite (void *, int32, int32) { return 0; }
AGSIFUNC(int) IAGSEngine::FRead (void *, int32, int32) { return 0; }
AGSIFUNC(void) IAGSEngine::DrawTextWrapped (int32 x, int32 y, int32 width, int32 font, int32 color, const char *text) {}
AGSIFUNC(void) IAGSEngine::SetVirtualScreen (BITMAP *) {}
AGSIFUNC(int) IAGSEngine::LookupParserWord (const char *word) { return 0; }
AGSIFUNC(void) IAGSEngine::BlitBitmap (int32 x, int32 y, BITMAP *, int32 masked) {}
AGSIFUNC(void) IAGSEngine::PollSystem () {}

AGSIFUNC(int) IAGSEngine::GetNumCharacters () { return 0; }
AGSIFUNC(AGSCharacter*) IAGSEngine::GetCharacter (int32) { return NULL; }
AGSIFUNC(AGSGameOptions*) IAGSEngine::GetGameOptions () { return NULL; }
AGSIFUNC(AGSColor*) IAGSEngine::GetPalette() { return NULL; }
AGSIFUNC(void) IAGSEngine::SetPalette (int32 start, int32 finish, AGSColor*) {}

AGSIFUNC(int) IAGSEngine::GetPlayerCharacter () { return 0; }
AGSIFUNC(void) IAGSEngine::RoomToViewport (int32 *x, int32 *y) {}
AGSIFUNC(void) IAGSEngine::ViewportToRoom (int32 *x, int32 *y) {}
AGSIFUNC(int) IAGSEngine::GetNumObjects () { return 0; }
AGSIFUNC(AGSObject*) IAGSEngine::GetObject (int32) { return NULL; }
AGSIFUNC(BITMAP *) IAGSEngine::GetSpriteGraphic (int32) { return NULL; }
AGSIFUNC(BITMAP *) IAGSEngine::CreateBlankBitmap (int32 width, int32 height, int32 coldep) { return NULL; }
AGSIFUNC(void) IAGSEngine::FreeBitmap (BITMAP *) {}

AGSIFUNC(BITMAP *) IAGSEngine::GetRoomMask(int32) { return NULL; }

AGSIFUNC(AGSViewFrame *) IAGSEngine::GetViewFrame(int32 view, int32 loop, int32 frame) { return NULL; }
AGSIFUNC(int) IAGSEngine::GetWalkbehindBaseline(int32 walkbehind) { return 0; }
AGSIFUNC(void *) IAGSEngine::GetScriptFunctionAddress(const char * funcName) { return NULL; }
AGSIFUNC(int) IAGSEngine::GetBitmapTransparentColor(BITMAP *) { return 0; }
AGSIFUNC(int) IAGSEngine::GetAreaScaling (int32 x, int32 y) { return 0; }
AGSIFUNC(int) IAGSEngine::IsGamePaused() { return 0; }

AGSIFUNC(int) IAGSEngine::GetRawPixelColor (int32 color) { return 0; }

AGSIFUNC(int) IAGSEngine::GetSpriteWidth (int32) { return 0; }
AGSIFUNC(int) IAGSEngine::GetSpriteHeight (int32) { return 0; }
AGSIFUNC(void) IAGSEngine::GetTextExtent (int32 font, const char *text, int32 *width, int32 *height) {}
AGSIFUNC(void) IAGSEngine::PlaySoundChannel (int32 channel, int32 soundType, int32 volume, int32 loop, const char *filename) {}
AGSIFUNC(int) IAGSEngine::IsChannelPlaying (int32 channel) { return 0; }

AGSIFUNC(void) IAGSEngine::MarkRegionDirty(int32 left, int32 top, int32 right, int32 bottom) {}
AGSIFUNC(AGSMouseCursor *) IAGSEngine::GetMouseCursor(int32 cursor) { return NULL; }
AGSIFUNC(void) IAGSEngine::GetRawColorComponents(int32 coldepth, int32 color, int32 *red, int32 *green, int32 *blue, int32 *alpha) {}
AGSIFUNC(int) IAGSEngine::MakeRawColorPixel(int32 coldepth, int32 red, int32 green, int32 blue, int32 alpha) { return 0; }
AGSIFUNC(int) IAGSEngine::GetFontType(int32 fontNum) { return 0; }
AGSIFUNC(int) IAGSEngine::CreateDynamicSprite(int32 coldepth, int32 width, int32 height) { return 0; }
AGSIFUNC(void) IAGSEngine::DeleteDynamicSprite(int32 slot) {}
AGSIFUNC(int) IAGSEngine::IsSpriteAlphaBlended(int32 slot) { return 0; }

AGSIFUNC(void) IAGSEngine::BlitSpriteTranslucent(int32 x, int32 y, BITMAP *, int32 trans) {}
AGSIFUNC(void) IAGSEngine::BlitSpriteRotated(int32 x, int32 y, BITMAP *, int32 angle) {}

#ifdef WINDOWS_VERSION
  AGSIFUNC(LPDIRECTSOUND) IAGSEngine::GetDirectSound() { return NULL; }
#endif
AGSIFUNC(void) IAGSEngine::DisableSound() {}

AGSIFUNC(void) IAGSEngine::NotifySpriteUpdated(int32 slot) {}
AGSIFUNC(void) IAGSEngine::SetSpriteAlphaBlended(int32 slot, int32 isAlphaBlended) {}

AGSIFUNC(void) IAGSEngine::SetMousePosition(int32 x, int32 y) {}
AGSIFUNC(void) IAGSEngine::SimulateMouseClick(int32 button) {}
AGSIFUNC(int) IAGSEngine::GetMovementPathWaypointCount(int32 pathId) { return 0; }
AGSIFUNC(int) IAGSEngine::GetMovementPathLastWaypoint(int32 pathId) { return 0; }
AGSIFUNC(void) IAGSEngine::GetMovementPathWaypointLocation(int32 pathId, int32 waypoint, int32 *x, int32 *y) {}
AGSIFUNC(void) IAGSEngine::GetMovementPathWaypointSpeed(int32 pathId, int32 waypoint, int32 *xSpeed, int32 *ySpeed) {}

AGSIFUNC(const char*) IAGSEngine::GetGraphicsDriverID() { return NULL; }

AGSIFUNC(int) IAGSEngine::IsRunningUnderDebugger() { return 0; }
AGSIFUNC(void) IAGSEngine::BreakIntoDebugger() {}
AGSIFUNC(void) IAGSEngine::GetPathToFileInCompiledFolder(const char* fileName, char* buffer) {}

#ifdef WINDOWS_VERSION
  AGSIFUNC(LPDIRECTINPUTDEVICE) IAGSEngine::GetDirectInputKeyboard() { return NULL; }
  AGSIFUNC(LPDIRECTINPUTDEVICE) IAGSEngine::GetDirectInputMouse() { return NULL; }
#endif
AGSIFUNC(IAGSFontRenderer*) IAGSEngine::ReplaceFontRenderer(int fontNumber, IAGSFontRenderer* newRenderer) { return NULL; }

//..............................................................................
