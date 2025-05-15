#include "terrain_renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>

// 静态变量保存当前的颜色配置
static TerrainColorConfig currentColorConfig;

// 颜色常量定义
#define COLOR_BLUE        1
#define COLOR_GREEN       2
#define COLOR_CYAN        3
#define COLOR_RED         4
#define COLOR_MAGENTA     5
#define COLOR_YELLOW      6
#define COLOR_WHITE       7
#define COLOR_BRIGHT_BLUE 9
#define COLOR_BRIGHT_GREEN 10
#define COLOR_BRIGHT_CYAN 11
#define COLOR_BRIGHT_RED  12
#define COLOR_BRIGHT_MAGENTA 13
#define COLOR_BRIGHT_YELLOW 14
#define COLOR_BRIGHT_WHITE 15

// 初始化地形渲染器
void initTerrainRenderer() {
    // 设置默认颜色配置
    currentColorConfig = getDefaultColorConfig();
    
    // 设置控制台输出支持ANSI颜色代码
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

// 获取默认地形颜色配置
TerrainColorConfig getDefaultColorConfig() {
    TerrainColorConfig config;
    
    // 不同深度水域的颜色（从深到浅）
    config.waterColors[0] = COLOR_BLUE;             // 深水
    config.waterColors[1] = COLOR_BRIGHT_BLUE;      // 中深水
    config.waterColors[2] = COLOR_CYAN;             // 中浅水
    config.waterColors[3] = COLOR_BRIGHT_CYAN;      // 浅水
    
    // 平地颜色
    config.flatColor = COLOR_GREEN;
    
    // 丘陵颜色（从低到高）
    config.hillColors[0] = COLOR_BRIGHT_GREEN;      // 低丘陵
    config.hillColors[1] = COLOR_YELLOW;            // 中丘陵
    config.hillColors[2] = COLOR_BRIGHT_YELLOW;     // 高丘陵
    
    // 山地颜色（从低到高）
    config.mountainColors[0] = COLOR_RED;           // 低山
    config.mountainColors[1] = COLOR_BRIGHT_RED;    // 高山
    
    // 特殊地形颜色
    config.forestColor = COLOR_GREEN;
    config.riverColor = COLOR_BRIGHT_BLUE;
    config.roadColor = COLOR_WHITE;
    config.cliffColor = COLOR_MAGENTA;
    
    return config;
}

// 设置自定义地形颜色配置
void setTerrainColorConfig(TerrainColorConfig config) {
    currentColorConfig = config;
}

// 获取地形对应的显示颜色
int getTerrainColor(TerrainType type) {
    switch (type) {
        case TERRAIN_WATER:
            if (type <= TERRAIN_MIN_HEIGHT + 0.75f)
                return currentColorConfig.waterColors[0]; // 深水
            else if (type <= TERRAIN_MIN_HEIGHT + 1.5f)
                return currentColorConfig.waterColors[1]; // 中深水
            else if (type <= TERRAIN_MIN_HEIGHT + 2.25f)
                return currentColorConfig.waterColors[2]; // 中浅水
            else
                return currentColorConfig.waterColors[3]; // 浅水
            
        case TERRAIN_FLAT:
            return currentColorConfig.flatColor;
            
        case TERRAIN_HILL_1:
            return currentColorConfig.hillColors[0];
        case TERRAIN_HILL_2:
            return currentColorConfig.hillColors[1];
        case TERRAIN_HILL_3:
            return currentColorConfig.hillColors[2];
            
        case TERRAIN_MOUNTAIN_1:
            return currentColorConfig.mountainColors[0];
        case TERRAIN_MOUNTAIN_2:
            return currentColorConfig.mountainColors[1];
            
        default:
            return COLOR_WHITE;
    }
}

// 根据渲染模式获取单元格颜色
int getTerrainCellColor(Terrain* terrain, TerrainRenderMode renderMode) {
    switch (renderMode) {
        case RENDER_MODE_NORMAL:
            // 首先检查是否有特殊地形特性
            if (terrain->flags & TERRAIN_FLAG_RIVER)
                return currentColorConfig.riverColor;
            else if (terrain->flags & TERRAIN_FLAG_FOREST)
                return currentColorConfig.forestColor;
            else if (terrain->flags & TERRAIN_FLAG_CLIFF)
                return currentColorConfig.cliffColor;
            else if (terrain->flags & TERRAIN_FLAG_ROAD)
                return currentColorConfig.roadColor;
            else
                return getTerrainColor(terrain->type);
            
        case RENDER_MODE_HEIGHT_MAP: {
            // 高度图模式，使用渐变色表示高度
            float normalizedHeight = (terrain->height - TERRAIN_MIN_HEIGHT) / 
                                    (TERRAIN_MAX_HEIGHT - TERRAIN_MIN_HEIGHT);
            
            // 简单的颜色渐变：蓝->绿->黄->红
            if (normalizedHeight < 0.25f)
                return COLOR_BLUE;
            else if (normalizedHeight < 0.5f)
                return COLOR_GREEN;
            else if (normalizedHeight < 0.75f)
                return COLOR_YELLOW;
            else
                return COLOR_RED;
        }
            
        case RENDER_MODE_TOPO_MAP: {
            // 等高线图模式
            float height = terrain->height;
            float contourInterval = 1.0f; // 等高线间隔
            
            // 如果高度接近等高线值，显示为白色
            if (fabs(height - round(height / contourInterval) * contourInterval) < 0.1f)
                return COLOR_WHITE;
            else {
                // 否则按高度显示渐变颜色
                return getTerrainCellColor(terrain, RENDER_MODE_HEIGHT_MAP);
            }
        }
            
        case RENDER_MODE_MOISTURE_MAP: {
            // 湿度图模式，从白色(干燥)到蓝色(湿润)
            float moisture = terrain->moisture;
            
            if (moisture < 0.25f)
                return COLOR_WHITE;
            else if (moisture < 0.5f)
                return COLOR_CYAN;
            else if (moisture < 0.75f)
                return COLOR_BRIGHT_BLUE;
            else
                return COLOR_BLUE;
        }
            
        case RENDER_MODE_TEMPERATURE_MAP: {
            // 温度图模式，从蓝色(寒冷)到红色(炎热)
            float temperature = terrain->temperature;
            
            if (temperature < 0.25f)
                return COLOR_BLUE;
            else if (temperature < 0.5f)
                return COLOR_GREEN;
            else if (temperature < 0.75f)
                return COLOR_YELLOW;
            else
                return COLOR_RED;
        }
            
        default:
            return getTerrainColor(terrain->type);
    }
}

// 根据渲染模式获取单元格符号
char* getTerrainCellSymbol(Terrain* terrain, TerrainRenderMode renderMode) {
    switch (renderMode) {
        case RENDER_MODE_NORMAL:
            // 检查是否有特殊地形特性
            if (terrain->flags & TERRAIN_FLAG_RIVER)
                return "≈"; // 波浪线表示河流
            else if (terrain->flags & TERRAIN_FLAG_FOREST)
                return "♣"; // 树表示森林
            else if (terrain->flags & TERRAIN_FLAG_CLIFF)
                return "▲"; // 三角形表示悬崖
            else if (terrain->flags & TERRAIN_FLAG_ROAD)
                return "="; // 等号表示道路
            else
                return "█"; // 默认方块
            
        case RENDER_MODE_TOPO_MAP:
            // 等高线图模式
            float height = terrain->height;
            float contourInterval = 1.0f;
            
            // 在等高线上显示高度数值
            if (fabs(height - round(height / contourInterval) * contourInterval) < 0.1f) {
                static char heightStr[4];
                sprintf(heightStr, "%d", (int)height);
                return heightStr;
            } else {
                return "·"; // 点表示非等高线区域
            }
            
        default:
            return "█"; // 默认使用方块字符
    }
}

// 渲染单个地形单元格
void renderTerrainCell(Terrain* terrain, TerrainDisplayConfig displayConfig, TerrainRenderMode renderMode) {
    // 获取显示颜色和符号
    int color = getTerrainCellColor(terrain, renderMode);
    char* symbol = getTerrainCellSymbol(terrain, renderMode);
    
    // 设置颜色
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
    
    // 根据符号大小打印
    if (displayConfig.symbolSize == 1) {
        printf("%s", symbol);
    } else {
        printf("%s%s", symbol, symbol); // 打印两次符号
    }
    
    // 重置颜色
    SetConsoleTextAttribute(hConsole, COLOR_WHITE);
}

// 渲染等高线图
void renderTopographicMap(int width, int height, Terrain** terrainMap, float contourInterval) {
    TerrainDisplayConfig displayConfig = getDefaultDisplayConfig();
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Terrain* terrain = &terrainMap[y][x];
            float terrainHeight = terrain->height;
            
            // 检查是否在等高线上
            if (fabs(terrainHeight - round(terrainHeight / contourInterval) * contourInterval) < 0.1f) {
                // 在等高线上，显示为白色
                HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                SetConsoleTextAttribute(hConsole, COLOR_WHITE);
                printf("█");
                SetConsoleTextAttribute(hConsole, COLOR_WHITE);
            } else {
                // 非等高线，使用灰度显示高度
                int grayLevel = (int)(((terrainHeight - TERRAIN_MIN_HEIGHT) / 
                                      (TERRAIN_MAX_HEIGHT - TERRAIN_MIN_HEIGHT)) * 15);
                HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                SetConsoleTextAttribute(hConsole, grayLevel);
                printf("·");
                SetConsoleTextAttribute(hConsole, COLOR_WHITE);
            }
        }
        printf("\n");
    }
}

// 渲染热力图
void renderHeatMap(int width, int height, Terrain** terrainMap, TerrainRenderMode mode) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Terrain* terrain = &terrainMap[y][x];
            int color;
            
            // 根据模式选择颜色映射
            switch (mode) {
                case RENDER_MODE_MOISTURE_MAP:
                    // 湿度热力图
                    {
                        float moisture = terrain->moisture;
                        if (moisture < 0.2f) color = COLOR_WHITE;
                        else if (moisture < 0.4f) color = COLOR_CYAN;
                        else if (moisture < 0.6f) color = COLOR_BRIGHT_BLUE;
                        else if (moisture < 0.8f) color = COLOR_BLUE;
                        else color = COLOR_BRIGHT_MAGENTA;
                    }
                    break;
                    
                case RENDER_MODE_TEMPERATURE_MAP:
                    // 温度热力图
                    {
                        float temperature = terrain->temperature;
                        if (temperature < 0.2f) color = COLOR_BLUE;
                        else if (temperature < 0.4f) color = COLOR_CYAN;
                        else if (temperature < 0.6f) color = COLOR_GREEN;
                        else if (temperature < 0.8f) color = COLOR_YELLOW;
                        else color = COLOR_RED;
                    }
                    break;
                    
                default:
                    // 高度热力图
                    {
                        float normalizedHeight = (terrain->height - TERRAIN_MIN_HEIGHT) / 
                                               (TERRAIN_MAX_HEIGHT - TERRAIN_MIN_HEIGHT);
                        if (normalizedHeight < 0.2f) color = COLOR_BLUE;
                        else if (normalizedHeight < 0.4f) color = COLOR_GREEN;
                        else if (normalizedHeight < 0.6f) color = COLOR_YELLOW;
                        else if (normalizedHeight < 0.8f) color = COLOR_RED;
                        else color = COLOR_BRIGHT_RED;
                    }
                    break;
            }
            
            // 设置颜色并打印
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hConsole, color);
            printf("█");
            SetConsoleTextAttribute(hConsole, COLOR_WHITE);
        }
        printf("\n");
    }
}

// 渲染地形高度分布直方图
void renderTerrainHistogram(int width, int height, Terrain** terrainMap) {
    // 计算高度分布
    int heightBins[TERRAIN_MAX_HEIGHT - TERRAIN_MIN_HEIGHT + 1] = {0};
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float h = terrainMap[y][x].height;
            int binIndex = (int)(h - TERRAIN_MIN_HEIGHT);
            if (binIndex >= 0 && binIndex <= TERRAIN_MAX_HEIGHT - TERRAIN_MIN_HEIGHT) {
                heightBins[binIndex]++;
            }
        }
    }
    
    // 找出最大计数，用于标准化
    int maxCount = 0;
    for (int i = 0; i <= TERRAIN_MAX_HEIGHT - TERRAIN_MIN_HEIGHT; i++) {
        if (heightBins[i] > maxCount) {
            maxCount = heightBins[i];
        }
    }
    
    // 渲染直方图
    int histogramHeight = 20; // 直方图高度
    printf("\n高度分布直方图：\n");
    
    for (int row = histogramHeight - 1; row >= 0; row--) {
        for (int i = 0; i <= TERRAIN_MAX_HEIGHT - TERRAIN_MIN_HEIGHT; i++) {
            float normalizedHeight = (float)heightBins[i] / maxCount;
            int barHeight = (int)(normalizedHeight * histogramHeight);
            
            if (barHeight > row) {
                // 根据高度设置颜色
                int color;
                if (i + TERRAIN_MIN_HEIGHT < 0) color = COLOR_BLUE;
                else if (i + TERRAIN_MIN_HEIGHT == 0) color = COLOR_GREEN;
                else if (i + TERRAIN_MIN_HEIGHT <= 3) color = COLOR_YELLOW;
                else color = COLOR_RED;
                
                HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                SetConsoleTextAttribute(hConsole, color);
                printf("█");
                SetConsoleTextAttribute(hConsole, COLOR_WHITE);
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    
    // 打印坐标轴
    for (int i = 0; i <= TERRAIN_MAX_HEIGHT - TERRAIN_MIN_HEIGHT; i++) {
        printf("-");
    }
    printf("\n");
    
    for (int i = 0; i <= TERRAIN_MAX_HEIGHT - TERRAIN_MIN_HEIGHT; i += 2) {
        printf("%d", i + TERRAIN_MIN_HEIGHT);
        if (i + 1 <= TERRAIN_MAX_HEIGHT - TERRAIN_MIN_HEIGHT) {
            printf(" ");
        }
    }
    printf("\n");
}

// 在控制台渲染地形3D视图
void renderTerrain3DView(int width, int height, Terrain** terrainMap, int viewAngle, int viewHeight) {
    // 简单的3D等距投影
    float angleRad = (float)viewAngle * 3.14159f / 180.0f;
    float cosAngle = cosf(angleRad);
    float sinAngle = sinf(angleRad);
    
    // 创建屏幕缓冲区
    int screenWidth = width * 2; // 加倍宽度以获得更好的水平分辨率
    int screenHeight = height + TERRAIN_MAX_HEIGHT - TERRAIN_MIN_HEIGHT + 1;
    float** zBuffer = (float**)malloc(screenHeight * sizeof(float*));
    int** colorBuffer = (int**)malloc(screenHeight * sizeof(int*));
    
    for (int i = 0; i < screenHeight; i++) {
        zBuffer[i] = (float*)malloc(screenWidth * sizeof(float));
        colorBuffer[i] = (int*)malloc(screenWidth * sizeof(int));
        
        // 初始化z缓冲区为最大值，颜色缓冲区为黑色
        for (int j = 0; j < screenWidth; j++) {
            zBuffer[i][j] = -99999.0f; // 负无穷
            colorBuffer[i][j] = 0;     // 黑色
        }
    }
    
    // 投影每个地形点
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float terrainHeight = terrainMap[y][x].height;
            
            // 计算投影坐标
            int projX = (int)((x - y) * cosAngle * 0.5f + screenWidth / 2);
            int projY = (int)((x + y) * sinAngle * 0.25f - terrainHeight + screenHeight / 2 - viewHeight);
            
            // 计算深度（用于z缓冲）
            float depth = x + y;
            
            // 如果这个点在前面且在屏幕范围内，将其写入缓冲区
            if (projX >= 0 && projX < screenWidth && projY >= 0 && projY < screenHeight) {
                if (depth > zBuffer[projY][projX]) {
                    zBuffer[projY][projX] = depth;
                    colorBuffer[projY][projX] = getTerrainColor(terrainMap[y][x].type);
                }
            }
        }
    }
    
    // 渲染缓冲区
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    for (int y = 0; y < screenHeight; y++) {
        for (int x = 0; x < screenWidth; x++) {
            if (zBuffer[y][x] > -99999.0f) {
                SetConsoleTextAttribute(hConsole, colorBuffer[y][x]);
                printf("█");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    
    // 重置控制台颜色
    SetConsoleTextAttribute(hConsole, COLOR_WHITE);
    
    // 释放缓冲区
    for (int i = 0; i < screenHeight; i++) {
        free(zBuffer[i]);
        free(colorBuffer[i]);
    }
    free(zBuffer);
    free(colorBuffer);
} 