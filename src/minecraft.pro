
SOURCES += \
    main.cpp \
    camera.cpp \
    gamewindow.cpp \
    chunkmanager.cpp \
    meshgenerator.cpp

unix{
	QMAKE_CXXFLAGS+=-fopenmp
	QMAKE_LFLAGS+=-fopenmp
}
win32{
	QMAKE_CXXFLAGS += -openmp
}
    
target.path = $$[QT_INSTALL_EXAMPLES]/gui/openglwindow
QT += opengl
INSTALLS += target
QMAKE_CXXFLAGS += -std=c++11


RESOURCES += \
    gestionnaire.qrc

HEADERS += \
    camera.h \
    gamewindow.h \
    chunkmanager.h \
    meshgenerator.h \
    chunk.h
