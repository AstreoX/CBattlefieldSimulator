#include "terrain.h"
#include "noise.h"
#include "terrain_generation.h"
#include "terrain_renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <windows.h>

// 柏林噪声算法的辅助函数
// 平滑插值
float terrain_lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// 生成随机梯度向量
float terrain_grad(int hash, float x, float y) {
    int h = hash & 15;
    float grad = 1.0f + (h & 7);  // 梯度值 1-8
    if ((h & 8) != 0) grad = -grad;  // 一半负值
    return (grad * x + grad * y);   // 梯度 * 距离向量
}

// 伪随机哈希函数
int terrain_hash(int x, int y, int seed) {
    int hash = x + y * 137 + seed * 13;
    hash = (hash << 13) ^ hash;
    return ((hash * (hash * hash * 15731 + 789221) + 1376312589) & 0x7fffffff);
}

// 改进的噪声函数，简化版柏林噪声
float noise(float x, float y, int seed) {
    int x0 = (int)floor(x);
    int y0 = (int)floor(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;
    
    float sx = x - (float)x0;
    float sy = y - (float)y0;
    
    float n0 = (float)terrain_hash(x0, y0, seed) / 0x7fffffff;
    float n1 = (float)terrain_hash(x1, y0, seed) / 0x7fffffff;
    float n2 = (float)terrain_hash(x0, y1, seed) / 0x7fffffff;
    float n3 = (float)terrain_hash(x1, y1, seed) / 0x7fffffff;
    
    // 使用平滑插值
    float ix0 = terrain_lerp(n0, n1, sx);
    float ix1 = terrain_lerp(n2, n3, sx);
    return terrain_lerp(ix0, ix1, sy) * 2 - 1; // 范围 -1 到 1
}

// 多重八度噪声函数（分形噪声）
float fractalNoise(float x, float y, int octaves, float persistence, float lacunarity, int seed) {
    float total = 0;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0;  // 用于正规化
    
    for (int i = 0; i < octaves; i++) {
        total += noise(x * frequency, y * frequency, seed + i) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return total / maxValue;  // 正规化为 -1 到 1
}

// 初始化地形系统
void initTerrain(int width, int height, Terrain** terrainMap) {
    // 分配每行的内存
    for (int i = 0; i < height; i++) {
        terrainMap[i] = (Terrain*)malloc(width * sizeof(Terrain));
        
        // 初始化每个格子的地形
        for (int j = 0; j < width; j++) {
            terrainMap[i][j].height = 0.0f;
            terrainMap[i][j].type = TERRAIN_FLAT;
            terrainMap[i][j].flags = TERRAIN_FLAG_NONE;
            terrainMap[i][j].moisture = 0.5f;    // 默认中等水分
            terrainMap[i][j].temperature = 0.5f; // 默认中等温度
        }
    }
}

// 释放地形系统资源
void freeTerrain(int height, Terrain** terrainMap) {
    for (int i = 0; i < height; i++) {
        free(terrainMap[i]);
    }
}

// 使用噪声算法生成地形
void generateTerrain(int width, int height, Terrain** terrainMap, float scale, int seed) {
    // 创建默认地形生成配置
    TerrainGeneratorConfig config = getDefaultTerrainConfig();
    
    // 应用自定义参数
    config.scale = scale;
    config.seed = seed;
    
    // 使用默认算法（分形布朗运动）生成地形
    generateTerrainWithAlgorithm(width, height, terrainMap, NOISE_FRACTAL_BROWNIAN, config);
    
    // 平滑地形，使水域和地形过渡更自然
    smoothTerrain(width, height, terrainMap, 2);
}

// 根据高度值获取地形类型
TerrainType getTerrainTypeFromHeight(float height) {
    if (height < -1.0f) return TERRAIN_WATER;  // 进一步降低水域阈值，使水域更加稀少
    if (height < 0.5f) return TERRAIN_FLAT;
    if (height < 1.5f) return TERRAIN_HILL_1;
    if (height < 2.5f) return TERRAIN_HILL_2;
    if (height < 3.5f) return TERRAIN_HILL_3;
    if (height < 4.5f) return TERRAIN_MOUNTAIN_1;
    return TERRAIN_MOUNTAIN_2;
}

// 获取地形对应的显示字符
char* getTerrainChar(TerrainType type) {
    // 对所有地形类型都返回相同的方块字符，用颜色来区分
    return "█";
}

// 获取地形对应的显示颜色
int terrain_getTerrainColor(TerrainType type) {
    // 使用地形渲染器中的颜色配置
    TerrainColorConfig config = getDefaultColorConfig();
    
    switch (type) {
        case TERRAIN_WATER:      return config.waterColors[0];
        case TERRAIN_FLAT:       return config.flatColor;
        case TERRAIN_HILL_1:     return config.hillColors[0];
        case TERRAIN_HILL_2:     return config.hillColors[1];
        case TERRAIN_HILL_3:     return config.hillColors[2];
        case TERRAIN_MOUNTAIN_1: return config.mountainColors[0];
        case TERRAIN_MOUNTAIN_2: return config.mountainColors[1];
        default:                 return 15; // 白色
    }
}

// 获取默认地形显示配置
TerrainDisplayConfig getDefaultDisplayConfig() {
    TerrainDisplayConfig config;
    config.showGrid = 0;
    config.useColorGradient = 1;
    config.showFeatures = 1;
    config.symbolSize = 1;
    return config;
}

// 设置单个地形单元格的特性
void setTerrainFeature(Terrain* terrain, unsigned int feature) {
    terrain->flags |= feature;
}

// 移除单个地形单元格的特性
void removeTerrainFeature(Terrain* terrain, unsigned int feature) {
    terrain->flags &= ~feature;
}

// 检查地形单元格是否有指定特性
int hasTerrainFeature(Terrain* terrain, unsigned int feature) {
    return (terrain->flags & feature) != 0;
}

// 计算地形上两点间的行走难度
float calculateMovementDifficulty(Terrain* start, Terrain* end) {
    // 基础难度：高度差的绝对值 + 1
    float heightDiff = fabsf(end->height - start->height);
    float difficulty = heightDiff + 1.0f;
    
    // 水域显著增加难度
    if (end->type == TERRAIN_WATER) {
        difficulty *= 3.0f;
    }
    
    // 特性标志影响
    if (end->flags & TERRAIN_FLAG_FOREST)
        difficulty += 1.5f;
    if (end->flags & TERRAIN_FLAG_CLIFF)
        difficulty += 5.0f;
    if (end->flags & TERRAIN_FLAG_RIVER)
        difficulty += 2.0f;
    if (end->flags & TERRAIN_FLAG_ROAD)
        difficulty -= 0.5f; // 道路降低难度
        
    // 确保难度至少为1
    return difficulty < 1.0f ? 1.0f : difficulty;
}

// 获取地形描述文本
const char* getTerrainDescription(TerrainType type) {
    switch (type) {
        case TERRAIN_WATER:      return "水域";
        case TERRAIN_FLAT:       return "平地";
        case TERRAIN_HILL_1:     return "低丘陵";
        case TERRAIN_HILL_2:     return "中丘陵";
        case TERRAIN_HILL_3:     return "高丘陵";
        case TERRAIN_MOUNTAIN_1: return "低山";
        case TERRAIN_MOUNTAIN_2: return "高山";
        default:                 return "未知地形";
    }
}