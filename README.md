# 战场模拟器

这是一个基于C语言的战场模拟器，用于模拟红蓝双方在战场上的对抗。

## 功能介绍

- 支持多种武器装备类型，包括坦克、飞机、自行火炮、导弹发射塔等
- 每种装备具有独特的属性和交互特性
- 红蓝双方在各自半场部署装备
- 自动模拟战斗过程，直到一方全军覆没

## 如何编译

使用GCC编译器：

```bash
gcc -Wall -Wextra -o battlefield_simulator main.c battlefield.c equipment.c simulation.c menu.c -lm
```

## 如何运行

编译完成后，直接运行可执行文件：

```bash
battlefield_simulator
```

## 游戏规则

1. 程序启动后，会首先让红方部署装备，然后让蓝方部署装备
2. 每方有固定的预算，不能超出预算
3. 装备只能部署在己方半场
4. 部署完成后，战斗自动开始
5. 当一方全部装备被摧毁时，判定另一方胜利

## 装备说明

装备类型定义在`equipment_types.txt`文件中，包含以下信息：
- 装备类型ID
- 装备名称
- 装备造价
- 最高生命值
- 最高行进速度
- 最大打击半径
- 最大装弹量
- 最高射速
- 是否可以飞行

装备之间的交互特性定义在`equipment_interactions.txt`文件中，包含：
- 攻击方装备类型ID
- 防守方装备类型ID
- 单发伤害值
- 射击精确度

## 战场显示说明

- 红方装备使用大写字母表示：T(坦克)、A(飞机)、C(火炮)等
- 蓝方装备使用小写字母表示：t(坦克)、a(飞机)、c(火炮)等
- 栅栏使用#表示
- 被摧毁的装备使用x表示

## 代码文件说明

- `main.c`: 程序入口点
- `battlefield.h/c`: 战场相关定义和实现
- `equipment.h/c`: 装备相关定义和实现
- `simulation.h/c`: 模拟逻辑相关定义和实现
- `equipment_types.txt`: 装备类型数据
- `equipment_interactions.txt`: 装备交互数据 


 by Gskyer
