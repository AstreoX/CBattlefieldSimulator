#include "battlefield.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

#define MAX_EQUIPMENTS_PER_TEAM 50
#define DEFAULT_BUDGET 10000

// 函数声明
void displayEquipmentInfo(Equipment* equipment);

// 初始化战场
void initBattlefield(Battlefield* battlefield, int width, int height) {
    battlefield->width = width;
    battlefield->height = height;
    
    // 分配单元格数组内存
    battlefield->cells = (Cell**)malloc(height * sizeof(Cell*));
    for (int i = 0; i < height; i++) {
        battlefield->cells[i] = (Cell*)malloc(width * sizeof(Cell));
        
        // 初始化单元格
        for (int j = 0; j < width; j++) {
            battlefield->cells[i][j].status = CELL_EMPTY;
            battlefield->cells[i][j].equipment = NULL;
        }
    }
    
    // 分配装备数组内存
    battlefield->maxEquipments = MAX_EQUIPMENTS_PER_TEAM;
    battlefield->redEquipments = (Equipment**)malloc(battlefield->maxEquipments * sizeof(Equipment*));
    battlefield->blueEquipments = (Equipment**)malloc(battlefield->maxEquipments * sizeof(Equipment*));
    
    // 初始化计数器和预算
    battlefield->redCount = 0;
    battlefield->blueCount = 0;
    battlefield->redBudget = 10000;
    battlefield->blueBudget = 10000;
    battlefield->redRemainingBudget = battlefield->redBudget;
    battlefield->blueRemainingBudget = battlefield->blueBudget;
    
    // 初始化指挥部
    battlefield->redHeadquarters = NULL;
    battlefield->blueHeadquarters = NULL;
    battlefield->redHQDeployed = 0;
    battlefield->blueHQDeployed = 0;
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
    printf("战场状态 (红方: %d, 蓝方: %d)\n", battlefield->redCount, battlefield->blueCount);
    printf("红方预算: %d/%d, 蓝方预算: %d/%d\n",
           battlefield->redRemainingBudget, battlefield->redBudget,
           battlefield->blueRemainingBudget, battlefield->blueBudget);

    // 打印X坐标标题
    printf("   ");
    for (int j = 0; j < battlefield->width; j++) {
        if (j % 10 == 0) {
            printf("%d", j / 10);
        } else {
            printf(" ");
        }
    }
    printf("\n");
    
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

    // 绘制战场
    for (int i = 0; i < battlefield->height; i++) {
        // 显示Y坐标
        if (i < 10) {
            printf("%d |", i);
        } else {
            printf("%d|", i);
        }
        
        for (int j = 0; j < battlefield->width; j++) {
            Cell* cell = getCell(battlefield, j, i);
            
            // 如果只查看指定方的单位，则根据条件显示
            if (viewOnly != TEAM_NONE) {
                // 如果格子是空的或者是非指定方的单位，则显示为空
                if (cell->status == CELL_EMPTY || 
                    (viewOnly == TEAM_RED && cell->status == CELL_OCCUPIED_BLUE) ||
                    (viewOnly == TEAM_BLUE && cell->status == CELL_OCCUPIED_RED)) {
                    
                    // 检查是否需要显示方向指示（只针对viewOnly方的单位）
                    int directionFound = 0;
                    if (cell->status == CELL_EMPTY) {
                        for (int dx = -1; dx <= 1 && !directionFound; dx++) {
                            for (int dy = -1; dy <= 1 && !directionFound; dy++) {
                                if (dx == 0 && dy == 0) continue;
                                
                                int nx = j + dx;
                                int ny = i + dy;
                                
                                if (isPositionValid(battlefield, nx, ny)) {
                                    Cell* neighborCell = getCell(battlefield, nx, ny);
                                    if (neighborCell->equipment && 
                                        neighborCell->equipment->team == viewOnly &&
                                        neighborCell->equipment->directionX == -dx && 
                                        neighborCell->equipment->directionY == -dy) {
                                        printf("%c", getDirectionChar(-dx, -dy));
                                        directionFound = 1;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    
                    if (!directionFound) {
                        printf(" ");
                    }
                    continue;
                }
            }
            
            if (cell->status == CELL_EMPTY) {
                // 检查周围8个方向是否有装备，并显示方向指示
                int directionFound = 0;
                for (int dx = -1; dx <= 1 && !directionFound; dx++) {
                    for (int dy = -1; dy <= 1 && !directionFound; dy++) {
                        if (dx == 0 && dy == 0) continue;
                        
                        int nx = j + dx;
                        int ny = i + dy;
                        
                        if (isPositionValid(battlefield, nx, ny)) {
                            Cell* neighborCell = getCell(battlefield, nx, ny);
                            if (neighborCell->equipment && 
                                (viewOnly == TEAM_NONE || neighborCell->equipment->team == viewOnly) &&
                                neighborCell->equipment->directionX == -dx && 
                                neighborCell->equipment->directionY == -dy) {
                                printf("%c", getDirectionChar(-dx, -dy));
                                directionFound = 1;
                                break;
                            }
                        }
                    }
                }
                
                if (!directionFound) {
                    printf(" ");
                }
            } else if (cell->status == CELL_OCCUPIED_RED) {
                if (cell->equipment && cell->equipment->isActive) {
                    // 根据装备类型显示不同字符
                    switch (cell->equipment->typeId) {
                        case 1: printf("T"); break; // 坦克
                        case 2: printf("A"); break; // 飞机
                        case 3: printf("C"); break; // 火炮
                        case 4: printf("M"); break; // 导弹
                        case 5: printf("S"); break; // 士兵
                        case 6: printf("G"); break; // 枪塔
                        case 7: printf("#"); break; // 栅栏
                        case 8: printf("V"); break; // 装甲车
                        case 9: printf("H"); break; // 重型机枪
                        case 10: printf("K"); break; // 反坦克炮
                        default: printf("R"); break;
                    }
                } else {
                    printf("x"); // 已摧毁
                }
            } else if (cell->status == CELL_OCCUPIED_BLUE) {
                if (cell->equipment && cell->equipment->isActive) {
                    // 根据装备类型显示不同字符
                    switch (cell->equipment->typeId) {
                        case 1: printf("t"); break; // 坦克
                        case 2: printf("a"); break; // 飞机
                        case 3: printf("c"); break; // 火炮
                        case 4: printf("m"); break; // 导弹
                        case 5: printf("s"); break; // 士兵
                        case 6: printf("g"); break; // 枪塔
                        case 7: printf("#"); break; // 栅栏
                        case 8: printf("v"); break; // 装甲车
                        case 9: printf("h"); break; // 重型机枪
                        case 10: printf("k"); break; // 反坦克炮
                        default: printf("b"); break;
                    }
                } else {
                    printf("x"); // 已摧毁
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