#pragma once

#define CHUNK_NUMBER 700
#define VBO_NUMBER 700

#define CHUNK_SIZE 31
#define CHUNK_SCALE 5

#define VIEW_SIZE 4

using Voxel = unsigned char;

struct Chunk {
    // Les coordonées du chunk
    int i;
    int j;
    int k;

    bool visible;
    float distanceFromCamera;

    int chunkBufferIndex;
    int vboIndex;
};

