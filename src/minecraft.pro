
SOURCES += \
    main.cpp \
    camera.cpp \
    gamewindow.cpp \
    biomes/BiomeMap.cpp \
    chunkmanager.cpp \
	LightManager.cpp \
    meshgenerator.cpp \
    biomes/ChunkGenerator.cpp \
    libnoise/latlon.cpp \
    libnoise/noisegen.cpp \
    libnoise/model/cylinder.cpp \
    libnoise/model/line.cpp \
    libnoise/model/plane.cpp \
    libnoise/model/sphere.cpp \
    libnoise/module/abs.cpp \
    libnoise/module/add.cpp \
    libnoise/module/billow.cpp \
    libnoise/module/blend.cpp \
    libnoise/module/cache.cpp \
    libnoise/module/checkerboard.cpp \
    libnoise/module/clamp.cpp \
    libnoise/module/const.cpp \
    libnoise/module/curve.cpp \
    libnoise/module/cylinders.cpp \
    libnoise/module/displace.cpp \
    libnoise/module/exponent.cpp \
    libnoise/module/invert.cpp \
    libnoise/module/max.cpp \
    libnoise/module/min.cpp \
    libnoise/module/modulebase.cpp \
    libnoise/module/multiply.cpp \
    libnoise/module/perlin.cpp \
    libnoise/module/power.cpp \
    libnoise/module/ridgedmulti.cpp \
    libnoise/module/rotatepoint.cpp \
    libnoise/module/scalebias.cpp \
    libnoise/module/scalepoint.cpp \
    libnoise/module/select.cpp \
    libnoise/module/spheres.cpp \
    libnoise/module/terrace.cpp \
    libnoise/module/translatepoint.cpp \
    libnoise/module/turbulence.cpp \
    libnoise/module/voronoi.cpp \
    physic/physicmanager.cpp \
    player.cpp \
    wireframebox.cpp \
    postprocess/framebuffer.cpp \
    postprocess/postprocess.cpp \
    npc/npc.cpp \
    npc/pathfinding.cpp \
    npc/npcmanager.cpp \
    npc/creeper.cpp \
    voxelbuffer.cpp \
    save.cpp \
    particlesystem.cpp \
    chunk.cpp

    
QMAKE_CFLAGS_RELEASE  -= -O2
QMAKE_CFLAGS_RELEASE  -= -O1
QMAKE_CXXFLAGS_RELEASE  -= -O2
QMAKE_CXXFLAGS_RELEASE  -= -O1
QMAKE_CFLAGS_RELEASE  *= -O3
QMAKE_LFLAGS_RELEASE  *= -O3
QMAKE_CXXFLAGS_RELEASE  *= -O3

QMAKE_CXXFLAGS += -std=c++11

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


RESOURCES += \
    gestionnaire.qrc

HEADERS += \
    camera.h \
    gamewindow.h \
    biomes/BiomeMap.h \
    chunkmanager.h \
	LightManager.h \
    meshgenerator.h \
    coords.h \
    chunk.h \
    defs.h \
    biomes/ChunkGenerator.h \
    libnoise/basictypes.h \
    libnoise/exception.h \
    libnoise/interp.h \
    libnoise/latlon.h \
    libnoise/mathconsts.h \
    libnoise/misc.h \
    libnoise/noise.h \
    libnoise/noisegen.h \
    libnoise/vectortable.h \
    libnoise/model/cylinder.h \
    libnoise/model/line.h \
    libnoise/model/model.h \
    libnoise/model/plane.h \
    libnoise/model/sphere.h \
    libnoise/module/abs.h \
    libnoise/module/add.h \
    libnoise/module/billow.h \
    libnoise/module/blend.h \
    libnoise/module/cache.h \
    libnoise/module/checkerboard.h \
    libnoise/module/clamp.h \
    libnoise/module/const.h \
    libnoise/module/curve.h \
    libnoise/module/cylinders.h \
    libnoise/module/displace.h \
    libnoise/module/exponent.h \
    libnoise/module/invert.h \
    libnoise/module/max.h \
    libnoise/module/min.h \
    libnoise/module/module.h \
    libnoise/module/modulebase.h \
    libnoise/module/multiply.h \
    libnoise/module/perlin.h \
    libnoise/module/power.h \
    libnoise/module/ridgedmulti.h \
    libnoise/module/rotatepoint.h \
    libnoise/module/scalebias.h \
    libnoise/module/scalepoint.h \
    libnoise/module/select.h \
    libnoise/module/spheres.h \
    libnoise/module/terrace.h \
    libnoise/module/translatepoint.h \
    libnoise/module/turbulence.h \
    libnoise/module/voronoi.h \
    physic/body.h \
    physic/physicmanager.h \
    player.h \
    wireframebox.h \
    postprocess/framebuffer.h \
    postprocess/postprocess.h \
    npc/npc.h \
    npc/pathfinding.hpp \
    npc/npcmanager.h \
    npc/creeper.h \
    voxelbuffer.h \
    save.h \
    particlesystem.h
