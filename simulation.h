#ifndef SIMULATION_H
#define SIMULATION_H

#include "battlefield.h"

// 模拟一步对抗
// 返回值：0表示继续，1表示游戏结束
int simulateStep(Battlefield* battlefield);

// 处理装备移动
void handleMovement(Battlefield* battlefield, Equipment* equipment);

// 处理装备攻击
void handleAttack(Battlefield* battlefield, Equipment* equipment);

// 检查是否有一方获胜
// 返回值：0表示没有，1表示红方获胜，2表示蓝方获胜
int checkVictory(Battlefield* battlefield);

// 查找最近的敌方装备
Equipment* findNearestEnemy(Battlefield* battlefield, Equipment* equipment);

// 计算两个装备之间的距离
int calculateEquipmentDistance(Equipment* e1, Equipment* e2);

#endif // SIMULATION_H 