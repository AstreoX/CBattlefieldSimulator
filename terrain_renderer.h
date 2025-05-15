#ifndef TERRAIN_RENDERER_H
#define TERRAIN_RENDERER_H

#include "terrain.h"

// 渲染模式枚举
typedef enum {
    RENDER_MODE_NORMAL,       // 普通模式，显示地形字符
    RENDER_MODE_HEIGHT_MAP,   // 高度图模式，使用颜色深浅表示高度
    RENDER_MODE_TOPO_MAP,     // 等高线图模式
    RENDER_MODE_MOISTURE_MAP, // 湿度图模式
    RENDER_MODE_TEMPERATURE_MAP // 温度图模式
} TerrainRenderMode;

// 地形颜色配置
typedef struct {
    int waterColors[4];      // 不同深度水域的颜色
    int flatColor;           // 平地颜色
    int hillColors[3];       // 不同高度丘陵的颜色
    int mountainColors[2];   // 不同高度山地的颜色
    int forestColor;         // 森林颜色
    int riverColor;          // 河流颜色
    int roadColor;           // 道路颜色
    int cliffColor;          // 悬崖颜色
} TerrainColorConfig;

// 初始化地形渲染器
void initTerrainRenderer();

// 渲染单个地形单元格
void renderTerrainCell(Terrain* terrain, TerrainDisplayConfig displayConfig, TerrainRenderMode renderMode);

// 获取默认地形颜色配置
TerrainColorConfig getDefaultColorConfig();

// 设置自定义地形颜色配置
void setTerrainColorConfig(TerrainColorConfig config);

// 根据渲染模式获取单元格颜色
int getTerrainCellColor(Terrain* terrain, TerrainRenderMode renderMode);

// 根据渲染模式获取单元格符号
char* getTerrainCellSymbol(Terrain* terrain, TerrainRenderMode renderMode);

// 渲染等高线图
void renderTopographicMap(int width, int height, Terrain** terrainMap, float contourInterval);

// 渲染热力图
void renderHeatMap(int width, int height, Terrain** terrainMap, TerrainRenderMode mode);

// 渲染地形高度分布直方图
void renderTerrainHistogram(int width, int height, Terrain** terrainMap);

// 在控制台渲染地形3D视图
void renderTerrain3DView(int width, int height, Terrain** terrainMap, int viewAngle, int viewHeight);

#endif // TERRAIN_RENDERER_H 