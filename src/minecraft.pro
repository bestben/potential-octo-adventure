
SOURCES += \
    main.cpp \
    camera.cpp \
    gamewindow.cpp \
    biomes/BiomeLayer.cpp \
    biomes/BiomeMap.cpp \
    biomes/main.cpp \
    biomes/OpenSimplexNoise.cpp \
    biomes/ChunkGenerator.cpp

unix{
	QMAKE_CXXFLAGS+=-fopenmp
	QMAKE_LFLAGS+=-fopenmp
        QMAKE_CXXFLAGS += -std=c++11
}
win32{
	QMAKE_CXXFLAGS += -openmp
}
    
target.path = $$[QT_INSTALL_EXAMPLES]/gui/openglwindow
QT += opengl
INSTALLS += target


RESOURCES += \
    gestionnaire.qrc

HEADERS += \
    camera.h \
    gamewindow.h \
    biomes/BiomeLayer.h \
    biomes/BiomeMap.h \
    biomes/defs.h \
    biomes/OpenSimplexNoise.hpp \
    defs.h \
    biomes/ChunkGenerator.h
