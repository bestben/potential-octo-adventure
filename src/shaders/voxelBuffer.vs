#version 330

uniform mat4 viewProj;
uniform vec3 boxPosition;
uniform float boxWidth;
uniform float boxHeight;

in int normalIndex;
in int vertexIndex;

out vec3 ex_pos;
out vec3 ex_normal;

uniform vec3 normals[6] = {
    vec3( -1, 0, 0 ),		// -X
	vec3( 1, 0, 0 ),		// +X
	vec3( 0, -1, 0 ),		// -Y
	vec3( 0, 1, 0 ),		// +Y
	vec3( 0, 0, -1 ),	    // -Z
	vec3( 0, 0, 1 )		    // +Z
};

void main() {
    int posX = (vertexIndex >> 2) & 1;
    int posY = (vertexIndex >> 1) & 1;
    int posZ = (vertexIndex >> 0) & 1;
    		
    vec3 pos = vec3(posX, posY, posZ) * vec3(boxWidth, boxHeight, boxWidth) + boxPosition;
    
    ex_pos = vec3(posX, posY, posZ);
    ex_normal = normals[normalIndex];
    
    gl_Position = viewProj * vec4(pos, 1.0) - vec4(0.0, 0.0, 0.001, 0.0);
}