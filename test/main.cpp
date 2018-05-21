/*******************************************************
 * Linux joystick test application -- main file        *
 *                                                     *
 * Author: Ferry Timmers                               *
 *                                                     *
 * Date: 16:47 19-8-2015                               *
 *                                                     *
 * Description: An application to test joystick        *
 *              development on linux.                  *
 *******************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "engine.h"

using Engine::Value;
using Engine::Handle;

//------------------------------------------------------------------------------

struct Point { int x, y; };

int main(int argc, char *argv[])
{
	Engine::Initialize();
	
	Value args[] = {1337};
	Value val = Engine::Call("JoystickName", 1, args);
	printf("(%s)\n", (const char *) val);
	
	Handle<Point> test = new Point();
	if (!test.empty())
		(*test)->x = 1337;
	
	Engine::Terminate();
	return EXIT_SUCCESS;
}

//..............................................................................
