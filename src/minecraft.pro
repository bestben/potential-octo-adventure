
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
    chunk.cpp \
    glm/detail/glm.cpp \
    libnoise/win32/dllmain.cpp \
    utilities/openglprogramshader.cpp \
    utilities/openglbuffer.cpp \
    utilities/openglvertexarrayobject.cpp \
    utilities/time.cpp \
    utilities/opengltexture.cpp

    
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

debug {
  DEFINES += MI_DEBUG
}
release {

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
    particlesystem.h \
    glm/detail/_features.hpp \
    glm/detail/_fixes.hpp \
    glm/detail/_noise.hpp \
    glm/detail/_swizzle.hpp \
    glm/detail/_swizzle_func.hpp \
    glm/detail/_vectorize.hpp \
    glm/detail/func_common.hpp \
    glm/detail/func_exponential.hpp \
    glm/detail/func_geometric.hpp \
    glm/detail/func_integer.hpp \
    glm/detail/func_matrix.hpp \
    glm/detail/func_noise.hpp \
    glm/detail/func_packing.hpp \
    glm/detail/func_trigonometric.hpp \
    glm/detail/func_vector_relational.hpp \
    glm/detail/intrinsic_common.hpp \
    glm/detail/intrinsic_exponential.hpp \
    glm/detail/intrinsic_geometric.hpp \
    glm/detail/intrinsic_integer.hpp \
    glm/detail/intrinsic_matrix.hpp \
    glm/detail/intrinsic_trigonometric.hpp \
    glm/detail/intrinsic_vector_relational.hpp \
    glm/detail/precision.hpp \
    glm/detail/setup.hpp \
    glm/detail/type_float.hpp \
    glm/detail/type_gentype.hpp \
    glm/detail/type_half.hpp \
    glm/detail/type_int.hpp \
    glm/detail/type_mat.hpp \
    glm/detail/type_mat2x2.hpp \
    glm/detail/type_mat2x3.hpp \
    glm/detail/type_mat2x4.hpp \
    glm/detail/type_mat3x2.hpp \
    glm/detail/type_mat3x3.hpp \
    glm/detail/type_mat3x4.hpp \
    glm/detail/type_mat4x2.hpp \
    glm/detail/type_mat4x3.hpp \
    glm/detail/type_mat4x4.hpp \
    glm/detail/type_vec.hpp \
    glm/detail/type_vec1.hpp \
    glm/detail/type_vec2.hpp \
    glm/detail/type_vec3.hpp \
    glm/detail/type_vec4.hpp \
    glm/gtc/bitfield.hpp \
    glm/gtc/constants.hpp \
    glm/gtc/epsilon.hpp \
    glm/gtc/integer.hpp \
    glm/gtc/matrix_access.hpp \
    glm/gtc/matrix_integer.hpp \
    glm/gtc/matrix_inverse.hpp \
    glm/gtc/matrix_transform.hpp \
    glm/gtc/noise.hpp \
    glm/gtc/packing.hpp \
    glm/gtc/quaternion.hpp \
    glm/gtc/random.hpp \
    glm/gtc/reciprocal.hpp \
    glm/gtc/round.hpp \
    glm/gtc/type_precision.hpp \
    glm/gtc/type_ptr.hpp \
    glm/gtc/ulp.hpp \
    glm/gtc/vec1.hpp \
    glm/gtx/associated_min_max.hpp \
    glm/gtx/bit.hpp \
    glm/gtx/closest_point.hpp \
    glm/gtx/color_space.hpp \
    glm/gtx/color_space_YCoCg.hpp \
    glm/gtx/common.hpp \
    glm/gtx/compatibility.hpp \
    glm/gtx/component_wise.hpp \
    glm/gtx/dual_quaternion.hpp \
    glm/gtx/euler_angles.hpp \
    glm/gtx/extend.hpp \
    glm/gtx/extented_min_max.hpp \
    glm/gtx/fast_exponential.hpp \
    glm/gtx/fast_square_root.hpp \
    glm/gtx/fast_trigonometry.hpp \
    glm/gtx/gradient_paint.hpp \
    glm/gtx/handed_coordinate_space.hpp \
    glm/gtx/integer.hpp \
    glm/gtx/intersect.hpp \
    glm/gtx/io.hpp \
    glm/gtx/log_base.hpp \
    glm/gtx/matrix_cross_product.hpp \
    glm/gtx/matrix_decompose.hpp \
    glm/gtx/matrix_interpolation.hpp \
    glm/gtx/matrix_major_storage.hpp \
    glm/gtx/matrix_operation.hpp \
    glm/gtx/matrix_query.hpp \
    glm/gtx/matrix_transform_2d.hpp \
    glm/gtx/mixed_product.hpp \
    glm/gtx/multiple.hpp \
    glm/gtx/norm.hpp \
    glm/gtx/normal.hpp \
    glm/gtx/normalize_dot.hpp \
    glm/gtx/number_precision.hpp \
    glm/gtx/optimum_pow.hpp \
    glm/gtx/orthonormalize.hpp \
    glm/gtx/perpendicular.hpp \
    glm/gtx/polar_coordinates.hpp \
    glm/gtx/projection.hpp \
    glm/gtx/quaternion.hpp \
    glm/gtx/range.hpp \
    glm/gtx/raw_data.hpp \
    glm/gtx/rotate_normalized_axis.hpp \
    glm/gtx/rotate_vector.hpp \
    glm/gtx/scalar_multiplication.hpp \
    glm/gtx/scalar_relational.hpp \
    glm/gtx/simd_mat4.hpp \
    glm/gtx/simd_quat.hpp \
    glm/gtx/simd_vec4.hpp \
    glm/gtx/spline.hpp \
    glm/gtx/std_based_type.hpp \
    glm/gtx/string_cast.hpp \
    glm/gtx/transform.hpp \
    glm/gtx/transform2.hpp \
    glm/gtx/type_aligned.hpp \
    glm/gtx/vector_angle.hpp \
    glm/gtx/vector_query.hpp \
    glm/gtx/wrap.hpp \
    glm/common.hpp \
    glm/exponential.hpp \
    glm/ext.hpp \
    glm/fwd.hpp \
    glm/geometric.hpp \
    glm/glm.hpp \
    glm/integer.hpp \
    glm/mat2x2.hpp \
    glm/mat2x3.hpp \
    glm/mat2x4.hpp \
    glm/mat3x2.hpp \
    glm/mat3x3.hpp \
    glm/mat3x4.hpp \
    glm/mat4x2.hpp \
    glm/mat4x3.hpp \
    glm/mat4x4.hpp \
    glm/matrix.hpp \
    glm/packing.hpp \
    glm/trigonometric.hpp \
    glm/vec2.hpp \
    glm/vec3.hpp \
    glm/vec4.hpp \
    glm/vector_relational.hpp \
    libnoise/win32/resource.h \
    utility.h \
    utilities/openglprogramshader.h \
    utilities/openglbuffer.h \
    utilities/openglvertexarrayobject.h \
    utilities/time.h \
    utilities/opengltexture.h
