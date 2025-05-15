#ifndef TERRAIN_GENERATION_H
#define TERRAIN_GENERATION_H

#include "terrain.h"

// 地形生成器配置
typedef struct {
    float scale;            // 噪声比例
    int seed;               // 随机种子
    int octaves;            // 噪声叠加次数
    float persistence;      // 振幅衰减
    float lacunarity;       // 频率增益
    float heightMultiplier; // 高度缩放
    float heightOffset;     // 高度偏移
} TerrainGeneratorConfig;

// 噪声算法类型
typedef enum {
    NOISE_PERLIN,            // 柏林噪声
    NOISE_SIMPLEX,           // 单纯噪声
    NOISE_FRACTAL_BROWNIAN,  // 分形布朗运动
    NOISE_DOMAIN_WARPING,    // 域变形
    NOISE_RIDGED_MULTI       // 山脊多重分形
} NoiseAlgorithmType;

// 获取默认地形生成器配置
TerrainGeneratorConfig getDefaultTerrainConfig();

// 使用柏林噪声生成地形
void generateTerrainWithPerlinNoise(int width, int height, Terrain** terrainMap, TerrainGeneratorConfig config);

// 使用分形噪声生成地形
void generateTerrainWithFractalNoise(int width, int height, Terrain** terrainMap, TerrainGeneratorConfig config);

// 使用域变形噪声生成地形
void generateTerrainWithDomainWarping(int width, int height, Terrain** terrainMap, TerrainGeneratorConfig config);

// 使用山脊多重分形噪声生成地形
void generateTerrainWithRidgedMulti(int width, int height, Terrain** terrainMap, TerrainGeneratorConfig config);

// 生成河流通道
void generateRivers(int width, int height, Terrain** terrainMap, int riverCount, int minLength, int maxLength);

// 平滑地形
void smoothTerrain(int width, int height, Terrain** terrainMap, int iterations);

// 生成陡峭悬崖
void generateCliffs(int width, int height, Terrain** terrainMap, float threshold);

// 按照类型选择地形生成算法
void generateTerrainWithAlgorithm(int width, int height, Terrain** terrainMap, NoiseAlgorithmType algorithmType, TerrainGeneratorConfig config);

// 限制水域面积，减少大面积水域的生成
void limitWaterAreas(int width, int height, Terrain** terrainMap);

#endif // TERRAIN_GENERATION_H 