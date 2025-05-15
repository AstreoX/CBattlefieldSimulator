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

    Equipment** enemyEquipments;
    int enemyCount;
    Equipment* enemyHQ = NULL;

    if (equipment->team == TEAM_RED) {
        enemyEquipments = battlefield->blueEquipments;
        enemyCount = battlefield->blueCount;
        enemyHQ = battlefield->blueHeadquarters;
    } else {
        enemyEquipments = battlefield->redEquipments;
        enemyCount = battlefield->redCount;
        enemyHQ = battlefield->redHeadquarters;
    }

    Equipment* nearest = NULL;
    int minDistance = INT_MAX;

    // 先考虑敌方指挥部（如果存在且激活）
    if (enemyHQ && enemyHQ->isActive) {
        int distance = calculateEquipmentDistance(equipment, enemyHQ);
        if (distance < minDistance) {
            minDistance = distance;
            nearest = enemyHQ;
        }
    }

    // 再遍历敌方普通装备，找到最近的一个
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

    // 是否发生碰撞的标志
    int collisionOccurred = hitLeft || hitRight || hitTop || hitBottom || hitEquipment;

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
    
    // 如果发生碰撞，向敌方大本营偏移一格
    if (collisionOccurred) {
        // 根据队伍确定敌方大本营位置
        Equipment* enemyHQ = NULL;
        if (equipment->team == TEAM_RED) {
            enemyHQ = battlefield->blueHeadquarters;
        } else {
            enemyHQ = battlefield->redHeadquarters;
        }

        if (enemyHQ && enemyHQ->isActive) {
            // 计算向敌方大本营的方向
            int dx = 0;
            int dy = 0;
            
            // 决定x方向的偏移
            if (enemyHQ->x > equipment->x) {
                dx = 1; // 向右偏移
            } else if (enemyHQ->x < equipment->x) {
                dx = -1; // 向左偏移
            }
            
            // 决定y方向的偏移
            if (enemyHQ->y > equipment->y) {
                dy = 1; // 向下偏移
            } else if (enemyHQ->y < equipment->y) {
                dy = -1; // 向上偏移
            }
            
            // 确定偏移后的位置
            int shiftX = equipment->x + dx;
            int shiftY = equipment->y + dy;
            
            // 检查偏移位置是否有效且为空
            if (isPositionValid(battlefield, shiftX, shiftY)) {
                Cell* shiftCell = getCell(battlefield, shiftX, shiftY);
                if (shiftCell->status == CELL_EMPTY) {
                    // 先从原位置移除
                    Cell* oldCell = getCell(battlefield, equipment->x, equipment->y);
                    oldCell->status = CELL_EMPTY;
                    oldCell->equipment = NULL;
                    
                    // 更新位置
                    equipment->x = shiftX;
                    equipment->y = shiftY;
                    
                    // 添加到新位置
                    shiftCell->equipment = equipment;
                    shiftCell->status = equipment->team == TEAM_RED ? CELL_OCCUPIED_RED : CELL_OCCUPIED_BLUE;
                    
                    // 已经移动，不需要继续常规移动
                    return;
                }
            }
        }
    }
    
    // 检查碰撞后的新位置（常规移动）
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

// 在控制台上绘制弹道
void drawProjectilePath(Battlefield* battlefield, Equipment* attacker, Equipment* target, int isHit) {
    if (!attacker || !target || !battlefield) {
        return;
    }
    
    // 创建一个临时战场数据结构来绘制弹道
    // 保存原始战场的单元格状态
    Cell** originalCells = (Cell**)malloc(battlefield->height * sizeof(Cell*));
    for (int i = 0; i < battlefield->height; i++) {
        originalCells[i] = (Cell*)malloc(battlefield->width * sizeof(Cell));
        for (int j = 0; j < battlefield->width; j++) {
            // 复制单元格状态
            originalCells[i][j] = battlefield->cells[i][j];
        }
    }
    
    // 使用Bresenham算法计算弹道路径
    int x1 = attacker->x;
    int y1 = attacker->y;
    int x2 = target->x;
    int y2 = target->y;
    
    // 计算直线坐标差异
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    int e2;
    
    // 计算所有弹道点位置，存储到数组中
    int maxPathLength = battlefield->width + battlefield->height; // 最大可能的路径长度
    int* pathX = (int*)malloc(maxPathLength * sizeof(int));
    int* pathY = (int*)malloc(maxPathLength * sizeof(int));
    int pathLength = 0;
    
    int currentX = x1;
    int currentY = y1;
    while (currentX != x2 || currentY != y2) {
        // 跳过起点
        if (currentX != x1 || currentY != y1) {
            // 只存储战场内的有效点
            if (isPositionValid(battlefield, currentX, currentY)) {
                pathX[pathLength] = currentX;
                pathY[pathLength] = currentY;
                pathLength++;
            }
        }
        
        // 计算下一个点
        e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            currentX += sx;
        }
        if (e2 < dx) {
            err += dx;
            currentY += sy;
        }
        
        // 防止无限循环
        if (pathLength >= maxPathLength - 1) {
            break;
        }
    }
    
    // 为终点添加特殊标记
    pathX[pathLength] = x2;
    pathY[pathLength] = y2;
    pathLength++;
    
    // 清屏以准备绘制战场和弹道
    system("cls");
    
    // 先绘制战场
    renderBattlefield(battlefield, TEAM_NONE);
    
    // 获取控制台句柄用于单独设置弹道字符颜色
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    WORD originalAttrs = consoleInfo.wAttributes;
    
    // 准确计算战场在控制台中的渲染位置
    // 根据战场渲染逻辑分析:
    // Line 0: 战场状态
    // Line 1: 预算信息
    // Line 2: 红方大本营血量
    // Line 3: 蓝方大本营血量
    // Line 4: X坐标十位
    // Line 5: X坐标个位
    // Line 6: 上边框
    // Line 7 onwards: Grid rows (e.g., "0 |cellcell...")
    int lineNumberWidth = 3; // Y坐标及"|" (e.g., "0 |" or "10|")
    int firstRowHeight = 7;  // 战场网格内容前的总行数

    // 直接绘制整条弹道线
    for (int i = 0; i < pathLength; i++) {
        int x = pathX[i];
        int y = pathY[i];
        
        // 跳过起点和被占用的非终点位置
        if ((x == x1 && y == y1) || 
            (i < pathLength - 1 && getCell(battlefield, x, y)->status != CELL_EMPTY)) {
            continue;
        }
        
        // 计算控制台坐标
        COORD pos = {lineNumberWidth + x, firstRowHeight + y};
        SetConsoleCursorPosition(hConsole, pos);
        
        // 根据攻击方队伍设置弹道字符颜色
        if (attacker->team == TEAM_RED) {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
        } else {
            SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        }
        
        // 绘制弹道字符
        if (i == pathLength - 1) {
            // 终点 - 命中效果
            printf("%s", isHit ? "X" : "O");
        } else {
            // 弹道轨迹
            printf("*");
        }
        
        // 恢复控制台颜色属性
        SetConsoleTextAttribute(hConsole, originalAttrs);
    }
    
    // 短暂停留以便观察
    Sleep(500);
    
    // 释放资源
    free(pathX);
    free(pathY);
    
    // 释放临时战场内存
    for (int i = 0; i < battlefield->height; i++) {
        free(originalCells[i]);
    }
    free(originalCells);
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

    // 检查是否可以攻击（在攻击范围内）
    if (canAttack(equipment, target, distance)) {
        // 获取交互数据
        EquipmentInteraction* interaction = getInteraction(equipment->typeId, target->typeId);
        if (!interaction) {
            return;
        }
        
        // 减少弹药量（无论是否命中都消耗弹药）
        equipment->currentAmmo--;
        
        // 根据命中率决定是否命中
        int randomValue = rand() % 100;
        int isHit = (randomValue < interaction->accuracy);
        
        // 绘制弹道（无论是否命中都显示弹道）
        drawProjectilePath(battlefield, equipment, target, isHit);
        
        // 只有命中才计算伤害
        if (isHit) {
            // 计算伤害
            int damage = calculateDamage(equipment, target);
            if (damage > 0) {
                // 减少目标生命值
                target->currentHealth -= damage;

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
}

// 检查是否有一方获胜
int checkVictory(Battlefield* battlefield) {
    int redActive = 0;
    int blueActive = 0;

    // 检查指挥部是否被摧毁
    if (battlefield->redHQDeployed && (!battlefield->redHeadquarters->isActive)) {
        printf("\n红方指挥部被摧毁！蓝方获胜！\n");
        return 2; // 蓝方获胜
    }
    
    if (battlefield->blueHQDeployed && (!battlefield->blueHeadquarters->isActive)) {
        printf("\n蓝方指挥部被摧毁！红方获胜！\n");
        return 1; // 红方获胜
    }

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
    // 先处理所有装备的移动
    // 处理红方装备移动
    for (int i = 0; i < battlefield->redCount; i++) {
        Equipment* equipment = battlefield->redEquipments[i];
        if (equipment->isActive) {
            handleMovement(battlefield, equipment);
        }
    }

    // 处理蓝方装备移动
    for (int i = 0; i < battlefield->blueCount; i++) {
        Equipment* equipment = battlefield->blueEquipments[i];
        if (equipment->isActive) {
            handleMovement(battlefield, equipment);
        }
    }
    
    // 再处理所有装备的攻击
    // 处理红方装备攻击
    for (int i = 0; i < battlefield->redCount; i++) {
        Equipment* equipment = battlefield->redEquipments[i];
        if (equipment->isActive) {
            handleAttack(battlefield, equipment);
        }
    }

    // 处理蓝方装备攻击
    for (int i = 0; i < battlefield->blueCount; i++) {
        Equipment* equipment = battlefield->blueEquipments[i];
        if (equipment->isActive) {
            handleAttack(battlefield, equipment);
        }
    }

    // 检查胜负
    return checkVictory(battlefield);
} 