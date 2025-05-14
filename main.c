#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <conio.h>
#include "battlefield.h"
#include "equipment.h"
#include "simulation.h"
#include "menu.h"

// Forward declarations
int simulateStep(Battlefield* battlefield); // Make sure simulateStep declaration is consistent

int main() {
    // 设置控制台代码页为UTF-8
    SetConsoleOutputCP(65001);
    
    srand((unsigned int)time(NULL));
    
    // 显示主菜单
    return showMainMenu();
    
    /* 原直接部署的代码已被替换为菜单系统
    // 初始化战场
    Battlefield battlefield;
    initBattlefield(&battlefield, 80, 60);
    
    // 加载装备
    loadEquipmentTypes("equipment_types.txt");
    loadEquipmentInteractions("equipment_interactions.txt");
    
    // 部署红蓝双方装备
    deployEquipment(&battlefield, TEAM_RED);
    deployEquipment(&battlefield, TEAM_BLUE);
    
    printf("\n双方部署完成，按任意键开始战斗模拟...\n");
    getch();
    
    // 开始战斗模拟
    while (1) {
        system("cls"); // Windows清屏命令
        renderBattlefield(&battlefield, TEAM_NONE); // 战斗阶段显示所有装备
        
        if (simulateStep(&battlefield)) {
            break; // 一方获胜，模拟结束
        }
        
        // 等待一段时间以便观察
        Sleep(500); // Windows系统中的Sleep函数，单位为毫秒
    }
    
    // 最后显示一次战场状态
    system("cls");
    renderBattlefield(&battlefield, TEAM_NONE);
    
    printf("\n模拟结束！按任意键退出...\n");
    getch();
    
    // 释放资源
    freeBattlefield(&battlefield);
    freeEquipmentTypes();
    */
    
    return 0;
} 