#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 队伍枚举
typedef enum {
    TEAM_RED,
    TEAM_BLUE,
    TEAM_NONE
} Team;

// 装备类型结构体
typedef struct {
    int typeId;             // 装备类型编号
    char name[32];          // 装备名称
    int cost;               // 装备造价
    int maxHealth;          // 最高生命值
    int maxSpeed;           // 最高行进速度 (0表示固定装备)
    int maxAttackRadius;    // 最大打击半径
    int maxAmmo;            // 最大装弹量
    int maxFireRate;        // 最高射速（每秒可发射的子弹数）
    int canFly;             // 是否可以飞行 (1表示可以飞行，0表示不可以)
} EquipmentType;

// 装备交互结构体 (描述不同装备之间的攻击效果)
typedef struct {
    int attackerId;         // 攻击方装备类型ID
    int defenderId;         // 防守方装备类型ID
    int damage;             // 单发伤害值
    int accuracy;           // 射击精确度 (0-100)
} EquipmentInteraction;

// 装备单元 (战场上的具体装备实例)
typedef struct {
    int id;                 // 装备单元ID
    int typeId;             // 装备类型ID
    Team team;              // 所属队伍
    char name[32];          // 装备名称
    int currentHealth;      // 当前生命值
    int currentSpeed;       // 当前速度
    int currentAmmo;        // 当前弹药量
    int x, y;               // 当前位置
    int directionX, directionY; // 移动方向
    int deployTime;         // 上场时间
    int isActive;           // 是否活跃 (1表示活跃，0表示已被摧毁)
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

// 根据ID获取装备类型
EquipmentType* getEquipmentTypeById(int typeId);

// 获取两种装备之间的交互信息
EquipmentInteraction* getInteraction(int attackerId, int defenderId);

// 创建一个新的装备实例
Equipment* createEquipment(int typeId, Team team, int x, int y, int dirX, int dirY);

// 释放装备类型资源
void freeEquipmentTypes();

// 计算装备对另一装备的伤害
int calculateDamage(Equipment* attacker, Equipment* defender);

// 检查装备是否可以攻击
int canAttack(Equipment* attacker, Equipment* defender, int distance);

#endif // EQUIPMENT_H 