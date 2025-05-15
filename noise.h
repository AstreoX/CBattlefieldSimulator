#ifndef NOISE_H
#define NOISE_H

// 噪声算法函数

// 平滑插值函数
float lerp(float a, float b, float t);

// 伪随机哈希函数
int hash(int x, int y, int seed);

// 梯度函数
float grad(int hash, float x, float y);

// 简单噪声函数，返回-1到1之间的噪声值
float noise2D(float x, float y, int seed);

// 柏林噪声，返回-1到1之间的值
float perlinNoise(float x, float y, int seed);

// 单纯形噪声，比柏林噪声更高效
float simplexNoise(float x, float y, int seed);

// 分形布朗运动噪声，使用多个八度的噪声叠加
float fractalBrownianMotion(float x, float y, int octaves, float persistence, float lacunarity, int seed);

// 域扭曲噪声，更自然的噪声效果
float domainWarpingNoise(float x, float y, float warpStrength, int seed);

// 山脊多重分形噪声，适合生成山脉
float ridgedMultiNoise(float x, float y, int octaves, float persistence, float lacunarity, int seed);

// 沃罗诺伊噪声，生成细胞状噪声（好用于生成岩石、晶体等）
float voronoiNoise(float x, float y, int seed);

// 混合两种噪声
float blendNoise(float x, float y, float blendFactor, int seed1, int seed2);

// 二次元域扭曲
void domainWarp2D(float* x, float* y, float warpStrength, int seed);

// 初始化噪声种子
void initNoiseSeed(int seed);

#endif // NOISE_H 