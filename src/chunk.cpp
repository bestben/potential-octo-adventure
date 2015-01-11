#include "chunk.h"

VoxelTextureMap VoxelTextures[(uint)VoxelType::COUNT];

void initializeTextureMaps(){
    // On ne devrait jamais demander la texture de l'air
    VoxelTextures[(uint)VoxelType::AIR] = { TextureID::ERROR_TEXTURE, TextureID::ERROR_TEXTURE, TextureID::ERROR_TEXTURE, TextureID::ERROR_TEXTURE, TextureID::ERROR_TEXTURE, TextureID::ERROR_TEXTURE};

    VoxelTextures[(uint)VoxelType::GRASS] = {
        TextureID::GRASS,
        TextureID::DIRT,
        TextureID::GRASS_SIDE,
        TextureID::GRASS_SIDE,
        TextureID::GRASS_SIDE,
        TextureID::GRASS_SIDE
    };

    FULL_BLOCK(DIRT)
    FULL_BLOCK(ROCK)
    FULL_BLOCK(SAND)
    FULL_BLOCK(GRAVEL)
    FULL_BLOCK(LAVA)
    FULL_BLOCK(WATER)
    FULL_BLOCK(LEAVES)

    VoxelTextures[(uint)VoxelType::STONE] = {
        TextureID::STONE_TOP,
        TextureID::STONE_TOP,
        TextureID::STONE_SIDE,
        TextureID::STONE_SIDE,
        TextureID::STONE_SIDE,
        TextureID::STONE_SIDE
    };

    VoxelTextures[(uint)VoxelType::TRUNK] = {
        TextureID::TRUNK_TOP,
        TextureID::TRUNK_TOP,
        TextureID::TRUNK_SIDE,
        TextureID::TRUNK_SIDE,
        TextureID::TRUNK_SIDE,
        TextureID::TRUNK_SIDE
    };
}

TextureID getTexture(VoxelType type, int side) {
    side = side % 6;
    TextureID* voxel = (TextureID*)&VoxelTextures[(uint)type];

    return *(voxel + side);
}

VoxelTextureMap getTextureMap(VoxelType type) {
    return VoxelTextures[(uint)type];
}
