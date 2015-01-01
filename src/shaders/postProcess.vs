#version 330

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
    
    gl_Position = vec4(posX * 2.0 - 1.0, posY * 2.0 - 1.0, 0.0, 1.0);
}
