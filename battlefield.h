#ifndef BATTLEFIELD_H
#define BATTLEFIELD_H

#include "equipment.h"

// 地形类型
typedef enum {
    TERRAIN_PLAIN,      // 平原
    TERRAIN_MOUNTAIN,   // 山地
    TERRAIN_FOREST,     // 森林
    TERRAIN_WATER,      // 水域
    TERRAIN_ROAD        // 道路
} TerrainType;

// 地形生成配置
typedef struct {
    // 基础地形生成概率
    struct {
        float plain;     // 平原生成概率
        float forest;    // 森林生成概率
        float mountain;  // 山地生成概率
        float water;     // 水域生成概率
        float road;      // 道路生成概率
    } baseProbabilities;

    // 地形过渡参数
    struct {
        int plainThreshold;      // 平原转换阈值
        int mountainThreshold;   // 山地转换阈值
        int forestThreshold;     // 森林转换阈值
        int transitionChance;    // 地形转换概率
    } transitionParams;

    // 湖泊生成参数
    struct {
        int minDistance;         // 湖泊最小间距
        int lakeSize;           // 湖泊大小
        int generationChance;    // 湖泊生成概率
        int plainCheckRadius;    // 平原检查半径
    } lakeParams;

    // 道路生成参数
    struct {
        int nodeSpacing;        // 道路节点间距
        int maxRoadLength;      // 最大道路长度
        int extraRoadChance;    // 额外道路生成概率
        int minPlainRadius;     // 最小平原半径
    } roadParams;

    // 噪声生成参数
    struct {
        float baseFrequency;    // 基础频率
        int noiseLayers;        // 噪声层数
        float persistence;      // 持续性
        float lacunarity;       // 间隙度
    } noiseParams;
} TerrainConfig;

// 战场格子状态
typedef enum {
    CELL_EMPTY,
    CELL_OCCUPIED_RED,
    CELL_OCCUPIED_BLUE
} CellStatus;

// 战场格子
typedef struct {
    CellStatus status;
    Equipment* equipment;
    TerrainType terrain;  // 地形类型
} Cell;

// 战场
typedef struct {
    int width;      // 战场宽度
    int height;     // 战场高度
    Cell** cells;   // 二维格子数组
    int redCount;   // 红方装备数量
    int blueCount;  // 蓝方装备数量
    Equipment** redEquipments;   // 红方装备数组
    Equipment** blueEquipments;  // 蓝方装备数组
    int maxEquipments;           // 每方最大装备数量
    int redBudget;               // 红方预算
    int blueBudget;              // 蓝方预算
    int redRemainingBudget;      // 红方剩余预算
    int blueRemainingBudget;     // 蓝方剩余预算
    Equipment* redHeadquarters;  // 红方指挥部
    Equipment* blueHeadquarters; // 蓝方指挥部
    int redHQDeployed;           // 红方指挥部是否已部署
    int blueHQDeployed;          // 蓝方指挥部是否已部署
    TerrainConfig terrainConfig; // 地形生成配置
} Battlefield;

// 初始化战场
void initBattlefield(Battlefield* battlefield, int width, int height);

// 释放战场资源
void freeBattlefield(Battlefield* battlefield);

// 部署装备到战场
int deployEquipment(Battlefield* battlefield, Team team);

// 部署指挥部到战场
int deployHeadquarters(Battlefield* battlefield, Team team);

// 渲染战场
// viewOnly参数如果不是TEAM_NONE，则只显示指定队伍的装备
void renderBattlefield(Battlefield* battlefield, Team viewOnly);

// 向战场添加装备
int addEquipmentToBattlefield(Battlefield* battlefield, Equipment* equipment);

// 从战场移除装备
int removeEquipmentFromBattlefield(Battlefield* battlefield, Equipment* equipment);

// 获取战场格子
Cell* getCell(Battlefield* battlefield, int x, int y);

// 检查位置是否在战场范围内
int isPositionValid(Battlefield* battlefield, int x, int y);

// 检查位置是否在本方半场
int isPositionInOwnHalf(Battlefield* battlefield, int x, int y, Team team);

// 检查是否有路径障碍
int hasPathObstacle(Battlefield* battlefield, int x1, int y1, int x2, int y2, int ignoreFlying);

// 占用4x4区域部署指挥部
int occupyHeadquartersArea(Battlefield* battlefield, int x, int y, Team team, Equipment* headquarters);

// 初始化战场地形
void initBattlefieldTerrain(Battlefield* battlefield);

// 获取地形颜色代码
const char* getTerrainColorCode(TerrainType terrain);

// 获取地形显示字符
char getTerrainChar(TerrainType terrain);

// 初始化地形配置
void initTerrainConfig(TerrainConfig* config);

// 地形生成菜单
int terrainGenerationMenu(Battlefield* battlefield);

#endif // BATTLEFIELD_H 