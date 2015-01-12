#include <QApplication>

#include "gamewindow.h"

#include <iostream>
#include <QScreen>
#include <QtOpenGL/QGLFormat>
#include <fstream>

int main(int argc, char* argv[]) {
    try {
        QApplication app(argc, argv);

        QSurfaceFormat format;
        if ((QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_4_3) != 0) {
            format.setVersion(4, 3);
        } else if ((QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_3_3) != 0) {
            format.setVersion(3, 3);
        } else {
            std::cerr << "Impossible de trouver une version d'opengl compatible" << std::endl;
            return -1;
        }
        format.setProfile(QSurfaceFormat::CoreProfile);
        #ifdef QT_DEBUG
        format.setOption(QSurfaceFormat::DebugContext);
        #endif
        format.setDepthBufferSize(24);
        format.setSwapInterval(1); // A changer pour caper ou non à 60FPS
        format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);

		std::ifstream file;
		file.open("config.txt", std::ios_base::in);

		int seed = 0;
		file >> seed;

		GameWindow mainWindow(seed);
        mainWindow.setFormat(format);

        QScreen* screen = app.primaryScreen();
        QRect rec = screen->geometry();
        #ifdef QT_DEBUG
        mainWindow.resize(rec.size().width(), rec.size().height() / 2);
        mainWindow.show();
        #endif
        #ifdef QT_NO_DEBUG
        mainWindow.resize(rec.size().width(), rec.size().height());
        mainWindow.show();
        //mainWindow.showFullScreen();
        #endif

        return app.exec();
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return -1;
}
