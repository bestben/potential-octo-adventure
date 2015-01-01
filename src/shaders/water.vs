#version 330

layout(location = 0) in int position;

uniform vec3 chunkPosition;
uniform mat4 viewProj;

out vec3 ex_pos;
flat out int ex_voxel;
out vec3 ex_normal;
out vec4 view_pos;
out vec3 voxel_pos;

uniform vec3 normals[6] = {
    vec3( -1, 0, 0 ),		// -X
	vec3( 1, 0, 0 ),		// +X
	vec3( 0, -1, 0 ),		// -Y
	vec3( 0, 1, 0 ),		// +Y
	vec3( 0, 0, -1 ),	    // -Z
	vec3( 0, 0, 1 )		    // +Z
};

void main() {
    int normalIndex = ((position & 0x03800000) >> 23);
    ex_voxel = ((position & 0x007F8000) >> 15);
    int posX = ((position & 0x00007C00) >> 10);
    int posY = ((position & 0x000003E0) >> 5);
    int posZ = (position & 0x0000001F);

    int i = int(posX);
    int j = int(posY);
    int k = int(posZ);

    voxel_pos = vec3(posX, posY, posZ);
    		
    vec3 tempPosition = chunkPosition + vec3(posX, posY, posZ);
    vec4 absolutePosition = vec4(tempPosition.x, tempPosition.y, tempPosition.z, 1.0);

    ex_pos = absolutePosition.xyz;
    ex_normal = normals[normalIndex];
    
    view_pos = viewProj * absolutePosition;
    gl_Position = viewProj * (absolutePosition + vec4(0.0, -0.2, 0.0, 0.0));
}