#version 330

uniform float xSize;
uniform float ySize;

out vec2 ex_textCoord;

void main() {

    int id = gl_VertexID;
    if (gl_VertexID == 4) {
        id = 2;
    } else if (gl_VertexID == 5) {
        id = 1;
    }
    int posX = (id >> 1) & 1;
    int posY = (id >> 0) & 1;

    ex_textCoord = vec2(posX, posY);
    		
    vec2 pos = vec2(posX * xSize - xSize / 2, posY * ySize - ySize / 2);
    
    gl_Position = vec4(pos, 0.0, 1.0);
}