#version 330

in uint position;

uniform vec3 chunkPosition;
uniform mat4 viewProj;

out vec3 ex_pos;
out flat uint ex_voxel;
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
    uint normalIndex = ((position & 0x03800000) >> 23);
    ex_voxel = ((position & 0x007F8000) >> 15);
    uint posX = ((position & 0x00007C00) >> 10);
    uint posY = ((position & 0x000003E0) >> 5);
    uint posZ = (position & 0x0000001F);
    
    vec3 tp = vec3(posX, posY, posZ);
    
    vec3 tempPosition = chunkPosition + tp;
    vec4 absolutePosition = vec4(tempPosition.x, tempPosition.y, tempPosition.z, 1.0);

    ex_pos = absolutePosition.xyz;
    ex_normal = normals[normalIndex];
        
    gl_Position = viewProj * absolutePosition;
}