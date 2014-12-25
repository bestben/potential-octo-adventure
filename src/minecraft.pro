
SOURCES += \
    main.cpp \
    camera.cpp \
    gamewindow.cpp \
    biomes/BiomeLayer.cpp \
    biomes/BiomeMap.cpp \
    chunkmanager.cpp \
    meshgenerator.cpp \
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
    chunkmanager.h \
    meshgenerator.h \
    chunk.h \
    biomes/OpenSimplexNoise.hpp \
    defs.h \
    biomes/ChunkGenerator.h
