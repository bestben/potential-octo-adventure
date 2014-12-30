#version 330

uniform mat4 viewProj;
uniform vec3 boxPosition;
uniform float boxScale;

void main() {
    int posX = (gl_VertexID >> 2) & 1;
    int posY = (gl_VertexID >> 1) & 1;
    int posZ = (gl_VertexID >> 0) & 1;
    		
    vec3 pos = vec3(posX, posY, posZ) * boxScale + boxPosition;
    
    gl_Position = viewProj * vec4(pos, 1.0);
}