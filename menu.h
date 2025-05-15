#ifndef MENU_H
#define MENU_H

#include "battlefield.h"
#include "equipment.h"
#include "terrain.h"
#include "terrain_generation.h"

// 计算字符串的显示宽度（考虑中文字符占两个宽度）
int getStringDisplayWidth(const char* str);

// 显示主菜单
int showMainMenu();

// 显示WAR标题
void displayWarTitle();

// 清屏函数
void clearScreen();

// 获取控制台窗口宽度（字符数）
int getConsoleWidth();

// 在指定宽度内居中打印文本
void printCentered(const char* text, int width);

// 绘制自适应宽度的表格边框
void drawTableBorder(char left, char middle, char right, char line, int width);

// 绘制自适应宽度的表格内容行
void drawTableRow(const char* text, int width);

// 绘制多列表格边框
void drawMultiColumnTableBorder(int columnCount, int* columnWidths, int tableWidth);

// 绘制多列表格行
void drawMultiColumnTableRow(const char** texts, int columnCount, int* columnWidths, int* alignments);

// 战场模拟主程序
void startBattleSimulation();

// 显示装备列表菜单
void showEquipmentListMenu();

// 显示某方所有装备
void displayTeamEquipments(Team team);

// 显示单个装备详情，包括攻击范围可视化
void displayEquipmentDetails(int typeId);

// 绘制装备攻击范围
void drawAttackRange(int attackRadius);

// 等待按键
void waitForKeyPress();

// 显示地形生成菜单
void showTerrainGenerationMenu(Battlefield* battlefield);

// 获取噪声算法名称
const char* getNoiseAlgorithmName(NoiseAlgorithmType type);

// 显示噪声算法选择菜单
void showNoiseAlgorithmSelectionMenu(NoiseAlgorithmType* algorithmType);

// 显示地形图例
void displayTerrainLegend();

// 地形系统测试函数
void testTerrainSystem();

#endif // MENU_H 