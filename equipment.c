#include "equipment.h"
#include <math.h>

// 全局装备类型数组
EquipmentType* g_equipmentTypes = NULL;
int g_equipmentTypesCount = 0;

// 全局装备交互数组
EquipmentInteraction* g_equipmentInteractions = NULL;
int g_equipmentInteractionsCount = 0;

// 装备ID计数器
static int g_nextEquipmentId = 1;

// 加载装备类型
int loadEquipmentTypes(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("无法打开装备类型文件: %s\n", filename);
        return 0;
    }

    // 首先计算装备类型数量
    int count = 0;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file)) {
        if (buffer[0] != '#' && strlen(buffer) > 5) { // 跳过注释行和空行
            count++;
        }
    }

    // 重置文件指针
    rewind(file);

    // 分配内存
    g_equipmentTypes = (EquipmentType*)malloc(count * sizeof(EquipmentType));
    if (!g_equipmentTypes) {
        printf("内存分配失败\n");
        fclose(file);
        return 0;
    }

    // 读取装备类型
    int index = 0;
    while (fgets(buffer, sizeof(buffer), file)) {
        if (buffer[0] == '#' || strlen(buffer) <= 5) { // 跳过注释行和空行
            continue;
        }

        EquipmentType* type = &g_equipmentTypes[index];
        sscanf(buffer, "%d,%[^,],%d,%d,%d,%d,%d,%d,%d",
               &type->typeId, type->name, &type->cost, &type->maxHealth,
               &type->maxSpeed, &type->maxAttackRadius, &type->maxAmmo,
               &type->maxFireRate, &type->canFly);
        index++;
    }

    g_equipmentTypesCount = count;
    fclose(file);
    return 1;
}

// 加载装备交互信息
int loadEquipmentInteractions(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("无法打开装备交互文件: %s\n", filename);
        return 0;
    }

    // 首先计算交互数量
    int count = 0;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file)) {
        if (buffer[0] != '#' && strlen(buffer) > 5) { // 跳过注释行和空行
            count++;
        }
    }

    // 重置文件指针
    rewind(file);

    // 分配内存
    g_equipmentInteractions = (EquipmentInteraction*)malloc(count * sizeof(EquipmentInteraction));
    if (!g_equipmentInteractions) {
        printf("内存分配失败\n");
        fclose(file);
        return 0;
    }

    // 读取交互信息
    int index = 0;
    while (fgets(buffer, sizeof(buffer), file)) {
        if (buffer[0] == '#' || strlen(buffer) <= 5) { // 跳过注释行和空行
            continue;
        }

        EquipmentInteraction* interaction = &g_equipmentInteractions[index];
        sscanf(buffer, "%d,%d,%d,%d",
               &interaction->attackerId, &interaction->defenderId,
               &interaction->damage, &interaction->accuracy);
        index++;
    }

    g_equipmentInteractionsCount = count;
    fclose(file);
    return 1;
}

// 根据ID获取装备类型
EquipmentType* getEquipmentTypeById(int typeId) {
    for (int i = 0; i < g_equipmentTypesCount; i++) {
        if (g_equipmentTypes[i].typeId == typeId) {
            return &g_equipmentTypes[i];
        }
    }
    return NULL;
}

// 获取两种装备之间的交互信息
EquipmentInteraction* getInteraction(int attackerId, int defenderId) {
    for (int i = 0; i < g_equipmentInteractionsCount; i++) {
        if (g_equipmentInteractions[i].attackerId == attackerId &&
            g_equipmentInteractions[i].defenderId == defenderId) {
            return &g_equipmentInteractions[i];
        }
    }
    return NULL;
}

// 创建一个新的装备实例
Equipment* createEquipment(int typeId, Team team, int x, int y, int dirX, int dirY) {
    EquipmentType* type = getEquipmentTypeById(typeId);
    if (!type) {
        return NULL;
    }

    Equipment* equipment = (Equipment*)malloc(sizeof(Equipment));
    if (!equipment) {
        return NULL;
    }

    equipment->id = g_nextEquipmentId++;
    equipment->typeId = typeId;
    equipment->team = team;
    strcpy(equipment->name, type->name);
    equipment->currentHealth = type->maxHealth;
    equipment->currentSpeed = type->maxSpeed;
    equipment->currentAmmo = type->maxAmmo;
    equipment->x = x;
    equipment->y = y;
    equipment->directionX = dirX;
    equipment->directionY = dirY;
    equipment->deployTime = 0;
    equipment->isActive = 1;

    return equipment;
}

// 释放装备类型资源
void freeEquipmentTypes() {
    if (g_equipmentTypes) {
        free(g_equipmentTypes);
        g_equipmentTypes = NULL;
        g_equipmentTypesCount = 0;
    }

    if (g_equipmentInteractions) {
        free(g_equipmentInteractions);
        g_equipmentInteractions = NULL;
        g_equipmentInteractionsCount = 0;
    }
}

// 计算装备对另一装备的伤害
int calculateDamage(Equipment* attacker, Equipment* defender) {
    if (!attacker || !defender || attacker->team == defender->team) {
        return 0;
    }

    EquipmentInteraction* interaction = getInteraction(attacker->typeId, defender->typeId);
    if (!interaction) {
        return 0;
    }

    // 考虑射击精度因素
    if (rand() % 100 >= interaction->accuracy) {
        return 0; // 未命中
    }

    return interaction->damage;
}

// 计算两点间距离 - 已由calculateEquipmentDistance函数替代，保留供日后使用
/* 
static double calculateDistance(int x1, int y1, int x2, int y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}
*/

// 检查装备是否可以攻击
int canAttack(Equipment* attacker, Equipment* defender, int distance) {
    if (!attacker || !defender || !attacker->isActive || !defender->isActive ||
        attacker->team == defender->team || attacker->currentAmmo <= 0) {
        return 0;
    }

    EquipmentType* attackerType = getEquipmentTypeById(attacker->typeId);
    if (!attackerType) {
        return 0;
    }

    // 检查是否在攻击范围内
    int attackRadius = attackerType->maxAttackRadius;
    if (distance > attackRadius) {
        return 0;
    }

    return 1;
} 