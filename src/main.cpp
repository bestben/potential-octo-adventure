#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "gamewindow.h"

#include <iostream>
#include <fstream>

int main(int /*argc*/, char* /*argv[]*/) {
#ifdef WIN32
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_crtBreakAlloc
#endif
    try {
		std::ifstream file;
		file.open("config.txt", std::ios_base::in);
		int seed = 0;
		file >> seed;

		GameWindow mainWindow(seed);
		mainWindow.init();
		mainWindow.run();

		return 0;
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return -1;
}
