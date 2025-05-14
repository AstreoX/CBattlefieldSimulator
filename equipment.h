#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 队伍类型
typedef enum {
    TEAM_NONE = 0,
    TEAM_RED = 1,
    TEAM_BLUE = 2
} Team;

// 装备类型
typedef struct {
    int typeId;             // 装备类型ID
    char name[50];          // 装备名称
    int cost;               // 装备造价
    int maxHealth;          // 最高生命值
    int maxSpeed;           // 最高行进速度
    int maxAttackRadius;    // 最大打击半径
    int maxAmmo;            // 最大装弹量
    int maxFireRate;        // 最高射速
    int canFly;             // 是否可以飞行
    float plainSpeed;       // 平原移动速度倍率
    float mountainSpeed;    // 山地移动速度倍率
    float forestSpeed;      // 森林移动速度倍率
    float waterSpeed;       // 水域移动速度倍率
    float roadSpeed;        // 道路移动速度倍率
} EquipmentType;

// 装备交互结构体 (描述不同装备之间的攻击效果)
typedef struct {
    int attackerId;         // 攻击方装备类型ID
    int defenderId;         // 防守方装备类型ID
    int damage;             // 单发伤害值
    int accuracy;           // 射击精确度 (0-100)
} EquipmentInteraction;

// 装备
typedef struct {
    int id;                 // 装备ID
    int typeId;             // 装备类型ID
    char name[50];          // 装备名称
    Team team;              // 所属队伍
    int x;                  // 当前位置X坐标
    int y;                  // 当前位置Y坐标
    int directionX;         // 朝向X分量
    int directionY;         // 朝向Y分量
    int currentHealth;      // 当前生命值
    int currentAmmo;        // 当前弹药量
    int isActive;           // 是否活跃
    float currentSpeed;     // 当前速度（考虑地形影响）
} Equipment;

// 全局装备类型数组
extern EquipmentType* g_equipmentTypes;
extern int g_equipmentTypesCount;

// 全局装备交互数组
extern EquipmentInteraction* g_equipmentInteractions;
extern int g_equipmentInteractionsCount;

// 加载装备类型
int loadEquipmentTypes(const char* filename);

// 加载装备交互信息
int loadEquipmentInteractions(const char* filename);

// 获取装备类型
EquipmentType* getEquipmentTypeById(int typeId);

// 获取两种装备之间的交互信息
EquipmentInteraction* getInteraction(int attackerId, int defenderId);

// 创建装备
Equipment* createEquipment(int typeId, Team team, int x, int y, int dirX, int dirY);

// 销毁装备
void destroyEquipment(Equipment* equipment);

// 更新装备速度（考虑地形影响）
void updateEquipmentSpeed(Equipment* equipment, int terrainType);

// 获取装备在地形上的实际移动速度
float getEquipmentTerrainSpeed(Equipment* equipment, int terrainType);

// 释放装备类型资源
void freeEquipmentTypes();

// 计算装备对另一装备的伤害
int calculateDamage(Equipment* attacker, Equipment* defender);

// 检查装备是否可以攻击
int canAttack(Equipment* attacker, Equipment* defender, int distance);

#endif // EQUIPMENT_H 