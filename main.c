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
#include "terrain.h"
#include "noise.h"
#include "terrain_generation.h"
#include "terrain_renderer.h"

// Forward declarations
int simulateStep(Battlefield* battlefield); // Make sure simulateStep declaration is consistent

// 地形系统测试函数
void testTerrainSystem() {
    system("cls");
    printf("地形系统测试\n");
    printf("===================\n\n");
    
    // 初始化地形渲染器
    initTerrainRenderer();
    
    // 创建地形地图（120x60）
    int width = 120;
    int height = 60;
    Terrain** terrainMap = (Terrain**)malloc(height * sizeof(Terrain*));
    initTerrain(width, height, terrainMap);
    
    // 创建地形生成配置
    TerrainGeneratorConfig config = getDefaultTerrainConfig();
    config.seed = (int)time(NULL);
    
    // 演示不同的地形生成算法
    printf("请选择地形生成算法:\n");
    printf("1. 柏林噪声 (Perlin Noise)\n");
    printf("2. 分形布朗运动 (Fractal Brownian Motion)\n");
    printf("3. 域扭曲 (Domain Warping)\n");
    printf("4. 山脊多重分形 (Ridged Multi)\n");
    printf("5. 单纯形噪声 (Simplex Noise)\n");
    printf("选择: ");
    
    int choice;
    scanf("%d", &choice);
    
    NoiseAlgorithmType algorithmType;
    switch (choice) {
        case 1: algorithmType = NOISE_PERLIN; break;
        case 2: algorithmType = NOISE_FRACTAL_BROWNIAN; break;
        case 3: algorithmType = NOISE_DOMAIN_WARPING; break;
        case 4: algorithmType = NOISE_RIDGED_MULTI; break;
        case 5: algorithmType = NOISE_SIMPLEX; break;
        default: algorithmType = NOISE_FRACTAL_BROWNIAN; break;
    }
    
    // 生成地形
    generateTerrainWithAlgorithm(width, height, terrainMap, algorithmType, config);
    
    // 添加一些地形特性
    // 生成河流
    printf("生成河流...\n");
    generateRivers(width, height, terrainMap, 5, 20, 100);
    
    // 生成悬崖
    printf("生成悬崖...\n");
    generateCliffs(width, height, terrainMap, 2.0f);
    
    // 平滑地形
    printf("平滑地形...\n");
    smoothTerrain(width, height, terrainMap, 1);
    
    // 渲染地形
    printf("\n选择渲染模式:\n");
    printf("1. 普通模式\n");
    printf("2. 高度图模式\n");
    printf("3. 等高线图模式\n");
    printf("4. 湿度图模式\n");
    printf("5. 温度图模式\n");
    printf("6. 显示直方图\n");
    printf("7. 3D视图\n");
    printf("选择: ");
    
    scanf("%d", &choice);
    
    system("cls");
    printf("地形渲染结果:\n\n");
    
    TerrainDisplayConfig displayConfig = getDefaultDisplayConfig();
    TerrainRenderMode renderMode;
    
    switch (choice) {
        case 1:
            renderMode = RENDER_MODE_NORMAL;
            // 普通模式渲染
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    renderTerrainCell(&terrainMap[y][x], displayConfig, renderMode);
                }
                printf("\n");
            }
            break;
        case 2:
            renderMode = RENDER_MODE_HEIGHT_MAP;
            // 高度图模式渲染
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    renderTerrainCell(&terrainMap[y][x], displayConfig, renderMode);
                }
                printf("\n");
            }
            break;
        case 3:
            // 等高线图模式
            renderTopographicMap(width, height, terrainMap, 1.0f);
            break;
        case 4:
            // 湿度图模式
            renderHeatMap(width, height, terrainMap, RENDER_MODE_MOISTURE_MAP);
            break;
        case 5:
            // 温度图模式
            renderHeatMap(width, height, terrainMap, RENDER_MODE_TEMPERATURE_MAP);
            break;
        case 6:
            // 显示高度分布直方图
            renderTerrainHistogram(width, height, terrainMap);
            break;
        case 7:
            // 3D视图
            renderTerrain3DView(width, height, terrainMap, 45, 10);
            break;
        default:
            renderMode = RENDER_MODE_NORMAL;
            // 普通模式渲染
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    renderTerrainCell(&terrainMap[y][x], displayConfig, renderMode);
                }
                printf("\n");
            }
            break;
    }
    
    printf("\n按任意键返回主菜单...");
    getch();
    
    // 释放地形资源
    freeTerrain(height, terrainMap);
    free(terrainMap);
}

int main() {
    // 设置控制台代码页为UTF-8
    SetConsoleOutputCP(65001);
    
    srand((unsigned int)time(NULL));
    
    // 测试地形系统
    testTerrainSystem();
    
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