#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
#include <math.h>
#include <time.h>
#include "simulation.h"

// 计算字符串的显示宽度（考虑中文字符占两个宽度）
int getStringDisplayWidth(const char* str) {
    int width = 0;
    
    while (*str) {
        // 检查是否是中文字符（UTF-8编码）
        if ((unsigned char)*str > 127) {
            // 中文字符占两个宽度
            width += 2;
            // 跳过UTF-8多字节字符的剩余字节
            if ((unsigned char)*str >= 0xE0) {
                str += 3; // 3字节UTF-8字符（大多数中文）
            } else if ((unsigned char)*str >= 0xC0) {
                str += 2; // 2字节UTF-8字符
            } else {
                str++; // 单字节
            }
        } else {
            // ASCII字符占一个宽度
            width += 1;
            str++;
        }
    }
    
    return width;
}

// 清屏函数
void clearScreen() {
    system("cls");
}

// 等待按键
void waitForKeyPress() {
    printf("\n按任意键继续...\n");
    getch();
}

// 获取控制台窗口宽度（字符数）
int getConsoleWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int width = 80; // 默认宽度
    
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    
    return width;
}

// 在指定宽度内居中打印文本
void printCentered(const char* text, int width) {
    int textLength = strlen(text);
    int padding = (width - textLength) / 2;
    
    if (padding < 0) padding = 0;
    
    // 打印前置空格进行居中
    for (int i = 0; i < padding; i++) {
        printf(" ");
    }
    
    // 打印文本
    printf("%s", text);
    
    // 如果文本长度是奇数，确保总长度正确
    if ((width - textLength) % 2 != 0) {
        padding++;
    }
    
    // 打印后置空格完成对齐
    for (int i = 0; i < padding; i++) {
        printf(" ");
    }
}

// 改进版绘制自适应宽度的表格边框
void drawTableBorder(char left, char middle, char right, char line, int width) {
    // 标记未使用的参数以消除警告
    (void)middle;
    
    printf("%c", left);
    for (int i = 0; i < width - 2; i++) {
        printf("%c", line);
    }
    printf("%c\n", right);
}

// 改进版绘制自适应宽度的表格内容行
void drawTableRow(const char* text, int width) {
    int displayWidth = getStringDisplayWidth(text);
    int contentWidth = width - 4; // 减去边框和空格
    int padding = (contentWidth - displayWidth) / 2;
    
    if (padding < 0) padding = 0;
    
    printf("| ");
    
    // 前置空格
    for (int i = 0; i < padding; i++) {
        printf(" ");
    }
    
    // 文本内容
    printf("%s", text);
    
    // 后置空格 - 确保总宽度精确匹配
    int remainingSpaces = contentWidth - displayWidth - padding;
    for (int i = 0; i < remainingSpaces; i++) {
        printf(" ");
    }
    
    printf(" |\n");
}

// 绘制多列表格边框
void drawMultiColumnTableBorder(int columnCount, int* columnWidths, int tableWidth) {
    // 标记参数为有意未使用
    (void)tableWidth;
    
    printf("+");
    
    for (int i = 0; i < columnCount; i++) {
        for (int j = 0; j < columnWidths[i]; j++) {
            printf("-");
        }
        
        if (i < columnCount - 1) {
            printf("+");
        }
    }
    
    printf("+\n");
}

// 改进版绘制多列表格行（处理中文字符宽度）
void drawMultiColumnTableRow(const char** texts, int columnCount, int* columnWidths, int* alignments) {
    printf("|");
    
    for (int i = 0; i < columnCount; i++) {
        int displayWidth = getStringDisplayWidth(texts[i]);
        int cellWidth = columnWidths[i] - 2; // 减去每列两边的空格
        
        // 左对齐 (0)、居中对齐 (1) 或右对齐 (2)
        printf(" ");
        if (alignments[i] == 1) { // 居中对齐
            int padding = (cellWidth - displayWidth) / 2;
            
            for (int j = 0; j < padding; j++) {
                printf(" ");
            }
            
            printf("%s", texts[i]);
            
            // 确保宽度精确匹配
            for (int j = 0; j < cellWidth - displayWidth - padding; j++) {
                printf(" ");
            }
        } else if (alignments[i] == 2) { // 右对齐
            int padding = cellWidth - displayWidth;
            
            for (int j = 0; j < padding; j++) {
                printf(" ");
            }
            
            printf("%s", texts[i]);
        } else { // 默认左对齐
            printf("%s", texts[i]);
            
            for (int j = 0; j < cellWidth - displayWidth; j++) {
                printf(" ");
            }
        }
        printf(" ");
        
        if (i < columnCount - 1) {
            printf("|");
        }
    }
    
    printf("|\n");
}

// 显示WAR标题
void displayWarTitle() {
    int consoleWidth = getConsoleWidth();
    printf("\n");
    
    // 检查控制台是否有足够宽度显示完整ASCII艺术标题
    if (consoleWidth >= 105) {
        // 大尺寸标题，居中显示
        int padding = (consoleWidth - 102) / 2;
        char* spaces = (char*)malloc(padding + 1);
        memset(spaces, ' ', padding);
        spaces[padding] = '\0';

        printf("%s██╗    ██╗ █████╗ ██████╗     ███████╗██╗███╗   ███╗██╗   ██╗██╗      █████╗ ████████╗ ██████╗ ██████╗ \n", spaces);
        printf("%s██║    ██║██╔══██╗██╔══██╗    ██╔════╝██║████╗ ████║██║   ██║██║     ██╔══██╗╚══██╔══╝██╔═══██╗██╔══██╗\n", spaces);
        printf("%s██║ █╗ ██║███████║██████╔╝    ███████╗██║██╔████╔██║██║   ██║██║     ███████║   ██║   ██║   ██║██████╔╝\n", spaces);
        printf("%s██║███╗██║██╔══██║██╔══██╗    ╚════██║██║██║╚██╔╝██║██║   ██║██║     ██╔══██║   ██║   ██║   ██║██╔══██╗\n", spaces);
        printf("%s╚███╔███╔╝██║  ██║██║  ██║    ███████║██║██║ ╚═╝ ██║╚██████╔╝███████╗██║  ██║   ██║   ╚██████╔╝██║  ██║\n", spaces);
        printf("%s ╚══╝╚══╝ ╚═╝  ╚═╝╚═╝  ╚═╝    ╚══════╝╚═╝╚═╝     ╚═╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝\n", spaces);
        
        free(spaces);
    } else if (consoleWidth >= 60) {
        // 中等尺寸标题，使用更简洁的样式
        printCentered("██     ██  █████  ██████      ███████ ██ ███    ███ ██    ██ ██       █████  ████████  ██████  ██████  ", consoleWidth);
        printf("\n");
        printCentered("██     ██ ██   ██ ██   ██     ██      ██ ████  ████ ██    ██ ██      ██   ██    ██    ██    ██ ██   ██ ", consoleWidth);
        printf("\n");
        printCentered("██  █  ██ ███████ ██████      ███████ ██ ██ ████ ██ ██    ██ ██      ███████    ██    ██    ██ ██████  ", consoleWidth);
        printf("\n");
        printCentered("██ ███ ██ ██   ██ ██   ██          ██ ██ ██  ██  ██ ██    ██ ██      ██   ██    ██    ██    ██ ██   ██ ", consoleWidth);
        printf("\n");
        printCentered(" ███ ███  ██   ██ ██   ██     ███████ ██ ██      ██  ██████  ███████ ██   ██    ██     ██████  ██   ██ ", consoleWidth);
        printf("\n");
    } else {
        // 小尺寸标题，使用更紧凑的样式
        printCentered("WAR SIMULATOR", consoleWidth);
        printf("\n");
    }
    
    printf("\n");
    
    // 子标题
    int boxWidth = consoleWidth > 70 ? 50 : consoleWidth - 10;
    if (boxWidth < 40) boxWidth = 40;
    
    drawTableBorder('+', '+', '+', '-', boxWidth);
    drawTableRow("战场模拟器 - 兵力部署和战术对抗的模拟演练", boxWidth);
    drawTableBorder('+', '+', '+', '-', boxWidth);
    
    printf("\n");
}

// 显示主菜单
int showMainMenu() {
    int choice = 0;
    
    while (1) {
        clearScreen();
        displayWarTitle();
        
        int width = getConsoleWidth();
        
        drawTableBorder('+', '+', '+', '-', width);
        drawTableRow("战场模拟器主菜单", width);
        drawTableBorder('+', '+', '+', '-', width);
        drawTableRow("1. 开始战场模拟", width);
        drawTableRow("2. 查看装备列表", width);
        drawTableRow("3. 地形系统测试", width);
        drawTableRow("0. 退出程序", width);
        drawTableBorder('+', '+', '+', '-', width);
        
        printf("\n请选择操作: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                // 开始战场模拟
                startBattleSimulation();
                break;
            case 2:
                // 显示装备列表
                showEquipmentListMenu();
                break;
            case 3:
                // 地形系统测试
                testTerrainSystem();
                break;
            case 0:
                // 退出程序
                return 0;
            default:
                printf("\n无效选择，请重新输入!\n");
                waitForKeyPress();
                break;
        }
    }
    
    return 0;
}

// 显示装备列表菜单
void showEquipmentListMenu() {
    int choice = 0;
    
    while (1) {
        clearScreen();
        printf("\n");
        
        int consoleWidth = getConsoleWidth();
        int tableWidth = consoleWidth > 70 ? 50 : consoleWidth - 10;
        if (tableWidth < 30) tableWidth = 30;
        
        drawTableBorder('+', '+', '+', '-', tableWidth);
        drawTableRow("武器装备列表", tableWidth);
        drawTableBorder('+', '+', '+', '-', tableWidth);
        
        // 使用表格样式显示菜单选项，保持一致的对齐
        printf("\n");
        drawTableBorder('+', '+', '+', '-', tableWidth);
        drawTableRow("1. 查看红方武器装备", tableWidth);
        drawTableRow("2. 查看蓝方武器装备", tableWidth);
        drawTableRow("0. 返回主菜单", tableWidth);
        drawTableBorder('+', '+', '+', '-', tableWidth);
        
        printf("\n请选择: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                displayTeamEquipments(TEAM_RED);
                break;
            case 2:
                displayTeamEquipments(TEAM_BLUE);
                break;
            case 0:
                return;
            default:
                printf("无效选择，请重新输入！\n");
                waitForKeyPress();
        }
    }
}

// 显示某方所有装备
void displayTeamEquipments(Team team) {
    int choice = 0;
    
    // 确保已经加载装备数据
    if (g_equipmentTypesCount == 0) {
        loadEquipmentTypes("equipment_types.txt");
        loadEquipmentInteractions("equipment_interactions.txt");
    }
    
    while (1) {
        clearScreen();
        printf("\n");
        
        int consoleWidth = getConsoleWidth();
        int tableWidth = consoleWidth > 70 ? 70 : consoleWidth - 4;
        if (tableWidth < 40) tableWidth = 40;
        
        // 显示标题
        char title[40];
        sprintf(title, "%s方武器装备", team == TEAM_RED ? "红" : "蓝");
        
        drawTableBorder('+', '+', '+', '-', tableWidth);
        drawTableRow(title, tableWidth);
        drawTableBorder('+', '+', '+', '-', tableWidth);
        
        printf("\n");
        
        // 定义列宽和对齐方式
        int columnCount;
        int* columnWidths;
        int* alignments;
        
        if (consoleWidth > 80) {
            columnCount = 7;
            columnWidths = (int*)malloc(columnCount * sizeof(int));
            alignments = (int*)malloc(columnCount * sizeof(int));
            
            // 计算表格内有效宽度 (减去左右边框和列分隔符)
            // 表格宽度 = 左边框(1) + 列宽之和 + 列分隔符数量(columnCount-1) + 右边框(1)
            int availableWidth = tableWidth - 2 - (columnCount - 1);
            
            // 按比例分配各列宽度
            columnWidths[0] = (int)(availableWidth * 0.08);  // ID
            columnWidths[1] = (int)(availableWidth * 0.30);  // 名称
            columnWidths[2] = (int)(availableWidth * 0.12);  // 造价
            columnWidths[3] = (int)(availableWidth * 0.12);  // 生命值
            columnWidths[4] = (int)(availableWidth * 0.12);  // 速度
            columnWidths[5] = (int)(availableWidth * 0.12);  // 攻击范围
            
            // 最后一列占用剩余空间，确保总宽度匹配
            columnWidths[6] = availableWidth - columnWidths[0] - columnWidths[1] 
                             - columnWidths[2] - columnWidths[3] - columnWidths[4] 
                             - columnWidths[5];
            
            // 对齐方式：0=左对齐，1=居中，2=右对齐
            alignments[0] = 2; // ID - 右对齐
            alignments[1] = 0; // 名称 - 左对齐
            alignments[2] = 2; // 造价 - 右对齐
            alignments[3] = 2; // 生命值 - 右对齐
            alignments[4] = 2; // 速度 - 右对齐
            alignments[5] = 2; // 攻击范围 - 右对齐
            alignments[6] = 2; // 弹药 - 右对齐
        } else {
            columnCount = 5;
            columnWidths = (int*)malloc(columnCount * sizeof(int));
            alignments = (int*)malloc(columnCount * sizeof(int));
            
            // 计算表格内有效宽度 (减去左右边框和列分隔符)
            int availableWidth = tableWidth - 2 - (columnCount - 1);
            
            // 按比例分配各列宽度
            columnWidths[0] = (int)(availableWidth * 0.10);  // ID
            columnWidths[1] = (int)(availableWidth * 0.35);  // 名称
            columnWidths[2] = (int)(availableWidth * 0.15);  // 造价
            columnWidths[3] = (int)(availableWidth * 0.15);  // 生命值
            
            // 最后一列占用剩余空间，确保总宽度匹配
            columnWidths[4] = availableWidth - columnWidths[0] - columnWidths[1] 
                             - columnWidths[2] - columnWidths[3];
            
            // 对齐方式
            alignments[0] = 2; // ID - 右对齐
            alignments[1] = 0; // 名称 - 左对齐
            alignments[2] = 2; // 造价 - 右对齐
            alignments[3] = 2; // 生命值 - 右对齐
            alignments[4] = 2; // 攻击范围 - 右对齐
        }
        
        // 显示表头
        drawMultiColumnTableBorder(columnCount, columnWidths, tableWidth);
        
        // 准备表头文本
        const char** headerTexts = (const char**)malloc(columnCount * sizeof(char*));
        
        headerTexts[0] = "ID";
        headerTexts[1] = "名称";
        headerTexts[2] = "造价";
        headerTexts[3] = "生命值";
        
        if (consoleWidth > 80) {
            headerTexts[4] = "速度";
            headerTexts[5] = "攻击范围";
            headerTexts[6] = "弹药";
        } else {
            headerTexts[4] = "攻击范围";
        }
        
        // 显示表头
        drawMultiColumnTableRow(headerTexts, columnCount, columnWidths, alignments);
        
        // 显示分隔线
        drawMultiColumnTableBorder(columnCount, columnWidths, tableWidth);
        
        // 显示所有可用装备
        for (int i = 0; i < g_equipmentTypesCount; i++) {
            EquipmentType* type = &g_equipmentTypes[i];
            
            // 准备行数据
            const char** rowTexts = (const char**)malloc(columnCount * sizeof(char*));
            char** textBuffers = (char**)malloc(columnCount * sizeof(char*));
            
            for (int j = 0; j < columnCount; j++) {
                textBuffers[j] = (char*)malloc(32 * sizeof(char));
            }
            
            // ID
            sprintf(textBuffers[0], "%d", type->typeId);
            // 名称
            sprintf(textBuffers[1], "%s", type->name);
            // 造价
            sprintf(textBuffers[2], "%d", type->cost);
            // 生命值
            sprintf(textBuffers[3], "%d", type->maxHealth);
            
            if (consoleWidth > 80) {
                // 速度
                sprintf(textBuffers[4], "%d", type->maxSpeed);
                // 攻击范围
                sprintf(textBuffers[5], "%d", type->maxAttackRadius);
                // 弹药
                sprintf(textBuffers[6], "%d", type->maxAmmo);
            } else {
                // 攻击范围
                sprintf(textBuffers[4], "%d", type->maxAttackRadius);
            }
            
            // 设置引用
            for (int j = 0; j < columnCount; j++) {
                rowTexts[j] = textBuffers[j];
            }
            
            // 显示行
            drawMultiColumnTableRow(rowTexts, columnCount, columnWidths, alignments);
            
            // 释放内存
            for (int j = 0; j < columnCount; j++) {
                free(textBuffers[j]);
            }
            free(textBuffers);
            free(rowTexts);
        }
        
        // 底部边框
        drawMultiColumnTableBorder(columnCount, columnWidths, tableWidth);
        
        // 释放列宽和对齐内存
        free(columnWidths);
        free(alignments);
        free(headerTexts);
        
        printf("\n0. 返回上级菜单\n");
        printf("\n请选择要查看详情的装备ID: ");
        scanf("%d", &choice);
        
        if (choice == 0) {
            return;
        } else {
            // 显示选中装备的详情
            int validChoice = 0;
            for (int i = 0; i < g_equipmentTypesCount; i++) {
                if (g_equipmentTypes[i].typeId == choice) {
                    displayEquipmentDetails(choice);
                    validChoice = 1;
                    break;
                }
            }
            
            if (!validChoice) {
                printf("无效的装备ID！\n");
                waitForKeyPress();
            }
        }
    }
}

// 显示单个装备详情，包括攻击范围可视化
void displayEquipmentDetails(int typeId) {
    clearScreen();
    
    EquipmentType* type = getEquipmentTypeById(typeId);
    if (!type) {
        printf("找不到ID为%d的装备！\n", typeId);
        waitForKeyPress();
        return;
    }
    
    int consoleWidth = getConsoleWidth();
    int tableWidth = consoleWidth > 70 ? 70 : consoleWidth - 4;
    
    if (tableWidth < 40) tableWidth = 40; // 最小宽度
    
    // 顶部边框
    drawTableBorder('+', '+', '+', '-', tableWidth);
    drawTableRow("装备详细信息", tableWidth);
    drawTableBorder('+', '+', '+', '-', tableWidth);
    
    // 使用表格形式显示装备详细属性
    // 定义两列：属性名和属性值
    int columnCount = 2;
    int* columnWidths = (int*)malloc(columnCount * sizeof(int));
    int* alignments = (int*)malloc(columnCount * sizeof(int));
    
    // 计算表格内有效宽度 (减去左右边框和列分隔符)
    // 表格宽度 = 左边框(1) + 列1宽度 + 列分隔符(1) + 列2宽度 + 右边框(1)
    int availableWidth = tableWidth - 3; // 减去2个边框和1个列分隔符
    
    // 第一列放属性名（左对齐），第二列放属性值（左对齐）
    columnWidths[0] = availableWidth / 3;
    columnWidths[1] = availableWidth - columnWidths[0];
    
    alignments[0] = 0; // 属性名 - 左对齐
    alignments[1] = 0; // 属性值 - 左对齐
    
    // 准备所有属性名和值
    char idBuffer[32], nameBuffer[64], costBuffer[32], healthBuffer[32];
    char speedBuffer[32], radiusBuffer[32], ammoBuffer[32], rateBuffer[32], flyBuffer[32];
    
    sprintf(idBuffer, "%d", type->typeId);
    sprintf(nameBuffer, "%s", type->name);
    sprintf(costBuffer, "%d", type->cost);
    sprintf(healthBuffer, "%d", type->maxHealth);
    sprintf(speedBuffer, "%d", type->maxSpeed);
    sprintf(radiusBuffer, "%d", type->maxAttackRadius);
    sprintf(ammoBuffer, "%d", type->maxAmmo);
    sprintf(rateBuffer, "%d", type->maxFireRate);
    sprintf(flyBuffer, "%s", type->canFly ? "是" : "否");
    
    // 显示每个属性行
    const char* row1[2] = {"装备ID:", idBuffer};
    const char* row2[2] = {"名称:", nameBuffer};
    const char* row3[2] = {"造价:", costBuffer};
    const char* row4[2] = {"生命值:", healthBuffer};
    const char* row5[2] = {"移动速度:", speedBuffer};
    const char* row6[2] = {"攻击范围:", radiusBuffer};
    const char* row7[2] = {"弹药量:", ammoBuffer};
    const char* row8[2] = {"射速:", rateBuffer};
    const char* row9[2] = {"可飞行:", flyBuffer};
    
    drawMultiColumnTableBorder(columnCount, columnWidths, tableWidth);
    drawMultiColumnTableRow(row1, columnCount, columnWidths, alignments);
    drawMultiColumnTableRow(row2, columnCount, columnWidths, alignments);
    drawMultiColumnTableRow(row3, columnCount, columnWidths, alignments);
    drawMultiColumnTableRow(row4, columnCount, columnWidths, alignments);
    drawMultiColumnTableRow(row5, columnCount, columnWidths, alignments);
    drawMultiColumnTableRow(row6, columnCount, columnWidths, alignments);
    drawMultiColumnTableRow(row7, columnCount, columnWidths, alignments);
    drawMultiColumnTableRow(row8, columnCount, columnWidths, alignments);
    drawMultiColumnTableRow(row9, columnCount, columnWidths, alignments);
    drawMultiColumnTableBorder(columnCount, columnWidths, tableWidth);
    
    // 释放内存
    free(columnWidths);
    free(alignments);
    
    printf("\n攻击范围可视化 (每个单元格表示一个地图格子，X表示装备位置):\n\n");
    drawAttackRange(type->maxAttackRadius);
    
    printf("\n该装备对其他装备的伤害和命中率:\n");
    
    // 定义列宽和对齐方式
    columnCount = 3;
    columnWidths = (int*)malloc(columnCount * sizeof(int));
    alignments = (int*)malloc(columnCount * sizeof(int));
    
    // 计算表格内有效宽度 (减去左右边框和列分隔符)
    // 表格宽度 = 左边框(1) + 列1宽度 + 列分隔符(1) + 列2宽度 + 列分隔符(1) + 列3宽度 + 右边框(1)
    availableWidth = tableWidth - 5; // 减去2个边框和3个列分隔符
    
    // 按比例分配列宽
    columnWidths[0] = (int)(availableWidth * 0.5); // 目标装备 50%
    columnWidths[1] = (int)(availableWidth * 0.2); // 单发伤害 20%
    columnWidths[2] = availableWidth - columnWidths[0] - columnWidths[1]; // 命中率 剩余部分
    
    // 对齐方式
    alignments[0] = 0; // 目标装备 - 左对齐
    alignments[1] = 2; // 单发伤害 - 右对齐
    alignments[2] = 2; // 命中率 - 右对齐
    
    // 表头文本
    const char* headerTexts[] = {"目标装备", "单发伤害", "命中率"};
    
    // 绘制表头
    drawMultiColumnTableBorder(columnCount, columnWidths, tableWidth);
    drawMultiColumnTableRow(headerTexts, columnCount, columnWidths, alignments);
    drawMultiColumnTableBorder(columnCount, columnWidths, tableWidth);
    
    // 绘制内容行
    for (int i = 0; i < g_equipmentTypesCount; i++) {
        int targetId = g_equipmentTypes[i].typeId;
        EquipmentInteraction* interaction = getInteraction(typeId, targetId);
        
        // 准备行数据
        char damageBuffer[32] = {0};
        char accuracyBuffer[32] = {0};
        
        // 设置数据
        if (interaction) {
            sprintf(damageBuffer, "%d", interaction->damage);
            sprintf(accuracyBuffer, "%d", interaction->accuracy);
        } else {
            sprintf(damageBuffer, "未知");
            sprintf(accuracyBuffer, "未知");
        }
        
        const char* rowTexts[] = {g_equipmentTypes[i].name, damageBuffer, accuracyBuffer};
        
        // 显示行
        drawMultiColumnTableRow(rowTexts, columnCount, columnWidths, alignments);
    }
    
    // 底部边框
    drawMultiColumnTableBorder(columnCount, columnWidths, tableWidth);
    
    // 释放内存
    free(columnWidths);
    free(alignments);
    
    waitForKeyPress();
}

// 绘制装备攻击范围
void drawAttackRange(int attackRadius) {
    int size = attackRadius * 2 + 1;
    int center = attackRadius;
    
    // 创建二维字符数组
    char** grid = (char**)malloc(size * sizeof(char*));
    for (int i = 0; i < size; i++) {
        grid[i] = (char*)malloc(size * sizeof(char));
        for (int j = 0; j < size; j++) {
            grid[i][j] = ' ';
        }
    }
    
    // 标记中心点
    grid[center][center] = 'X';
    
    // 标记攻击范围
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int dx = x - center;
            int dy = y - center;
            int distance = (int)sqrt(dx*dx + dy*dy);
            
            if (distance <= attackRadius && grid[y][x] == ' ') {
                grid[y][x] = '.';
            }
        }
    }
    
    // 打印攻击范围
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("%c ", grid[i][j]);
        }
        printf("\n");
    }
    
    // 释放内存
    for (int i = 0; i < size; i++) {
        free(grid[i]);
    }
    free(grid);
}

// 战场模拟主程序
void startBattleSimulation() {
    clearScreen();
    
    // 初始化战场 - 扩大为120列60行
    Battlefield battlefield;
    initBattlefield(&battlefield, 120, 60);
    
    // 加载装备
    loadEquipmentTypes("equipment_types.txt");
    loadEquipmentInteractions("equipment_interactions.txt");
    
    // 提示是否生成地形
    printf("是否生成地形? (1=是, 0=否): ");
    int useTerrain;
    scanf("%d", &useTerrain);
    
    if (useTerrain) {
        // 询问是否使用高级地形选项
        printf("是否使用高级地形选项? (1=是, 0=否): ");
        int useAdvancedOptions;
        scanf("%d", &useAdvancedOptions);
        
        if (useAdvancedOptions) {
            // 使用地形生成菜单
            showTerrainGenerationMenu(&battlefield);
        } else {
            // 使用默认比例尺和随机种子生成地形
            float scale = 10.0f;
            int seed = (int)time(NULL);
            printf("使用默认设置生成地形...\n");
            generateBattlefieldTerrain(&battlefield, scale, seed);
            printf("地形生成完成！\n");
        }
    }
    
    printf("战场已初始化，开始部署指挥部...\n");
    waitForKeyPress();
    
    // 部署红蓝双方指挥部
    while (!deployHeadquarters(&battlefield, TEAM_RED)) {
        printf("请重新部署红方指挥部\n");
    }
    
    while (!deployHeadquarters(&battlefield, TEAM_BLUE)) {
        printf("请重新部署蓝方指挥部\n");
    }
    
    printf("指挥部部署完成，接下来部署作战单位...\n");
    waitForKeyPress();
    
    // 部署红蓝双方装备
    deployEquipment(&battlefield, TEAM_RED);
    deployEquipment(&battlefield, TEAM_BLUE);
    
    printf("\n双方部署完成，按任意键开始战斗模拟...\n");
    getch();
    
    // 开始战斗模拟
    while (1) {
        clearScreen();
        renderBattlefield(&battlefield, TEAM_NONE); // 战斗阶段显示所有装备
        
        if (simulateStep(&battlefield)) {
            break; // 一方获胜，模拟结束
        }
        
        // 等待一段时间以便观察
        Sleep(500); // Windows系统中的Sleep函数，单位为毫秒
    }
    
    // 最后显示一次战场状态
    clearScreen();
    renderBattlefield(&battlefield, TEAM_NONE);
    
    printf("\n模拟结束！按任意键返回主菜单...\n");
    getch();
    
    // 释放资源
    freeBattlefield(&battlefield);
}

// 显示地形生成菜单
void showTerrainGenerationMenu(Battlefield* battlefield) {
    int consoleWidth = getConsoleWidth();
    int tableWidth = consoleWidth > 80 ? 80 : consoleWidth - 4;
    int choice = 0;
    float terrainScale = 10.0f; // 默认地形比例尺
    int seed = (int)time(NULL); // 默认随机种子
    NoiseAlgorithmType algorithmType = NOISE_FRACTAL_BROWNIAN; // 默认使用分形布朗运动
    
    while (1) {
        clearScreen();
        displayWarTitle();
        
        printf("\n");
        printCentered("地形生成系统", tableWidth);
        printf("\n");
        
        drawTableBorder('+', '+', '+', '-', tableWidth);
        drawTableRow("通过噪声算法生成逼真的战场地形", tableWidth);
        drawTableBorder('+', '+', '+', '-', tableWidth);
        
        char scaleInfo[100];
        sprintf(scaleInfo, "当前地形比例尺: %.1f (较大的值会生成更细致的地形)", terrainScale);
        drawTableRow(scaleInfo, tableWidth);
        
        char seedInfo[100];
        sprintf(seedInfo, "当前随机种子: %d", seed);
        drawTableRow(seedInfo, tableWidth);
        
        char algoInfo[100];
        sprintf(algoInfo, "当前地形算法: %s", getNoiseAlgorithmName(algorithmType));
        drawTableRow(algoInfo, tableWidth);
        
        drawTableBorder('+', '+', '+', '-', tableWidth);
        
        printf("\n");
        printf("1. 调整地形比例尺\n");
        printf("2. 设置随机种子\n");
        printf("3. 选择地形生成算法\n");
        printf("4. 生成地形\n");
        printf("5. 预览地形\n");
        printf("0. 返回上级菜单\n");
        printf("\n");
        printf("请选择一个选项 (0-5): ");
        
        scanf("%d", &choice);
        
        switch (choice) {
            case 0:
                return;
            
            case 1: {
                printf("请输入地形比例尺 (1.0-50.0, 较大的值会生成更细致的地形): ");
                scanf("%f", &terrainScale);
                if (terrainScale < 1.0f) terrainScale = 1.0f;
                if (terrainScale > 50.0f) terrainScale = 50.0f;
                break;
            }
            
            case 2: {
                printf("请输入随机种子 (0为随机生成): ");
                scanf("%d", &seed);
                if (seed == 0) {
                    seed = (int)time(NULL);
                    printf("已生成随机种子: %d\n", seed);
                    waitForKeyPress();
                }
                break;
            }
            
            case 3: {
                showNoiseAlgorithmSelectionMenu(&algorithmType);
                break;
            }
            
            case 4: {
                // 使用用户选择的算法生成地形
                generateBattlefieldTerrainWithAlgorithm(battlefield, terrainScale, seed, algorithmType);
                printf("地形生成完成！\n");
                waitForKeyPress();
                break;
            }
            
            case 5: {
                clearScreen();
                if (!battlefield->useTerrainSystem) {
                    printf("请先生成地形！\n");
                    waitForKeyPress();
                    break;
                }
                
                printf("地形预览 (高度范围: -3 到 5, 负数为水域)\n");
                printf("颜色示意: ");
                
                HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                WORD defaultColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
                
                for (int i = TERRAIN_WATER; i <= TERRAIN_MOUNTAIN_2; i++) {
                    TerrainType type = (TerrainType)i;
                    SetConsoleTextAttribute(hConsole, terrain_getTerrainColor(type));
                    printf("%s", getTerrainChar(type));
                    SetConsoleTextAttribute(hConsole, defaultColor);
                    printf(" ");
                }
                printf("\n\n");
                
                // 显示地形图
                renderBattlefield(battlefield, TEAM_NONE);
                
                printf("\n按任意键返回...\n");
                waitForKeyPress();
                break;
            }
            
            default:
                printf("无效选项，请重试\n");
                waitForKeyPress();
                break;
        }
    }
}

// 显示地形图例
void displayTerrainLegend() {
    printf("地形图例: ");
    for (int i = 0; i < TERRAIN_TYPE_COUNT; i++) {
        TerrainType type = (TerrainType)i;
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, terrain_getTerrainColor(type));
        printf("%s", getTerrainChar(type));
        SetConsoleTextAttribute(hConsole, 7); // 重置为默认颜色
        printf("=%s ", getTerrainDescription(type));
    }
    printf("\n");
}

// 获取噪声算法名称
const char* getNoiseAlgorithmName(NoiseAlgorithmType type) {
    switch (type) {
        case NOISE_PERLIN:
            return "柏林噪声 (平滑变化)";
        case NOISE_SIMPLEX:
            return "单纯形噪声 (高性能)";
        case NOISE_FRACTAL_BROWNIAN:
            return "分形布朗运动 (自然地形)";
        case NOISE_DOMAIN_WARPING:
            return "域变形噪声 (扭曲地形)";
        case NOISE_RIDGED_MULTI:
            return "山脊多重分形 (山脉地形)";
        default:
            return "未知算法";
    }
}

// 显示噪声算法选择菜单
void showNoiseAlgorithmSelectionMenu(NoiseAlgorithmType* algorithmType) {
    int consoleWidth = getConsoleWidth();
    int tableWidth = consoleWidth > 80 ? 80 : consoleWidth - 4;
    int choice = 0;
    
    clearScreen();
    displayWarTitle();
    
    printf("\n");
    printCentered("地形生成算法选择", tableWidth);
    printf("\n");
    
    drawTableBorder('+', '+', '+', '-', tableWidth);
    drawTableRow("不同的噪声算法会生成不同风格的地形", tableWidth);
    drawTableBorder('+', '+', '+', '-', tableWidth);
    
    // 当前选择的算法
    char currentAlgo[100];
    sprintf(currentAlgo, "当前选择: %s", getNoiseAlgorithmName(*algorithmType));
    drawTableRow(currentAlgo, tableWidth);
    drawTableBorder('+', '+', '+', '-', tableWidth);
    
    printf("\n");
    printf("可用算法:\n");
    printf("1. 柏林噪声 - 经典地形算法，产生平滑的地形变化\n");
    printf("2. 单纯形噪声 - 类似柏林噪声，但性能更高\n");
    printf("3. 分形布朗运动 - 多层叠加噪声，产生自然逼真的地形\n");
    printf("4. 域变形噪声 - 扭曲的地形，产生更加复杂的地形特征\n");
    printf("5. 山脊多重分形 - 特别适合生成山脉地形\n");
    printf("0. 返回上级菜单\n");
    printf("\n");
    printf("请选择一个算法 (0-5): ");
    
    scanf("%d", &choice);
    
    switch (choice) {
        case 0:
            return;
        case 1:
            *algorithmType = NOISE_PERLIN;
            break;
        case 2:
            *algorithmType = NOISE_SIMPLEX;
            break;
        case 3:
            *algorithmType = NOISE_FRACTAL_BROWNIAN;
            break;
        case 4:
            *algorithmType = NOISE_DOMAIN_WARPING;
            break;
        case 5:
            *algorithmType = NOISE_RIDGED_MULTI;
            break;
        default:
            printf("无效选项，默认使用分形布朗运动\n");
            *algorithmType = NOISE_FRACTAL_BROWNIAN;
            waitForKeyPress();
            break;
    }
    
    printf("已选择: %s\n", getNoiseAlgorithmName(*algorithmType));
    waitForKeyPress();
} 