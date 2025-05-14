#include "simulation.h"
#include <math.h>
#include <windows.h>

// 计算两个装备之间的距离
int calculateEquipmentDistance(Equipment* e1, Equipment* e2) {
    if (!e1 || !e2) {
        return INT_MAX;
    }
    return (int)sqrt((e2->x - e1->x) * (e2->x - e1->x) + (e2->y - e1->y) * (e2->y - e1->y));
}

// 查找最近的敌方装备
Equipment* findNearestEnemy(Battlefield* battlefield, Equipment* equipment) {
    if (!equipment || !equipment->isActive) {
        return NULL;
    }

    Equipment* nearest = NULL;
    int minDistance = INT_MAX;

    // 根据装备所属队伍选择敌方装备数组
    Equipment** enemyEquipments;
    int enemyCount;

    if (equipment->team == TEAM_RED) {
        enemyEquipments = battlefield->blueEquipments;
        enemyCount = battlefield->blueCount;
    } else {
        enemyEquipments = battlefield->redEquipments;
        enemyCount = battlefield->redCount;
    }

    // 遍历敌方装备，找到最近的一个
    for (int i = 0; i < enemyCount; i++) {
        Equipment* enemy = enemyEquipments[i];
        if (!enemy->isActive) {
            continue;
        }

        int distance = calculateEquipmentDistance(equipment, enemy);
        if (distance < minDistance) {
            minDistance = distance;
            nearest = enemy;
        }
    }

    return nearest;
}

// 处理装备移动
void handleMovement(Battlefield* battlefield, Equipment* equipment) {
    if (!equipment || !equipment->isActive) {
        return;
    }

    EquipmentType* type = getEquipmentTypeById(equipment->typeId);
    if (!type || type->maxSpeed == 0) { // 固定装备不移动
        return;
    }

    // 计算新位置
    int newX = equipment->x + equipment->directionX;
    int newY = equipment->y + equipment->directionY;

    // 检查水平和垂直方向上的碰撞
    int hitLeft = (newX < 0);                       // 左边界碰撞
    int hitRight = (newX >= battlefield->width);    // 右边界碰撞
    int hitTop = (newY < 0);                        // 上边界碰撞
    int hitBottom = (newY >= battlefield->height);  // 下边界碰撞
    
    // 检查是否碰到了其他装备
    int hitEquipment = 0;
    // 只有在不碰到边界的情况下才检查装备碰撞
    if (!hitLeft && !hitRight && !hitTop && !hitBottom) {
        Cell* cell = getCell(battlefield, newX, newY);
        hitEquipment = (cell->status != CELL_EMPTY);
    }

    // 计算反弹方向
    if (hitLeft || hitRight || (hitEquipment && equipment->directionX != 0)) {
        // 水平方向反弹
        equipment->directionX = -equipment->directionX;
    }
    
    if (hitTop || hitBottom || (hitEquipment && equipment->directionY != 0)) {
        // 垂直方向反弹
        equipment->directionY = -equipment->directionY;
    }

    // 对于斜向移动的装备，处理角落碰撞（可能同时碰到两个边界）
    if ((hitLeft && hitTop) || (hitLeft && hitBottom) || 
        (hitRight && hitTop) || (hitRight && hitBottom)) {
        // 同时碰到两个边界，两个方向都反向
        equipment->directionX = -equipment->directionX;
        equipment->directionY = -equipment->directionY;
    }
    
    // 检查碰撞后的新位置
    newX = equipment->x + equipment->directionX;
    newY = equipment->y + equipment->directionY;
    
    // 确保新位置有效，如果无效则不移动
    if (!isPositionValid(battlefield, newX, newY)) {
        return;
    }
    
    // 确保新位置未被占用
    Cell* cell = getCell(battlefield, newX, newY);
    if (cell->status != CELL_EMPTY) {
        return;
    }

    // 执行移动
    // 先从原位置移除
    Cell* oldCell = getCell(battlefield, equipment->x, equipment->y);
    oldCell->status = CELL_EMPTY;
    oldCell->equipment = NULL;

    // 更新装备位置
    equipment->x = newX;
    equipment->y = newY;

    // 添加到新位置
    cell->equipment = equipment;
    cell->status = equipment->team == TEAM_RED ? CELL_OCCUPIED_RED : CELL_OCCUPIED_BLUE;
}

// 处理装备攻击
void handleAttack(Battlefield* battlefield, Equipment* equipment) {
    if (!equipment || !equipment->isActive || equipment->currentAmmo <= 0) {
        return;
    }

    // 查找最近的敌方装备
    Equipment* target = findNearestEnemy(battlefield, equipment);
    if (!target) {
        return;
    }

    // 计算距离
    int distance = calculateEquipmentDistance(equipment, target);

    // 检查是否可以攻击
    if (canAttack(equipment, target, distance)) {
        // 计算伤害
        int damage = calculateDamage(equipment, target);
        if (damage > 0) {
            // 减少目标生命值
            target->currentHealth -= damage;
            
            // 减少弹药量
            equipment->currentAmmo--;

            // 检查目标是否被摧毁
            if (target->currentHealth <= 0) {
                target->currentHealth = 0;
                target->isActive = 0;
                
                // 从战场移除
                removeEquipmentFromBattlefield(battlefield, target);
            }
        }
    }
}

// 检查是否有一方获胜
int checkVictory(Battlefield* battlefield) {
    int redActive = 0;
    int blueActive = 0;

    // 统计双方活跃装备数量
    for (int i = 0; i < battlefield->redCount; i++) {
        if (battlefield->redEquipments[i]->isActive) {
            redActive++;
        }
    }

    for (int i = 0; i < battlefield->blueCount; i++) {
        if (battlefield->blueEquipments[i]->isActive) {
            blueActive++;
        }
    }

    // 判断胜负
    if (redActive == 0 && blueActive > 0) {
        printf("\n蓝方获胜！\n");
        return 2; // 蓝方获胜
    } else if (blueActive == 0 && redActive > 0) {
        printf("\n红方获胜！\n");
        return 1; // 红方获胜
    } else if (redActive == 0 && blueActive == 0) {
        printf("\n平局！\n");
        return 3; // 平局
    }

    return 0; // 继续
}

// 模拟一步对抗
int simulateStep(Battlefield* battlefield) {
    // 处理红方装备
    for (int i = 0; i < battlefield->redCount; i++) {
        Equipment* equipment = battlefield->redEquipments[i];
        if (equipment->isActive) {
            handleMovement(battlefield, equipment);
            handleAttack(battlefield, equipment);
        }
    }

    // 处理蓝方装备
    for (int i = 0; i < battlefield->blueCount; i++) {
        Equipment* equipment = battlefield->blueEquipments[i];
        if (equipment->isActive) {
            handleMovement(battlefield, equipment);
            handleAttack(battlefield, equipment);
        }
    }

    // 检查胜负
    return checkVictory(battlefield);
} 