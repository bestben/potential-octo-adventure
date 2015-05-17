#include "gamewindow.h"

#include <iostream>
#include <fstream>

int main(int /*argc*/, char* /*argv[]*/) {
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
