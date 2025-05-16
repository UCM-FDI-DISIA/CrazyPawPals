#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <iostream>
#include "game/Game.h"
#include <SDL.h>

int main(int, char**) {

	//para buscar donde esta el memory leaks
	//_CrtSetBreakAlloc(28115);
	if(Game::Init()) {
		Game::Instance()->start();
		Game::Instance()->Release();
	}
	// try {
	// }
	// catch (const std::string& e) { // catch exceptions thrown as strings
	// 	std::cerr << e << std::endl;
	// 	assert(false && "fatal error: exception thrown as string through std::cerr");
	// }
	// catch (const char* e) { // catch exceptions thrown as char*
	// 	std::cerr << e << std::endl;
	// 	assert(false && "fatal error: exception thrown as char* through std::cerr");
	// }
	// catch (const std::exception& e) { // catch exceptions thrown as a sub-type of std::exception
	// 	std::cerr << e.what();
	// 	assert(false && "fatal error: exception thrown as std::exception through std::cerr");
	// }
	// catch (...) {
	// 	std::cerr << "Caught and exception of unknown type ...";
	// 	assert(false && "fatal error: exception thrown as unknown type through std::cerr");
	// }

	//_CrtDumpMemoryLeaks();
	return 0;
}
