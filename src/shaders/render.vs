#version 330

layout(location = 0) in uint position;

uniform vec3 chunkPosition;
uniform mat4 viewProj;

out vec3 ex_pos;
flat out int ex_voxel;
out vec3 ex_normal;

out vec3 voxel_pos;

out mat4 normal_mat;
out vec3 ex_normal_eye;
out vec4 view_pos;


uniform vec3 normals[6] = {
    vec3( -1, 0, 0 ),		// -X
	vec3( 1, 0, 0 ),		// +X
	vec3( 0, -1, 0 ),		// -Y
	vec3( 0, 1, 0 ),		// +Y
	vec3( 0, 0, -1 ),	    // -Z
	vec3( 0, 0, 1 )		    // +Z
};


void main() {
    //uint light = ((position & uint(0xFC000000) )>> 26);
    
    uint normalIndex = ((position & uint(0x03800000)) >> 23);
    ex_voxel = int((position & uint(0x007F8000)) >> 15);
    uint posX = ((position & uint(0x00007C00)) >> 10);
    uint posY = ((position & uint(0x000003E0)) >> 5);
    uint posZ = (position & uint(0x0000001F));
    
    int i = int(posX);
    int j = int(posY);
    int k = int(posZ);

    voxel_pos = vec3(posX, posY, posZ);
    
    vec3 tempPosition = chunkPosition + vec3(posX, posY, posZ);
    vec4 absolutePosition = vec4(tempPosition.x, tempPosition.y, tempPosition.z, 1.0);

    ex_pos = absolutePosition.xyz;
    ex_normal = normals[normalIndex];
    view_pos = viewProj * absolutePosition;


    gl_Position = view_pos;
}