#include "meshgenerator.h"

MeshGenerator::MeshGenerator()
{

}

MeshGenerator::~MeshGenerator()
{

}

int MeshGenerator::generate(Voxel* data, GLuint* vertices) {
    int* mask = new int[CHUNK_SIZE * CHUNK_SIZE];
    int vertexCount = 0;

    for (int axis = 0; axis < 3; ++axis) {
        const int u = (axis + 1) % 3;
        const int v = (axis + 2) % 3;

        int x[3] = {0}, q[3] = {0};
        memset(mask, 0, CHUNK_SIZE * CHUNK_SIZE * sizeof(int));

        // Calcule du mask
        q[axis] = 1;
        for (x[axis] = -1; x[axis] < CHUNK_SIZE;) {
            int counter = 0;
            for (x[v] = 0; x[v] < CHUNK_SIZE; ++x[v]) {
                for (x[u] = 0; x[u] < CHUNK_SIZE; ++x[u], ++counter)
                {
                    const int a = 0 <= x[axis] ? getVoxel(data, x[0], x[1], x[2]) : 0;
                    const int b = x[axis] < CHUNK_SIZE - 1 ? getVoxel(data, x[0] + q[0],
                            x[1] + q[1],
                            x[2] + q[2]) : 0;
                    const bool ba = static_cast<bool>(a);
                    if (ba == static_cast<bool>(b))
                        mask[counter] = 0;
                    else if (ba)
                        mask[counter] = a;
                    else
                        mask[counter] = -b;
                }
            }
            ++x[axis];

            int width = 0, height = 0;
            counter = 0;
            for (int j = 0; j < CHUNK_SIZE; ++j) {
                for (int i = 0; i < CHUNK_SIZE;) {
                    int c = mask[counter];
                    if (c) {
                        // Calcule de la largeur
                        for (width = 1; c == mask[counter + width] &&
                             i + width < CHUNK_SIZE; ++width) {
                        }

                        // Calcule de la hauteur
                        bool done = false;
                        for (height = 1; j + height < CHUNK_SIZE; ++height) {
                            for (int k = 0; k < width; ++k)
                                if (c != mask[counter + k + height * CHUNK_SIZE]) {
                                    done = true;
                                    break;
                                }
                            if (done)
                                break;
                        }

                        // Ajout d'une face
                        x[u] = i;
                        x[v] = j;

                        int du[3] = {0}, dv[3] = {0};

                        int nIndex = axis * 2;
                        if (c > 0) {
                            dv[v] = height;
                            du[u] = width;
                            nIndex++;
                        } else {
                            c = -c;
                            du[v] = height;
                            dv[u] = width;
                        }

                        vertices[vertexCount++] = getVertex(x[0], x[1], x[2], nIndex, c);
                        vertices[vertexCount++] = getVertex(x[0] + dv[0], x[1] + dv[1], x[2] + dv[2], nIndex, c);
                        vertices[vertexCount++] = getVertex(x[0] + du[0] + dv[0], x[1] + du[1] + dv[1], x[2] + du[2] + dv[2], nIndex, c);
                        vertices[vertexCount++] = getVertex(x[0] + du[0] + dv[0], x[1] + du[1] + dv[1], x[2] + du[2] + dv[2], nIndex, c);
                        vertices[vertexCount++] = getVertex(x[0] + du[0], x[1] + du[1], x[2] + du[2], nIndex, c);
                        vertices[vertexCount++] = getVertex(x[0], x[1], x[2], nIndex, c);

                        for (int b = 0; b < width; ++b)
                            for (int a = 0; a < height; ++a)
                                mask[counter + b + a * CHUNK_SIZE] = 0;

                        i += width; counter += width;
                    } else {
                        ++i;
                        ++counter;
                    }
                }
            }
        }
    }
    delete[] mask;
    return vertexCount;
}

GLuint MeshGenerator::getVoxel(Voxel* data, int i, int j, int k) {
    return data[i + CHUNK_SIZE * (j + CHUNK_SIZE * k)];
}

GLuint MeshGenerator::getVertex(int x, int y, int z, int normalIndex, int voxel) {
    return ((normalIndex << 23) | (voxel & 0x000000FF) << 15) | (x << 10) | (y << 5) | (z);
}
