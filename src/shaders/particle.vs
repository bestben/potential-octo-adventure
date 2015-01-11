#version 330

in vec3 position;

uniform int voxelType;

flat out int ex_voxelType;

uniform mat4 mvp;

void main() {
    ex_voxelType = voxelType;
    gl_Position = mvp * vec4(position, 1.0);
    gl_PointSize = 100.0;
}