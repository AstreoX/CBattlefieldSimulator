#ifndef TERRAIN_H
#define TERRAIN_H

// 地形高度范围定义
#define TERRAIN_MIN_HEIGHT -3
#define TERRAIN_MAX_HEIGHT 5

// 地形特性标志
#define TERRAIN_FLAG_NONE      0x0000  // 无特性
#define TERRAIN_FLAG_RIVER     0x0001  // 河流
#define TERRAIN_FLAG_FOREST    0x0002  // 森林
#define TERRAIN_FLAG_CLIFF     0x0004  // 悬崖
#define TERRAIN_FLAG_ROAD      0x0008  // 道路

// 地形类型枚举
typedef enum {
    TERRAIN_WATER,      // 水域（高度 < 0）
    TERRAIN_FLAT,       // 平地（高度 = 0）
    TERRAIN_HILL_1,     // 丘陵1（高度 = 1）
    TERRAIN_HILL_2,     // 丘陵2（高度 = 2）
    TERRAIN_HILL_3,     // 丘陵3（高度 = 3）
    TERRAIN_MOUNTAIN_1, // 山地1（高度 = 4）
    TERRAIN_MOUNTAIN_2, // 山地2（高度 = 5）
    TERRAIN_TYPE_COUNT  // 类型总数
} TerrainType;

// 地形结构
typedef struct {
    float height;      // 地形高度值 (-3 to 5)
    TerrainType type;  // 地形类型
    unsigned int flags; // 地形特性标志
    float moisture;    // 水分值 (0.0-1.0)
    float temperature; // 温度值 (0.0-1.0)
} Terrain;

// 地形展示配置
typedef struct {
    int showGrid;           // 是否显示网格
    int useColorGradient;   // 是否使用颜色渐变
    int showFeatures;       // 是否显示特殊地形特性
    int symbolSize;         // 符号大小 (1 或 2)
} TerrainDisplayConfig;

// 初始化地形系统
void initTerrain(int width, int height, Terrain** terrainMap);

// 释放地形系统资源
void freeTerrain(int height, Terrain** terrainMap);

// 使用噪声算法生成地形
void generateTerrain(int width, int height, Terrain** terrainMap, float scale, int seed);

// 根据高度值获取地形类型
TerrainType getTerrainTypeFromHeight(float height);

// 获取地形对应的显示字符
char* getTerrainChar(TerrainType type);

// 获取地形对应的显示颜色
int terrain_getTerrainColor(TerrainType type);

// 获取默认地形显示配置
TerrainDisplayConfig getDefaultDisplayConfig();

// 设置单个地形单元格的特性
void setTerrainFeature(Terrain* terrain, unsigned int feature);

// 移除单个地形单元格的特性
void removeTerrainFeature(Terrain* terrain, unsigned int feature);

// 检查地形单元格是否有指定特性
int hasTerrainFeature(Terrain* terrain, unsigned int feature);

// 计算地形上两点间的行走难度
float calculateMovementDifficulty(Terrain* start, Terrain* end);

// 获取地形描述文本
const char* getTerrainDescription(TerrainType type);

#endif // TERRAIN_H
