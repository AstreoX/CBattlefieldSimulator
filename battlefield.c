#include "battlefield.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_EQUIPMENTS_PER_TEAM 50
#define DEFAULT_BUDGET 10000

// 函数声明
void displayEquipmentInfo(Equipment* equipment);
static float noise2D(int x, int y, int seed);
static float smoothNoise2D(int x, int y, int seed);
static float interpolate(float a, float b, float x);
static float interpolatedNoise(float x, float y, int seed);
static float multiLayerNoise(float x, float y, int seed, const TerrainConfig* config);
static int checkTerrainConnectivity(Battlefield* battlefield, int x, int y, TerrainType type);
static int isSuitableForWater(Battlefield* battlefield, int x, int y);
static void generateSmallLake(Battlefield* battlefield, int centerX, int centerY);
static void optimizeTerrain(Battlefield* battlefield);
static int isInPlainArea(Battlefield* battlefield, int x, int y, int radius);
static float distance(int x1, int y1, int x2, int y2);
static int canBuildRoad(Battlefield* battlefield, int x1, int y1, int x2, int y2);
static void buildRoad(Battlefield* battlefield, int x1, int y1, int x2, int y2);
static void generateRoadNetwork(Battlefield* battlefield);
static int checkTerrainCoverage(Battlefield* battlefield);

// 噪声生成相关函数
static float noise2D(int x, int y, int seed) {
    int n = x + y * 57 + seed;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

static float smoothNoise2D(int x, int y, int seed) {
    float corners = (noise2D(x-1, y-1, seed) + noise2D(x+1, y-1, seed) + 
                    noise2D(x-1, y+1, seed) + noise2D(x+1, y+1, seed)) / 16;
    float sides = (noise2D(x-1, y, seed) + noise2D(x+1, y, seed) + 
                  noise2D(x, y-1, seed) + noise2D(x, y+1, seed)) / 8;
    float center = noise2D(x, y, seed) / 4;
    return corners + sides + center;
}

static float interpolate(float a, float b, float x) {
    float ft = x * 3.1415927f;
    float f = (1 - cosf(ft)) * 0.5f;
    return a * (1 - f) + b * f;
}

static float interpolatedNoise(float x, float y, int seed) {
    int intX = (int)x;
    int intY = (int)y;
    float fracX = x - intX;
    float fracY = y - intY;

    float v1 = smoothNoise2D(intX, intY, seed);
    float v2 = smoothNoise2D(intX + 1, intY, seed);
    float v3 = smoothNoise2D(intX, intY + 1, seed);
    float v4 = smoothNoise2D(intX + 1, intY + 1, seed);

    float i1 = interpolate(v1, v2, fracX);
    float i2 = interpolate(v3, v4, fracX);

    return interpolate(i1, i2, fracY);
}

// 初始化地形配置
void initTerrainConfig(TerrainConfig* config) {
    // 基础地形生成概率
    config->baseProbabilities.plain = 0.50f;    // 50%平原（减少）
    config->baseProbabilities.forest = 0.10f;   // 10%森林
    config->baseProbabilities.mountain = 0.35f; // 35%山地（增加）
    config->baseProbabilities.water = 0.10f;    // 10%水域
    config->baseProbabilities.road = 0.10f;     // 10%道路（增加）

    // 地形过渡参数
    config->transitionParams.plainThreshold = 5;     // 需要5个平原才转换
    config->transitionParams.mountainThreshold = 3;  // 降低山地转换阈值，使山地更容易形成
    config->transitionParams.forestThreshold = 4;    // 需要4个森林才转换
    config->transitionParams.transitionChance = 35;  // 增加转换概率到35%

    // 湖泊生成参数
    config->lakeParams.minDistance = 12;        // 湖泊最小间距
    config->lakeParams.lakeSize = 4;           // 湖泊大小(4x4)（增加）
    config->lakeParams.generationChance = 15;   // 15%生成概率
    config->lakeParams.plainCheckRadius = 2;    // 检查2格范围内的平原

    // 道路生成参数
    config->roadParams.nodeSpacing = 15;        // 道路节点间距
    config->roadParams.maxRoadLength = 20;      // 最大道路长度
    config->roadParams.extraRoadChance = 5;     // 额外道路生成概率
    config->roadParams.minPlainRadius = 3;      // 最小平原半径

    // 噪声生成参数
    config->noiseParams.baseFrequency = 0.1f;   // 基础频率
    config->noiseParams.noiseLayers = 4;        // 4层噪声
    config->noiseParams.persistence = 0.5f;     // 持续性
    config->noiseParams.lacunarity = 2.0f;      // 间隙度
}

// 多层噪声生成
static float multiLayerNoise(float x, float y, int seed, const TerrainConfig* config) {
    float noise = 0.0f;
    float amplitude = 1.0f;
    float frequency = config->noiseParams.baseFrequency;
    float maxValue = 0.0f;
    
    // 使用多层噪声来创建更自然的地形过渡
    for (int i = 0; i < config->noiseParams.noiseLayers; i++) {
        noise += interpolatedNoise(x * frequency, y * frequency, seed + i) * amplitude;
        maxValue += amplitude;
        amplitude *= config->noiseParams.persistence;
        frequency *= config->noiseParams.lacunarity;
    }
    
    return noise / maxValue;
}

// 检查地形连通性
static int checkTerrainConnectivity(Battlefield* battlefield, int x, int y, TerrainType type) {
    int count = 0;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            
            int nx = x + dx;
            int ny = y + dy;
            
            if (isPositionValid(battlefield, nx, ny)) {
                if (battlefield->cells[ny][nx].terrain == type) {
                    count++;
                }
            }
        }
    }
    return count;
}

// 检查是否适合生成水域
static int isSuitableForWater(Battlefield* battlefield, int x, int y) {
    const TerrainConfig* config = &battlefield->terrainConfig;
    int radius = config->lakeParams.plainCheckRadius;
    
    // 检查周围是否都是平原
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (isPositionValid(battlefield, nx, ny)) {
                if (battlefield->cells[ny][nx].terrain != TERRAIN_PLAIN) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

// 生成小湖泊
static void generateSmallLake(Battlefield* battlefield, int centerX, int centerY) {
    const TerrainConfig* config = &battlefield->terrainConfig;
    int size = config->lakeParams.lakeSize / 2;
    
    // 生成湖泊
    for (int dy = -size; dy <= size; dy++) {
        for (int dx = -size; dx <= size; dx++) {
            int nx = centerX + dx;
            int ny = centerY + dy;
            if (isPositionValid(battlefield, nx, ny)) {
                battlefield->cells[ny][nx].terrain = TERRAIN_WATER;
            }
        }
    }
}

// 优化地形生成
static void optimizeTerrain(Battlefield* battlefield) {
    // 水域优化：确保水域的连通性
    for (int i = 0; i < battlefield->height; i++) {
        for (int j = 0; j < battlefield->width; j++) {
            if (battlefield->cells[i][j].terrain == TERRAIN_WATER) {
                int waterNeighbors = checkTerrainConnectivity(battlefield, j, i, TERRAIN_WATER);
                if (waterNeighbors < 2) {
                    // 如果水域周围没有足够的水域，将其转换为平原
                    battlefield->cells[i][j].terrain = TERRAIN_PLAIN;
                }
            }
        }
    }

    // 山地优化：确保山地的聚集性
    for (int i = 0; i < battlefield->height; i++) {
        for (int j = 0; j < battlefield->width; j++) {
            if (battlefield->cells[i][j].terrain == TERRAIN_MOUNTAIN) {
                int mountainNeighbors = checkTerrainConnectivity(battlefield, j, i, TERRAIN_MOUNTAIN);
                if (mountainNeighbors < 1) {
                    // 如果山地周围没有其他山地，将其转换为平原
                    battlefield->cells[i][j].terrain = TERRAIN_PLAIN;
                }
            }
        }
    }

    // 森林优化：确保森林的连续性
    for (int i = 0; i < battlefield->height; i++) {
        for (int j = 0; j < battlefield->width; j++) {
            if (battlefield->cells[i][j].terrain == TERRAIN_FOREST) {
                int forestNeighbors = checkTerrainConnectivity(battlefield, j, i, TERRAIN_FOREST);
                if (forestNeighbors < 1) {
                    // 如果森林周围没有其他森林，将其转换为平原
                    battlefield->cells[i][j].terrain = TERRAIN_PLAIN;
                }
            }
        }
    }
}

// 获取地形颜色代码
const char* getTerrainColorCode(TerrainType terrain) {
    switch (terrain) {
        case TERRAIN_PLAIN:
            return "\033[42m";  // 绿色背景
        case TERRAIN_MOUNTAIN:
            return "\033[100m"; // 灰色背景
        case TERRAIN_FOREST:
            return "\033[32m";  // 绿色前景
        case TERRAIN_WATER:
            return "\033[44m";  // 蓝色背景
        case TERRAIN_ROAD:
            return "\033[43m";  // 黄色背景
        default:
            return "\033[0m";   // 默认颜色
    }
}

// 获取地形显示字符
char getTerrainChar(TerrainType terrain) {
    switch (terrain) {
        case TERRAIN_PLAIN:
            return '.';  // 平原用点表示
        case TERRAIN_MOUNTAIN:
            return '^';  // 山地用尖角表示
        case TERRAIN_FOREST:
            return '*';  // 森林用星号表示
        case TERRAIN_WATER:
            return '~';  // 水域用波浪表示
        case TERRAIN_ROAD:
            return '=';  // 道路用等号表示
        default:
            return ' ';
    }
}

// 检查是否在平原区域
static int isInPlainArea(Battlefield* battlefield, int x, int y, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int nx = x + dx;
            int ny = y + dy;
            if (isPositionValid(battlefield, nx, ny)) {
                if (battlefield->cells[ny][nx].terrain != TERRAIN_PLAIN) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

// 道路节点结构
typedef struct {
    int x, y;
    int connected;
} RoadNode;

// 计算两点间距离
static float distance(int x1, int y1, int x2, int y2) {
    return sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// 检查两点之间是否可以修建道路
static int canBuildRoad(Battlefield* battlefield, int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int steps = dx > dy ? dx : dy;
    
    if (steps == 0) return 0;
    
    float xIncrement = (float)(x2 - x1) / steps;
    float yIncrement = (float)(y2 - y1) / steps;
    
    float x = x1, y = y1;
    for (int i = 0; i < steps; i++) {
        x += xIncrement;
        y += yIncrement;
        
        int checkX = (int)(x + 0.5f);
        int checkY = (int)(y + 0.5f);
        
        if (isPositionValid(battlefield, checkX, checkY)) {
            TerrainType terrain = battlefield->cells[checkY][checkX].terrain;
            if (terrain != TERRAIN_PLAIN && terrain != TERRAIN_ROAD) {
                return 0;
            }
        }
    }
    return 1;
}

// 检查地形覆盖率
static int checkTerrainCoverage(Battlefield* battlefield) {
    int terrainCounts[5] = {0}; // 统计各种地形的数量
    
    for (int i = 0; i < battlefield->height; i++) {
        for (int j = 0; j < battlefield->width; j++) {
            terrainCounts[battlefield->cells[i][j].terrain]++;
        }
    }
    
    int totalCells = battlefield->width * battlefield->height;
    int minRequired = totalCells * 0.05; // 每种地形至少占5%
    
    // 检查每种地形是否达到最小要求
    for (int i = 0; i < 5; i++) {
        if (terrainCounts[i] < minRequired) {
            return 0;
        }
    }
    
    return 1;
}

// 生成道路网络
static void generateRoadNetwork(Battlefield* battlefield) {
    // 收集所有可能的道路节点
    RoadNode* nodes = (RoadNode*)malloc(battlefield->width * battlefield->height * sizeof(RoadNode));
    int nodeCount = 0;
    
    // 在平原区域选择节点
    for (int i = 0; i < battlefield->height; i += battlefield->terrainConfig.roadParams.nodeSpacing) {
        for (int j = 0; j < battlefield->width; j += battlefield->terrainConfig.roadParams.nodeSpacing) {
            if (isInPlainArea(battlefield, j, i, battlefield->terrainConfig.roadParams.minPlainRadius)) {
                nodes[nodeCount].x = j;
                nodes[nodeCount].y = i;
                nodes[nodeCount].connected = 0;
                nodeCount++;
            }
        }
    }
    
    if (nodeCount < 2) {
        free(nodes);
        return;
    }
    
    // 使用类似Prim算法的思想连接节点
    nodes[0].connected = 1; // 从第一个节点开始
    
    while (1) {
        float minDist = -1;
        int bestNode = -1;
        int bestConnected = -1;
        
        // 找到最近的未连接节点
        for (int i = 0; i < nodeCount; i++) {
            if (!nodes[i].connected) {
                for (int j = 0; j < nodeCount; j++) {
                    if (nodes[j].connected) {
                        float dist = distance(nodes[i].x, nodes[i].y, nodes[j].x, nodes[j].y);
                        if (dist < battlefield->terrainConfig.roadParams.maxRoadLength && 
                            canBuildRoad(battlefield, nodes[i].x, nodes[i].y, nodes[j].x, nodes[j].y)) {
                            if (minDist == -1 || dist < minDist) {
                                minDist = dist;
                                bestNode = i;
                                bestConnected = j;
                            }
                        }
                    }
                }
            }
        }
        
        if (bestNode == -1) break;
        
        // 连接节点
        buildRoad(battlefield, nodes[bestNode].x, nodes[bestNode].y, 
                 nodes[bestConnected].x, nodes[bestConnected].y);
        nodes[bestNode].connected = 1;
    }
    
    // 添加一些额外的道路连接，但更谨慎地选择
    for (int i = 0; i < nodeCount; i++) {
        for (int j = i + 1; j < nodeCount; j++) {
            float dist = distance(nodes[i].x, nodes[i].y, nodes[j].x, nodes[j].y);
            // 只在距离较近且没有其他连接的情况下添加额外道路
            if (dist < battlefield->terrainConfig.roadParams.maxRoadLength * 0.7f && 
                rand() % 100 < battlefield->terrainConfig.roadParams.extraRoadChance) {
                // 检查是否已经有其他道路连接这两个区域
                int hasNearbyRoad = 0;
                for (int dx = -2; dx <= 2; dx++) {
                    for (int dy = -2; dy <= 2; dy++) {
                        int checkX = nodes[i].x + dx;
                        int checkY = nodes[i].y + dy;
                        if (isPositionValid(battlefield, checkX, checkY) &&
                            battlefield->cells[checkY][checkX].terrain == TERRAIN_ROAD) {
                            hasNearbyRoad = 1;
                            break;
                        }
                    }
                }
                if (!hasNearbyRoad) {
                    buildRoad(battlefield, nodes[i].x, nodes[i].y, nodes[j].x, nodes[j].y);
                }
            }
        }
    }
    
    free(nodes);
}

// 修建道路
static void buildRoad(Battlefield* battlefield, int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int steps = dx > dy ? dx : dy;
    
    if (steps == 0) return;
    
    float xIncrement = (float)(x2 - x1) / steps;
    float yIncrement = (float)(y2 - y1) / steps;
    
    float x = x1, y = y1;
    for (int i = 0; i <= steps; i++) {
        int checkX = (int)(x + 0.5f);
        int checkY = (int)(y + 0.5f);
        
        if (isPositionValid(battlefield, checkX, checkY)) {
            // 只在平原上修建道路
            if (battlefield->cells[checkY][checkX].terrain == TERRAIN_PLAIN) {
                battlefield->cells[checkY][checkX].terrain = TERRAIN_ROAD;
            }
        }
        
        x += xIncrement;
        y += yIncrement;
    }
}

// 初始化战场地形
void initBattlefieldTerrain(Battlefield* battlefield) {
    if (!battlefield || !battlefield->cells) {
        return;
    }

    // 初始化随机数生成器
    srand((unsigned int)time(NULL));
    int seed = rand();
    int maxAttempts = 10; // 最大尝试次数
    int attempts = 0;

    do {
        // 使用多层噪声生成地形
        for (int i = 0; i < battlefield->height; i++) {
            for (int j = 0; j < battlefield->width; j++) {
                float noise = multiLayerNoise(j, i, seed, &battlefield->terrainConfig);
                noise = (noise + 1.0f) * 0.5f; // 将噪声值映射到0-1范围

                if (noise < battlefield->terrainConfig.baseProbabilities.plain) {
                    battlefield->cells[i][j].terrain = TERRAIN_PLAIN;
                } else if (noise < battlefield->terrainConfig.baseProbabilities.plain + 
                          battlefield->terrainConfig.baseProbabilities.forest) {
                    battlefield->cells[i][j].terrain = TERRAIN_FOREST;
                } else if (noise < battlefield->terrainConfig.baseProbabilities.plain + 
                          battlefield->terrainConfig.baseProbabilities.forest +
                          battlefield->terrainConfig.baseProbabilities.mountain) {
                    battlefield->cells[i][j].terrain = TERRAIN_MOUNTAIN;
                } else if (noise < battlefield->terrainConfig.baseProbabilities.plain + 
                          battlefield->terrainConfig.baseProbabilities.forest +
                          battlefield->terrainConfig.baseProbabilities.mountain +
                          battlefield->terrainConfig.baseProbabilities.water) {
                    battlefield->cells[i][j].terrain = TERRAIN_WATER;
                } else {
                    battlefield->cells[i][j].terrain = TERRAIN_ROAD;
                }
            }
        }

        // 优化地形过渡
        for (int i = 1; i < battlefield->height - 1; i++) {
            for (int j = 1; j < battlefield->width - 1; j++) {
                TerrainType current = battlefield->cells[i][j].terrain;
                
                // 计算周围8格的地形类型
                int plainCount = 0;
                int forestCount = 0;
                int mountainCount = 0;
                int waterCount = 0;
                
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        
                        TerrainType neighbor = battlefield->cells[i + dy][j + dx].terrain;
                        switch (neighbor) {
                            case TERRAIN_PLAIN: plainCount++; break;
                            case TERRAIN_FOREST: forestCount++; break;
                            case TERRAIN_MOUNTAIN: mountainCount++; break;
                            case TERRAIN_WATER: waterCount++; break;
                            default: break;
                        }
                    }
                }
                
                // 根据周围地形调整当前地形
                if (current == TERRAIN_FOREST && plainCount >= battlefield->terrainConfig.transitionParams.plainThreshold) {
                    battlefield->cells[i][j].terrain = TERRAIN_PLAIN;
                } else if (current == TERRAIN_MOUNTAIN && plainCount >= battlefield->terrainConfig.transitionParams.plainThreshold) {
                    battlefield->cells[i][j].terrain = TERRAIN_PLAIN;
                } else if (current == TERRAIN_PLAIN) {
                    if (mountainCount >= battlefield->terrainConfig.transitionParams.mountainThreshold && 
                        rand() % 100 < battlefield->terrainConfig.transitionParams.transitionChance) {
                        battlefield->cells[i][j].terrain = TERRAIN_MOUNTAIN;
                    } else if (forestCount >= battlefield->terrainConfig.transitionParams.forestThreshold && 
                             rand() % 100 < battlefield->terrainConfig.transitionParams.transitionChance) {
                        battlefield->cells[i][j].terrain = TERRAIN_FOREST;
                    }
                }
            }
        }

        // 优化地形
        optimizeTerrain(battlefield);

        // 在平原区域生成小湖泊
        for (int i = 1; i < battlefield->height - 1; i += battlefield->terrainConfig.lakeParams.minDistance) {
            for (int j = 1; j < battlefield->width - 1; j += battlefield->terrainConfig.lakeParams.minDistance) {
                if (battlefield->cells[i][j].terrain == TERRAIN_PLAIN && 
                    isSuitableForWater(battlefield, j, i) && 
                    rand() % 100 < battlefield->terrainConfig.lakeParams.generationChance) {
                    generateSmallLake(battlefield, j, i);
                }
            }
        }

        // 生成道路网络
        generateRoadNetwork(battlefield);

        // 确保指挥部区域是平原
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                // 红方指挥部区域
                battlefield->cells[i][j].terrain = TERRAIN_PLAIN;
                // 蓝方指挥部区域
                battlefield->cells[battlefield->height-4+i][battlefield->width-4+j].terrain = TERRAIN_PLAIN;
            }
        }

        attempts++;
        seed++; // 改变种子以生成不同的地形
    } while (!checkTerrainCoverage(battlefield) && attempts < maxAttempts);
}

// 初始化战场
void initBattlefield(Battlefield* battlefield, int width, int height) {
    if (!battlefield || width <= 0 || height <= 0) {
        return;
    }

    // 初始化地形配置
    initTerrainConfig(&battlefield->terrainConfig);

    battlefield->width = width;
    battlefield->height = height;
    battlefield->redCount = 0;
    battlefield->blueCount = 0;
    battlefield->maxEquipments = 20;  // 每方最大装备数量
    battlefield->redBudget = 10000;   // 红方初始预算
    battlefield->blueBudget = 10000;  // 蓝方初始预算
    battlefield->redRemainingBudget = battlefield->redBudget;
    battlefield->blueRemainingBudget = battlefield->blueBudget;
    battlefield->redHeadquarters = NULL;
    battlefield->blueHeadquarters = NULL;
    battlefield->redHQDeployed = 0;
    battlefield->blueHQDeployed = 0;
    
    // 分配二维数组内存
    battlefield->cells = (Cell**)malloc(height * sizeof(Cell*));
    if (!battlefield->cells) {
        return;
    }

    for (int i = 0; i < height; i++) {
        battlefield->cells[i] = (Cell*)malloc(width * sizeof(Cell));
        if (!battlefield->cells[i]) {
            // 清理已分配的内存
            for (int j = 0; j < i; j++) {
                free(battlefield->cells[j]);
            }
            free(battlefield->cells);
            battlefield->cells = NULL;
            return;
        }
    }
    
    // 分配装备数组内存
    battlefield->redEquipments = (Equipment**)malloc(battlefield->maxEquipments * sizeof(Equipment*));
    battlefield->blueEquipments = (Equipment**)malloc(battlefield->maxEquipments * sizeof(Equipment*));
    
    // 初始化所有格子
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            battlefield->cells[i][j].status = CELL_EMPTY;
            battlefield->cells[i][j].equipment = NULL;
        }
    }

    // 初始化地形
    initBattlefieldTerrain(battlefield);
}

// 释放战场资源
void freeBattlefield(Battlefield* battlefield) {
    // 先释放单元格内存
    for (int i = 0; i < battlefield->height; i++) {
        free(battlefield->cells[i]);
    }
    free(battlefield->cells);
    
    // 释放装备内存
    for (int i = 0; i < battlefield->redCount; i++) {
        if (battlefield->redEquipments[i] && 
            battlefield->redEquipments[i]->typeId != 11) { // 不在这里释放指挥部，避免重复释放
            free(battlefield->redEquipments[i]);
        }
    }
    
    for (int i = 0; i < battlefield->blueCount; i++) {
        if (battlefield->blueEquipments[i] && 
            battlefield->blueEquipments[i]->typeId != 11) { // 不在这里释放指挥部，避免重复释放
            free(battlefield->blueEquipments[i]);
        }
    }
    
    free(battlefield->redEquipments);
    free(battlefield->blueEquipments);
    
    // 单独释放指挥部内存
    if (battlefield->redHeadquarters) {
        free(battlefield->redHeadquarters);
    }
    
    if (battlefield->blueHeadquarters) {
        free(battlefield->blueHeadquarters);
    }
}

// 获取战场格子
Cell* getCell(Battlefield* battlefield, int x, int y) {
    if (x < 0 || x >= battlefield->width || y < 0 || y >= battlefield->height) {
        return NULL;
    }
    return &battlefield->cells[y][x];
}

// 检查位置是否在战场范围内
int isPositionValid(Battlefield* battlefield, int x, int y) {
    return (x >= 0 && x < battlefield->width && y >= 0 && y < battlefield->height);
}

// 检查位置是否在本方半场
int isPositionInOwnHalf(Battlefield* battlefield, int x, int y, Team team) {
    (void)y; // 避免未使用参数警告
    
    if (team == TEAM_RED) {
        return (x < battlefield->width / 2);
    } else if (team == TEAM_BLUE) {
        return (x >= battlefield->width / 2);
    }
    return 0;
}

// 向战场添加装备
int addEquipmentToBattlefield(Battlefield* battlefield, Equipment* equipment) {
    if (!equipment || !isPositionValid(battlefield, equipment->x, equipment->y)) {
        return 0;
    }

    // 检查是否在本方半场
    if (!isPositionInOwnHalf(battlefield, equipment->x, equipment->y, equipment->team)) {
        printf("装备只能部署在己方半场！\n");
        return 0;
    }

    // 检查单元格是否已被占用
    Cell* cell = getCell(battlefield, equipment->x, equipment->y);
    if (cell->status != CELL_EMPTY) {
        printf("该位置已被占用！\n");
        return 0;
    }

    // 检查预算是否足够
    EquipmentType* type = getEquipmentTypeById(equipment->typeId);
    if (!type) {
        return 0;
    }

    if (equipment->team == TEAM_RED) {
        if (type->cost > battlefield->redRemainingBudget) {
            printf("红方预算不足！\n");
            return 0;
        }
        if (battlefield->redCount >= battlefield->maxEquipments) {
            printf("红方装备数量已达上限！\n");
            return 0;
        }
        battlefield->redRemainingBudget -= type->cost;
        battlefield->redEquipments[battlefield->redCount++] = equipment;
        cell->status = CELL_OCCUPIED_RED;
    } else {
        if (type->cost > battlefield->blueRemainingBudget) {
            printf("蓝方预算不足！\n");
            return 0;
        }
        if (battlefield->blueCount >= battlefield->maxEquipments) {
            printf("蓝方装备数量已达上限！\n");
            return 0;
        }
        battlefield->blueRemainingBudget -= type->cost;
        battlefield->blueEquipments[battlefield->blueCount++] = equipment;
        cell->status = CELL_OCCUPIED_BLUE;
    }

    cell->equipment = equipment;
    return 1;
}

// 从战场移除装备
int removeEquipmentFromBattlefield(Battlefield* battlefield, Equipment* equipment) {
    if (!equipment || !isPositionValid(battlefield, equipment->x, equipment->y)) {
        return 0;
    }

    Cell* cell = getCell(battlefield, equipment->x, equipment->y);
    if (cell->equipment != equipment) {
        return 0;
    }

    cell->status = CELL_EMPTY;
    cell->equipment = NULL;

    // 实际上我们不从数组中移除，只是标记为非活跃
    equipment->isActive = 0;
    return 1;
}

// 检查是否有路径障碍
int hasPathObstacle(Battlefield* battlefield, int x1, int y1, int x2, int y2, int ignoreFlying) {
    // 简化版本：只检查直线上的障碍
    // 实际游戏中可能需要更复杂的路径查找算法
    
    // 忽略飞行单位的障碍检查
    if (ignoreFlying) {
        return 0;
    }

    // 计算起点和终点之间的步数
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int steps = dx > dy ? dx : dy;
    
    if (steps == 0) {
        return 0;
    }

    // 计算每一步的增量
    float xIncrement = (float)(x2 - x1) / steps;
    float yIncrement = (float)(y2 - y1) / steps;
    
    // 从起点向终点逐步检查
    float x = x1, y = y1;
    for (int i = 0; i < steps; i++) {
        x += xIncrement;
        y += yIncrement;
        
        int checkX = (int)(x + 0.5f);
        int checkY = (int)(y + 0.5f);
        
        if (isPositionValid(battlefield, checkX, checkY)) {
            Cell* cell = getCell(battlefield, checkX, checkY);
            if (cell->status != CELL_EMPTY) {
                // 检查是否是栅栏类型的装备（可以根据实际游戏规则调整）
                if (cell->equipment && cell->equipment->typeId == 7) { // 假设typeId=7是栅栏
                    return 1;
                }
            }
        }
    }
    
    return 0;
}

// 显示装备方向箭头
char getDirectionChar(int dirX, int dirY) {
    if (dirX == 0 && dirY == -1) return '^';      // 上
    if (dirX == 0 && dirY == 1) return 'v';       // 下
    if (dirX == -1 && dirY == 0) return '<';      // 左
    if (dirX == 1 && dirY == 0) return '>';       // 右
    if (dirX == -1 && dirY == -1) return '\\';    // 左上
    if (dirX == 1 && dirY == -1) return '/';      // 右上
    if (dirX == -1 && dirY == 1) return '/';      // 左下
    if (dirX == 1 && dirY == 1) return '\\';      // 右下
    return ' ';  // 无方向
}

// 渲染战场
void renderBattlefield(Battlefield* battlefield, Team viewOnly) {
    if (!battlefield) {
        return;
    }

    // 绘制列坐标（十位）
    printf("   ");
    for (int j = 0; j < battlefield->width; j++) {
        if (j % 10 == 0) {
            printf("%d", (j / 10) % 10);
        } else {
            printf(" ");
        }
    }
    printf("\n");

    // 绘制列坐标（个位）
    printf("   ");
    for (int j = 0; j < battlefield->width; j++) {
        printf("%d", j % 10);
    }
    printf("\n");

    // 绘制上边框
    printf("  +");
    for (int j = 0; j < battlefield->width; j++) {
        printf("-");
    }
    printf("+\n");

    // 绘制战场内容
    for (int i = 0; i < battlefield->height; i++) {
        printf("%2d|", i);
        for (int j = 0; j < battlefield->width; j++) {
            Cell* cell = &battlefield->cells[i][j];
            
            // 获取地形颜色和字符
            const char* terrainColor = getTerrainColorCode(cell->terrain);
            char terrainChar = getTerrainChar(cell->terrain);
            
            // 根据格子状态显示不同内容
                    if (cell->status == CELL_EMPTY) {
                printf("%s%c\033[0m", terrainColor, terrainChar);
            } else if (cell->status == CELL_OCCUPIED_RED) {
                if (viewOnly == TEAM_NONE || viewOnly == TEAM_RED) {
                if (cell->equipment && cell->equipment->isActive) {
                    // 根据装备类型显示不同字符
                    switch (cell->equipment->typeId) {
                            case 1: printf("\033[31mT\033[0m"); break; // 坦克
                            case 2: printf("\033[31mA\033[0m"); break; // 飞机
                            case 3: printf("\033[31mC\033[0m"); break; // 火炮
                            case 4: printf("\033[31mM\033[0m"); break; // 导弹
                            case 5: printf("\033[31mS\033[0m"); break; // 士兵
                            case 6: printf("\033[31mG\033[0m"); break; // 枪塔
                            case 7: printf("\033[31m#\033[0m"); break; // 栅栏
                            case 8: printf("\033[31mV\033[0m"); break; // 装甲车
                            case 9: printf("\033[31mH\033[0m"); break; // 重型机枪
                            case 10: printf("\033[31mK\033[0m"); break; // 反坦克炮
                            default: printf("\033[31mR\033[0m"); break;
                    }
                } else {
                        printf("\033[31mx\033[0m"); // 已摧毁
                    }
                } else {
                    printf("%s%c\033[0m", terrainColor, terrainChar);
                }
            } else if (cell->status == CELL_OCCUPIED_BLUE) {
                if (viewOnly == TEAM_NONE || viewOnly == TEAM_BLUE) {
                if (cell->equipment && cell->equipment->isActive) {
                    // 根据装备类型显示不同字符
                    switch (cell->equipment->typeId) {
                            case 1: printf("\033[34mt\033[0m"); break; // 坦克
                            case 2: printf("\033[34ma\033[0m"); break; // 飞机
                            case 3: printf("\033[34mc\033[0m"); break; // 火炮
                            case 4: printf("\033[34mm\033[0m"); break; // 导弹
                            case 5: printf("\033[34ms\033[0m"); break; // 士兵
                            case 6: printf("\033[34mg\033[0m"); break; // 枪塔
                            case 7: printf("\033[34m#\033[0m"); break; // 栅栏
                            case 8: printf("\033[34mv\033[0m"); break; // 装甲车
                            case 9: printf("\033[34mh\033[0m"); break; // 重型机枪
                            case 10: printf("\033[34mk\033[0m"); break; // 反坦克炮
                            default: printf("\033[34mb\033[0m"); break;
                    }
                } else {
                        printf("\033[34mx\033[0m"); // 已摧毁
                    }
                } else {
                    printf("%s%c\033[0m", terrainColor, terrainChar);
                }
            }
        }
        printf("|\n");
    }

    // 绘制下边框
    printf("  +");
    for (int j = 0; j < battlefield->width; j++) {
        printf("-");
    }
    printf("+\n");

    // 显示图例
    printf("\n地形图例:\n");
    printf("\033[42m.\033[0m 平原  ");
    printf("\033[100m^\033[0m 山地  ");
    printf("\033[32m*\033[0m 森林  ");
    printf("\033[44m~\033[0m 水域  ");
    printf("\033[43m=\033[0m 道路\n");

    // 显示红方装备状态（如果viewOnly是TEAM_NONE或TEAM_RED）
    if (viewOnly == TEAM_NONE || viewOnly == TEAM_RED) {
        printf("红方装备:\n");
        int activeRedCount = 0;
        for (int i = 0; i < battlefield->redCount; i++) {
            if (battlefield->redEquipments[i]->isActive) {
                activeRedCount++;
                displayEquipmentInfo(battlefield->redEquipments[i]);
            }
        }
        if (activeRedCount == 0 && battlefield->redCount > 0) {
            printf("红方全军覆没！\n");
        }
    }

    // 显示蓝方装备状态（如果viewOnly是TEAM_NONE或TEAM_BLUE）
    if (viewOnly == TEAM_NONE || viewOnly == TEAM_BLUE) {
        printf("蓝方装备:\n");
        int activeBlueCount = 0;
        for (int i = 0; i < battlefield->blueCount; i++) {
            if (battlefield->blueEquipments[i]->isActive) {
                activeBlueCount++;
                displayEquipmentInfo(battlefield->blueEquipments[i]);
            }
        }
        if (activeBlueCount == 0 && battlefield->blueCount > 0) {
            printf("蓝方全军覆没！\n");
        }
    }
}

// 部署装备菜单
void showDeployMenu(Battlefield* battlefield, Team team) {
    system("cls");
    
    // 显示战场当前状态，只显示当前方的装备
    renderBattlefield(battlefield, team);
    
    printf("\n装备部署菜单 - %s方\n", team == TEAM_RED ? "红" : "蓝");
    printf("可用预算: %d\n", team == TEAM_RED ? battlefield->redRemainingBudget : battlefield->blueRemainingBudget);
    
    // 显示图例
    printf("\n图例说明:\n");
    printf("红方: T(坦克) A(飞机) C(火炮) M(导弹) S(士兵) G(枪塔) #(栅栏) V(装甲车) H(重机枪) K(反坦克炮)\n");
    printf("蓝方: t(坦克) a(飞机) c(火炮) m(导弹) s(士兵) g(枪塔) #(栅栏) v(装甲车) h(重机枪) k(反坦克炮)\n");
    printf("方向: ^(上) v(下) <(左) >(右) \\(左上/右下) /(右上/左下)\n");
    printf("边界: 红方区域(左半场 0-%d), 蓝方区域(右半场 %d-%d)\n", battlefield->width/2-1, battlefield->width/2, battlefield->width-1);
    
    printf("\n可用装备类型:\n");
    for (int i = 0; i < g_equipmentTypesCount; i++) {
        printf("%d. %s (造价: %d, 生命值: %d, 速度: %d, 攻击范围: %d, 弹药: %d)\n",
               g_equipmentTypes[i].typeId, g_equipmentTypes[i].name, g_equipmentTypes[i].cost,
               g_equipmentTypes[i].maxHealth, g_equipmentTypes[i].maxSpeed,
               g_equipmentTypes[i].maxAttackRadius, g_equipmentTypes[i].maxAmmo);
    }
    
    printf("\n");
    printf("请选择装备类型 (输入0结束部署): ");
}

// 显示装备信息
void displayEquipmentInfo(Equipment* equipment) {
    if (!equipment || !equipment->isActive) {
        return;
    }

    EquipmentType* type = getEquipmentTypeById(equipment->typeId);
    if (!type) {
        return;
    }

    char dirChar = getDirectionChar(equipment->directionX, equipment->directionY);
    printf("ID: %d, 名称: %s, 位置: (%d,%d), 方向: %c, 生命值: %d/%d, 弹药: %d/%d\n",
           equipment->id, equipment->name, equipment->x, equipment->y, dirChar,
           equipment->currentHealth, type->maxHealth,
           equipment->currentAmmo, type->maxAmmo);
}

// 部署装备到战场
int deployEquipment(Battlefield* battlefield, Team team) {
    while (1) {
        showDeployMenu(battlefield, team);
        
        int typeId;
        scanf("%d", &typeId);
        
        if (typeId == 0) {
            return 1;
        }
        
        // 检查是否为指挥部类型，不允许在常规部署阶段部署指挥部
        if (typeId == 11) {
            printf("指挥部只能在游戏开始阶段部署！\n");
            printf("按任意键继续...\n");
            getch();
            continue;
        }
        
        EquipmentType* type = getEquipmentTypeById(typeId);
        if (!type) {
            printf("无效的装备类型ID！\n");
            printf("按任意键继续...\n");
            getch();
            continue;
        }
        
        printf("选择位置 (x y): ");
        int x, y;
        scanf("%d %d", &x, &y);
        
        if (!isPositionValid(battlefield, x, y)) {
            printf("位置超出边界！\n");
            printf("按任意键继续...\n");
            getch();
            continue;
        }
        
        if (!isPositionInOwnHalf(battlefield, x, y, team)) {
            printf("必须在己方半场部署装备！\n");
            printf("己方半场范围: %s\n", 
                  team == TEAM_RED ? "x: 0-39" : "x: 40-79");
            printf("按任意键继续...\n");
            getch();
            continue;
        }
        
        printf("选择方向 (dx dy): ");
        int dirX, dirY;
        scanf("%d %d", &dirX, &dirY);
        
        // 规范化方向向量
        if (dirX != 0 || dirY != 0) {
            // 计算最大公约数
            int a = abs(dirX);
            int b = abs(dirY);
            while (b) {
                int t = b;
                b = a % b;
                a = t;
            }
            int gcd = a;
            
            if (gcd > 0) {
                dirX /= gcd;
                dirY /= gcd;
            }
            
            // 限制方向向量的值范围
            if (dirX > 1) dirX = 1;
            if (dirX < -1) dirX = -1;
            if (dirY > 1) dirY = 1;
            if (dirY < -1) dirY = -1;
        }
        
        // 显示用户选择的方向
        char dirChar = getDirectionChar(dirX, dirY);
        printf("已选择方向: %c\n", dirChar);
        
        Equipment* equipment = createEquipment(typeId, team, x, y, dirX, dirY);
        if (!equipment) {
            printf("创建装备失败！\n");
            printf("按任意键继续...\n");
            getch();
            continue;
        }
        
        if (!addEquipmentToBattlefield(battlefield, equipment)) {
            printf("部署装备失败！\n");
            free(equipment);
            printf("按任意键继续...\n");
            getch();
            continue;
        }
        
        // 不再刷新显示战场，只显示成功信息
        printf("\n装备部署成功！位置: (%d,%d), 方向: %c\n", x, y, dirChar);
        printf("按任意键继续部署...\n");
        getch();
    }
    
    return 1;
}

// 占用4x4区域部署指挥部
int occupyHeadquartersArea(Battlefield* battlefield, int x, int y, Team team, Equipment* headquarters) {
    // 检查4x4区域是否完全在战场内且在己方半场
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int checkX = x + j;
            int checkY = y + i;
            
            // 检查坐标是否有效
            if (!isPositionValid(battlefield, checkX, checkY)) {
                printf("指挥部超出战场范围！\n");
                return 0;
            }
            
            // 检查是否在己方半场
            if (!isPositionInOwnHalf(battlefield, checkX, checkY, team)) {
                printf("指挥部必须部署在己方半场！\n");
                return 0;
            }
            
            // 检查是否已被占用
            Cell* cell = getCell(battlefield, checkX, checkY);
            if (cell->status != CELL_EMPTY) {
                printf("指挥部区域已被占用！\n");
                return 0;
            }
        }
    }
    
    // 占用整个4x4区域
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int occupyX = x + j;
            int occupyY = y + i;
            
            Cell* cell = getCell(battlefield, occupyX, occupyY);
            
            // 只将左上角的位置与指挥部关联
            if (i == 0 && j == 0) {
                cell->equipment = headquarters;
            } else {
                // 其他位置只标记为占用状态
                cell->equipment = NULL;
            }
            
            cell->status = team == TEAM_RED ? CELL_OCCUPIED_RED : CELL_OCCUPIED_BLUE;
        }
    }
    
    // 设置指挥部位置
    headquarters->x = x;
    headquarters->y = y;
    
    return 1;
}

// 部署指挥部到战场
int deployHeadquarters(Battlefield* battlefield, Team team) {
    char teamName[10];
    strcpy(teamName, team == TEAM_RED ? "红方" : "蓝方");
    
    // 检查是否已经部署过指挥部
    if ((team == TEAM_RED && battlefield->redHQDeployed) ||
        (team == TEAM_BLUE && battlefield->blueHQDeployed)) {
        printf("%s指挥部已经部署，不能重复部署！\n", teamName);
        return 1;  // 返回1表示"成功"，因为指挥部已存在
    }
    
    printf("\n开始部署%s指挥部 (4x4区域)...\n", teamName);
    printf("请输入指挥部左上角的坐标 (x y): ");
    
    int x, y;
    scanf("%d %d", &x, &y);
    
    // 创建指挥部装备
    Equipment* headquarters = createEquipment(11, team, x, y, 0, 0);
    if (!headquarters) {
        printf("创建指挥部失败！\n");
        return 0;
    }
    
    // 占用4x4区域
    if (!occupyHeadquartersArea(battlefield, x, y, team, headquarters)) {
        free(headquarters);
        return 0;
    }
    
    // 保存指挥部引用并更新部署状态
    if (team == TEAM_RED) {
        battlefield->redHeadquarters = headquarters;
        battlefield->redHQDeployed = 1;
    } else {
        battlefield->blueHeadquarters = headquarters;
        battlefield->blueHQDeployed = 1;
    }
    
    printf("%s指挥部部署成功！\n", teamName);
    return 1;
} 