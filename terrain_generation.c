#include "terrain_generation.h"
#include "noise.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>

// 获取默认地形生成器配置
TerrainGeneratorConfig getDefaultTerrainConfig() {
    TerrainGeneratorConfig config;
    config.scale = 5.0f;
    config.seed = (int)time(NULL);
    config.octaves = 6;
    config.persistence = 0.5f;
    config.lacunarity = 2.0f;
    config.heightMultiplier = 4.0f;
    config.heightOffset = 2.0f;  // 进一步增加高度偏移，大幅减少水域面积
    return config;
}

// 使用柏林噪声生成地形
void generateTerrainWithPerlinNoise(int width, int height, Terrain** terrainMap, TerrainGeneratorConfig config) {
    // 初始化噪声种子
    initNoiseSeed(config.seed);
    
    // 生成柏林噪声地形
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // 计算噪声值
            float nx = (float)x / width * config.scale;
            float ny = (float)y / height * config.scale;
            
            // 使用柏林噪声生成地形高度
            float noiseValue = perlinNoise(nx, ny, config.seed);
            
            // 将噪声值映射到地形高度范围
            float terrainHeight = (noiseValue * 2 - 1) * config.heightMultiplier + config.heightOffset;
            
            // 限制高度范围
            if (terrainHeight < TERRAIN_MIN_HEIGHT) terrainHeight = TERRAIN_MIN_HEIGHT;
            if (terrainHeight > TERRAIN_MAX_HEIGHT) terrainHeight = TERRAIN_MAX_HEIGHT;
            
            // 保存高度值和确定地形类型
            terrainMap[y][x].height = terrainHeight;
            terrainMap[y][x].type = getTerrainTypeFromHeight(terrainHeight);
            
            // 初始化其他地形属性
            terrainMap[y][x].flags = TERRAIN_FLAG_NONE;
            terrainMap[y][x].moisture = 0.5f;    // 默认中等水分
            terrainMap[y][x].temperature = 0.5f; // 默认中等温度
        }
    }
}

// 使用分形噪声生成地形
void generateTerrainWithFractalNoise(int width, int height, Terrain** terrainMap, TerrainGeneratorConfig config) {
    // 使用分形布朗运动噪声生成地形
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // 计算噪声坐标
            float nx = (float)x / width * config.scale;
            float ny = (float)y / height * config.scale;
            
            // 使用分形布朗运动生成噪声
            float noiseValue = fractalBrownianMotion(
                nx, ny, 
                config.octaves, 
                config.persistence, 
                config.lacunarity, 
                config.seed
            );
            
            // 将噪声值映射到地形高度范围
            float terrainHeight = (noiseValue * 2 - 1) * config.heightMultiplier + config.heightOffset;
            
            // 限制高度范围
            if (terrainHeight < TERRAIN_MIN_HEIGHT) terrainHeight = TERRAIN_MIN_HEIGHT;
            if (terrainHeight > TERRAIN_MAX_HEIGHT) terrainHeight = TERRAIN_MAX_HEIGHT;
            
            // 保存高度值和确定地形类型
            terrainMap[y][x].height = terrainHeight;
            terrainMap[y][x].type = getTerrainTypeFromHeight(terrainHeight);
            
            // 初始化其他地形属性
            terrainMap[y][x].flags = TERRAIN_FLAG_NONE;
            terrainMap[y][x].moisture = 0.5f;
            terrainMap[y][x].temperature = 0.5f;
        }
    }
}

// 使用域变形噪声生成地形
void generateTerrainWithDomainWarping(int width, int height, Terrain** terrainMap, TerrainGeneratorConfig config) {
    float warpStrength = 1.0f; // 域扭曲强度
    
    // 使用域扭曲噪声生成地形
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // 计算噪声坐标
            float nx = (float)x / width * config.scale;
            float ny = (float)y / height * config.scale;
            
            // 使用域扭曲噪声
            float noiseValue = domainWarpingNoise(nx, ny, warpStrength, config.seed);
            
            // 将噪声值映射到地形高度范围
            float terrainHeight = (noiseValue * 2 - 1) * config.heightMultiplier + config.heightOffset;
            
            // 限制高度范围
            if (terrainHeight < TERRAIN_MIN_HEIGHT) terrainHeight = TERRAIN_MIN_HEIGHT;
            if (terrainHeight > TERRAIN_MAX_HEIGHT) terrainHeight = TERRAIN_MAX_HEIGHT;
            
            // 保存高度值和确定地形类型
            terrainMap[y][x].height = terrainHeight;
            terrainMap[y][x].type = getTerrainTypeFromHeight(terrainHeight);
            
            // 初始化其他地形属性
            terrainMap[y][x].flags = TERRAIN_FLAG_NONE;
            terrainMap[y][x].moisture = 0.5f;
            terrainMap[y][x].temperature = 0.5f;
        }
    }
}

// 使用山脊多重分形噪声生成地形
void generateTerrainWithRidgedMulti(int width, int height, Terrain** terrainMap, TerrainGeneratorConfig config) {
    // 使用山脊多重分形噪声生成地形
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // 计算噪声坐标
            float nx = (float)x / width * config.scale;
            float ny = (float)y / height * config.scale;
            
            // 使用山脊多重分形噪声
            float noiseValue = ridgedMultiNoise(
                nx, ny, 
                config.octaves, 
                config.persistence, 
                config.lacunarity, 
                config.seed
            );
            
            // 将噪声值映射到地形高度范围
            float terrainHeight = (noiseValue * 2 - 1) * config.heightMultiplier + config.heightOffset;
            
            // 限制高度范围
            if (terrainHeight < TERRAIN_MIN_HEIGHT) terrainHeight = TERRAIN_MIN_HEIGHT;
            if (terrainHeight > TERRAIN_MAX_HEIGHT) terrainHeight = TERRAIN_MAX_HEIGHT;
            
            // 保存高度值和确定地形类型
            terrainMap[y][x].height = terrainHeight;
            terrainMap[y][x].type = getTerrainTypeFromHeight(terrainHeight);
            
            // 初始化其他地形属性
            terrainMap[y][x].flags = TERRAIN_FLAG_NONE;
            terrainMap[y][x].moisture = 0.5f;
            terrainMap[y][x].temperature = 0.5f;
        }
    }
}

// 按照类型选择地形生成算法
void generateTerrainWithAlgorithm(int width, int height, Terrain** terrainMap, NoiseAlgorithmType algorithmType, TerrainGeneratorConfig config) {
    switch (algorithmType) {
        case NOISE_PERLIN:
            generateTerrainWithPerlinNoise(width, height, terrainMap, config);
            break;
        case NOISE_FRACTAL_BROWNIAN:
            generateTerrainWithFractalNoise(width, height, terrainMap, config);
            break;
        case NOISE_DOMAIN_WARPING:
            generateTerrainWithDomainWarping(width, height, terrainMap, config);
            break;
        case NOISE_RIDGED_MULTI:
            generateTerrainWithRidgedMulti(width, height, terrainMap, config);
            break;
        case NOISE_SIMPLEX:
            // 简化实现，使用柏林噪声但以单纯形噪声函数替代
            generateTerrainWithPerlinNoise(width, height, terrainMap, config);
            break;
        default:
            // 默认使用分形噪声
            generateTerrainWithFractalNoise(width, height, terrainMap, config);
            break;
    }
    
    // 限制水域面积
    limitWaterAreas(width, height, terrainMap);
}

// 限制水域面积，减少大面积水域的生成
void limitWaterAreas(int width, int height, Terrain** terrainMap) {
    // 计算水域数量
    int waterCount = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (terrainMap[y][x].type == TERRAIN_WATER) {
                waterCount++;
            }
        }
    }
    
    // 计算水域占比
    float waterRatio = (float)waterCount / (width * height);
    
    // 如果水域面积过大（超过10%），逐步提升水域单元格的高度
    if (waterRatio > 0.1f) {
        // 随机选择一些水域单元格提升高度
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (terrainMap[y][x].type == TERRAIN_WATER) {
                    // 根据当前水域占比，决定提升高度的概率
                    float raiseChance = (waterRatio - 0.1f) / 0.5f + 0.5f; // 提高转换概率
                    
                    if ((float)rand() / RAND_MAX < raiseChance) {
                        // 提升高度到平地水平
                        terrainMap[y][x].height = 0.0f;
                        terrainMap[y][x].type = TERRAIN_FLAT;
                    }
                }
            }
        }
    }
    
    // 消除孤立的水域单元格，使水域更连贯
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            if (terrainMap[y][x].type == TERRAIN_WATER) {
                // 检查是否为孤立水域（周围8个格子中水域少于3个）
                int waterNeighbors = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        if (terrainMap[y + dy][x + dx].type == TERRAIN_WATER) {
                            waterNeighbors++;
                        }
                    }
                }
                
                // 如果是孤立水域，转换为平地
                if (waterNeighbors < 3) {
                    terrainMap[y][x].height = 0.0f;
                    terrainMap[y][x].type = TERRAIN_FLAT;
                }
            }
        }
    }
}

// 生成河流通道
void generateRivers(int width, int height, Terrain** terrainMap, int riverCount, int minLength, int maxLength) {
    // 找到可能的河流起点（高地区域）
    int maxPossibleStartPoints = width * height / 10; // 最多考虑10%的单元格作为起点
    int* startPointsX = (int*)malloc(maxPossibleStartPoints * sizeof(int));
    int* startPointsY = (int*)malloc(maxPossibleStartPoints * sizeof(int));
    int startPointCount = 0;
    
    // 寻找高于平均高度的位置作为河流起点
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (terrainMap[y][x].height > 2.0f && startPointCount < maxPossibleStartPoints) {
                startPointsX[startPointCount] = x;
                startPointsY[startPointCount] = y;
                startPointCount++;
            }
        }
    }
    
    if (startPointCount == 0) {
        free(startPointsX);
        free(startPointsY);
        return; // 没有合适的起点
    }
    
    // 确定要生成的河流数量（不能超过可用起点数）
    int actualRiverCount = (riverCount < startPointCount) ? riverCount : startPointCount;
    
    // 为每条河流选择一个起点并生成河流
    for (int r = 0; r < actualRiverCount; r++) {
        // 随机选择一个起点
        int startIndex = rand() % startPointCount;
        int x = startPointsX[startIndex];
        int y = startPointsY[startIndex];
        
        // 替换已使用的起点（避免重复）
        startPointsX[startIndex] = startPointsX[--startPointCount];
        startPointsY[startIndex] = startPointsY[startPointCount];
        
        // 确定河流长度
        int riverLength = minLength + rand() % (maxLength - minLength + 1);
        
        // 生成河流路径
        for (int step = 0; step < riverLength; step++) {
            // 标记当前单元格为河流
            if (x >= 0 && x < width && y >= 0 && y < height) {
                terrainMap[y][x].flags |= TERRAIN_FLAG_RIVER;
                terrainMap[y][x].moisture = 1.0f; // 河流水分最大
                
                // 如果达到水域或地图边缘，结束河流
                if (terrainMap[y][x].height < 0 || x == 0 || y == 0 || x == width-1 || y == height-1) {
                    break;
                }
            } else {
                break; // 超出地图范围
            }
            
            // 寻找下一个最低点作为河流流向
            float lowestHeight = terrainMap[y][x].height;
            int nextX = x;
            int nextY = y;
            
            // 检查八个方向的邻居
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;
                    
                    int nx = x + dx;
                    int ny = y + dy;
                    
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        if (terrainMap[ny][nx].height < lowestHeight) {
                            lowestHeight = terrainMap[ny][nx].height;
                            nextX = nx;
                            nextY = ny;
                        }
                    }
                }
            }
            
            // 如果找不到更低的点，河流结束
            if (nextX == x && nextY == y) {
                break;
            }
            
            // 移动到下一个点
            x = nextX;
            y = nextY;
        }
    }
    
    free(startPointsX);
    free(startPointsY);
}

// 平滑地形
void smoothTerrain(int width, int height, Terrain** terrainMap, int iterations) {
    // 创建临时地形数组存储平滑过程中的中间值
    Terrain** tempTerrainMap = (Terrain**)malloc(height * sizeof(Terrain*));
    for (int i = 0; i < height; i++) {
        tempTerrainMap[i] = (Terrain*)malloc(width * sizeof(Terrain));
    }
    
    // 复制初始地形到临时数组
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            tempTerrainMap[y][x] = terrainMap[y][x];
        }
    }
    
    // 执行多次平滑迭代
    for (int iter = 0; iter < iterations; iter++) {
        // 对每个单元格进行平滑处理
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float totalHeight = 0.0f;
                int neighbors = 0;
                
                // 计算周围8个邻居的平均高度
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int nx = x + dx;
                        int ny = y + dy;
                        
                        if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                            totalHeight += terrainMap[ny][nx].height;
                            neighbors++;
                        }
                    }
                }
                
                // 计算平均高度并应用到临时数组
                if (neighbors > 0) {
                    float avgHeight = totalHeight / neighbors;
                    tempTerrainMap[y][x].height = avgHeight;
                    tempTerrainMap[y][x].type = getTerrainTypeFromHeight(avgHeight);
                }
            }
        }
        
        // 将临时数组复制回主地形
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                terrainMap[y][x].height = tempTerrainMap[y][x].height;
                terrainMap[y][x].type = tempTerrainMap[y][x].type;
            }
        }
    }
    
    // 释放临时内存
    for (int i = 0; i < height; i++) {
        free(tempTerrainMap[i]);
    }
    free(tempTerrainMap);
}

// 生成陡峭悬崖
void generateCliffs(int width, int height, Terrain** terrainMap, float threshold) {
    // 寻找高度差超过阈值的区域
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            float currentHeight = terrainMap[y][x].height;
            
            // 检查相邻单元格的高度差
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;
                    
                    int nx = x + dx;
                    int ny = y + dy;
                    float neighborHeight = terrainMap[ny][nx].height;
                    
                    // 如果高度差超过阈值，标记为悬崖
                    if (fabs(currentHeight - neighborHeight) > threshold) {
                        terrainMap[y][x].flags |= TERRAIN_FLAG_CLIFF;
                        break;
                    }
                }
                
                // 如果已经标记为悬崖，跳出循环
                if (terrainMap[y][x].flags & TERRAIN_FLAG_CLIFF) {
                    break;
                }
            }
        }
    }
} 