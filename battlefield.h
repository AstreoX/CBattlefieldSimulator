#ifndef BATTLEFIELD_H
#define BATTLEFIELD_H

#include "equipment.h"
#include "terrain.h"
#include "terrain_generation.h"

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
    Terrain** terrain;           // 地形数据
    int useTerrainSystem;        // 是否使用地形系统
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

// 生成地形
void generateBattlefieldTerrain(Battlefield* battlefield, float scale, int seed);

// 使用指定算法生成地形
void generateBattlefieldTerrainWithAlgorithm(Battlefield* battlefield, float scale, int seed, NoiseAlgorithmType algorithmType);

#endif // BATTLEFIELD_H 