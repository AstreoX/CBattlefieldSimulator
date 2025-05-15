#include "noise.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

// 静态种子变量
static int noiseSeed = 0;

// 初始化噪声种子
void initNoiseSeed(int seed) {
    noiseSeed = seed;
    if (noiseSeed == 0) {
        noiseSeed = (int)time(NULL);
    }
}

// 平滑插值函数
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// 平滑曲线函数，使过渡更自然
float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10); // 6t^5 - 15t^4 + 10t^3
}

// 伪随机哈希函数
int hash(int x, int y, int seed) {
    int hash = x + y * 137 + seed * 13;
    hash = (hash << 13) ^ hash;
    return ((hash * (hash * hash * 15731 + 789221) + 1376312589) & 0x7fffffff);
}

// 梯度函数
float grad(int hash, float x, float y) {
    int h = hash & 15;
    float grad = 1.0f + (h & 7);  // 梯度值 1-8
    if ((h & 8) != 0) grad = -grad;  // 一半负值
    return (grad * x + grad * y);   // 梯度 * 距离向量
}

// 简单噪声函数
float noise2D(float x, float y, int seed) {
    int x0 = (int)floor(x);
    int y0 = (int)floor(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;
    
    float sx = x - (float)x0;
    float sy = y - (float)y0;
    
    float n0 = (float)hash(x0, y0, seed) / 0x7fffffff;
    float n1 = (float)hash(x1, y0, seed) / 0x7fffffff;
    float n2 = (float)hash(x0, y1, seed) / 0x7fffffff;
    float n3 = (float)hash(x1, y1, seed) / 0x7fffffff;
    
    // 使用平滑插值
    float ix0 = lerp(n0, n1, sx);
    float ix1 = lerp(n2, n3, sx);
    return lerp(ix0, ix1, sy) * 2 - 1; // 范围 -1 到 1
}

// 柏林噪声实现
float perlinNoise(float x, float y, int seed) {
    int x0 = (int)floor(x);
    int y0 = (int)floor(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;
    
    float sx = x - (float)x0;
    float sy = y - (float)y0;
    
    // 使用平滑曲线函数使过渡更自然
    float u = fade(sx);
    float v = fade(sy);
    
    // 计算四个顶点的哈希值
    int n00 = hash(x0, y0, seed);
    int n01 = hash(x0, y1, seed);
    int n10 = hash(x1, y0, seed);
    int n11 = hash(x1, y1, seed);
    
    // 计算四个顶点的梯度值
    float g00 = grad(n00, sx, sy);
    float g01 = grad(n01, sx, sy - 1);
    float g10 = grad(n10, sx - 1, sy);
    float g11 = grad(n11, sx - 1, sy - 1);
    
    // 插值得到最终噪声值
    float x0y0 = lerp(g00, g10, u);
    float x0y1 = lerp(g01, g11, u);
    return lerp(x0y0, x0y1, v) * 0.5f + 0.5f; // 转换到 0-1 范围
}

// 简单版单纯形噪声
float simplexNoise(float x, float y, int seed) {
    // 简化的单纯形噪声实现
    // 实际上用了改进的2D噪声算法
    const float F2 = 0.366025403f; // (sqrt(3)-1)/2
    const float G2 = 0.211324865f; // (3-sqrt(3))/6
    
    // 偏移坐标到单纯形网格
    float s = (x + y) * F2;
    float xs = x + s;
    float ys = y + s;
    int i = (int)floor(xs);
    int j = (int)floor(ys);
    
    float t = (float)(i + j) * G2;
    float X0 = i - t;
    float Y0 = j - t;
    float x0 = x - X0;
    float y0 = y - Y0;
    
    // 确定单纯形中的两个顶点
    int i1, j1;
    if (x0 > y0) { i1 = 1; j1 = 0; }
    else { i1 = 0; j1 = 1; }
    
    float x1 = x0 - i1 + G2;
    float y1 = y0 - j1 + G2;
    float x2 = x0 - 1.0f + 2.0f * G2;
    float y2 = y0 - 1.0f + 2.0f * G2;
    
    // 计算三个顶点的贡献
    float n0 = 0, n1 = 0, n2 = 0;
    
    float t0 = 0.5f - x0*x0 - y0*y0;
    if (t0 < 0) n0 = 0.0f;
    else {
        t0 *= t0;
        n0 = t0 * t0 * grad(hash(i, j, seed), x0, y0);
    }
    
    float t1 = 0.5f - x1*x1 - y1*y1;
    if (t1 < 0) n1 = 0.0f;
    else {
        t1 *= t1;
        n1 = t1 * t1 * grad(hash(i + i1, j + j1, seed), x1, y1);
    }
    
    float t2 = 0.5f - x2*x2 - y2*y2;
    if (t2 < 0) n2 = 0.0f;
    else {
        t2 *= t2;
        n2 = t2 * t2 * grad(hash(i + 1, j + 1, seed), x2, y2);
    }
    
    // 将结果缩放到[-1,1]区间
    return 70.0f * (n0 + n1 + n2);
}

// 分形布朗运动噪声，使用多个八度的噪声叠加
float fractalBrownianMotion(float x, float y, int octaves, float persistence, float lacunarity, int seed) {
    float total = 0;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0;  // 用于正规化
    
    for (int i = 0; i < octaves; i++) {
        total += perlinNoise(x * frequency, y * frequency, seed + i) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return total / maxValue;  // 正规化为 0 到 1
}

// 域扭曲噪声，更自然的噪声效果
float domainWarpingNoise(float x, float y, float warpStrength, int seed) {
    // 使用两个噪声场来扭曲输入坐标
    float warpX = perlinNoise(x + 5.2f, y + 1.3f, seed) * 2 - 1;
    float warpY = perlinNoise(x + 8.7f, y + 2.8f, seed + 1) * 2 - 1;
    
    // 扭曲输入坐标
    float warped_x = x + warpX * warpStrength;
    float warped_y = y + warpY * warpStrength;
    
    // 应用扭曲后坐标的噪声计算
    return perlinNoise(warped_x, warped_y, seed + 2);
}

// 二次元域扭曲
void domainWarp2D(float* x, float* y, float warpStrength, int seed) {
    float warpX = perlinNoise(*x + 5.2f, *y + 1.3f, seed) * 2 - 1;
    float warpY = perlinNoise(*x + 8.7f, *y + 2.8f, seed + 1) * 2 - 1;
    
    *x += warpX * warpStrength;
    *y += warpY * warpStrength;
}

// 山脊多重分形噪声，适合生成山脉
float ridgedMultiNoise(float x, float y, int octaves, float persistence, float lacunarity, int seed) {
    float total = 0;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0;
    
    for (int i = 0; i < octaves; i++) {
        // 生成噪声并转换为山脊形状（1 - abs(noise)）
        float n = perlinNoise(x * frequency, y * frequency, seed + i);
        n = 1.0f - 2.0f * fabsf(n - 0.5f);  // 转换为山脊形状
        n = n * n;  // 锐化山脊
        
        total += n * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return total / maxValue;
}

// 沃罗诺伊噪声，生成细胞状噪声（好用于生成岩石、晶体等）
float voronoiNoise(float x, float y, int seed) {
    int x0 = (int)floor(x);
    int y0 = (int)floor(y);
    
    float minDist = 1.0f;  // 初始化为最大可能值
    
    // 考察当前单元格及其8个邻居
    for (int yi = -1; yi <= 1; yi++) {
        for (int xi = -1; xi <= 1; xi++) {
            int cx = x0 + xi;
            int cy = y0 + yi;
            
            // 为每个单元格生成一个随机点
            float jitter = 0.7f;  // 随机点位置的抖动范围
            float px = cx + jitter * ((float)hash(cx, cy, seed) / 0x7fffffff);
            float py = cy + jitter * ((float)hash(cx, cy, seed + 1) / 0x7fffffff);
            
            // 计算输入位置到随机点的距离
            float dx = px - x;
            float dy = py - y;
            float dist = sqrtf(dx*dx + dy*dy);
            
            if (dist < minDist) {
                minDist = dist;
            }
        }
    }
    
    return minDist;  // 0-1范围（接近有限）
}

// 混合两种噪声
float blendNoise(float x, float y, float blendFactor, int seed1, int seed2) {
    float noise1 = perlinNoise(x, y, seed1);
    float noise2 = simplexNoise(x, y, seed2);
    
    return lerp(noise1, noise2, blendFactor);
} 