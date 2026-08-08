/*
 * ===========================================================================
 * 设计模式：Abstract Factory（抽象工厂模式）
 * 文件：HotDrinkFactory.hpp —— 抽象工厂及具体工厂
 * ===========================================================================
 *
 * 【核心思想】
 * 抽象工厂（HotDrinkFactory）定义了创建产品的接口（make），
 * 每个具体工厂（TeaFactory、CoffeeFactory）负责创建一种具体的产品。
 * 客户端通过抽象工厂接口操作，不依赖具体工厂类。
 *
 * 【设计要点】
 * - HotDrinkFactory 是抽象工厂，声明纯虚函数 make()
 * - TeaFactory 创建 Tea 对象
 * - CoffeeFactory 创建 Coffee 对象
 * - 返回类型是 unique_ptr<HotDrink>，遵循"依赖抽象而非具体"原则
 * - 如果需要添加新饮品（如 HotChocolate），只需新增具体工厂和产品类
 */

#pragma once
#include "HotDrink.hpp"

using namespace std;

/*
 * HotDrinkFactory: 抽象工厂（Abstract Factory）
 *
 * 定义创建热饮的接口。每个具体工厂实现不同的热饮创建逻辑。
 * 抽象工厂允许我们实现不同的工厂，每个工厂负责创建一种产品。
 */
struct HotDrinkFactory {
  // 纯虚函数 —— 每个具体工厂必须实现
  virtual unique_ptr<HotDrink> make() const = 0;
};

/*
 * TeaFactory: 具体工厂 —— 创建茶
 *
 * make() 返回 Tea 对象的 unique_ptr。
 * 如果需要额外的初始化逻辑（如设置默认温度、选择茶叶种类等），
 * 可以在这里添加。
 */
struct TeaFactory : HotDrinkFactory {
  unique_ptr<HotDrink> make() const override {
    // 额外的茶制备逻辑应该放在这里
    return make_unique<Tea>();
  }
};

/*
 * CoffeeFactory: 具体工厂 —— 创建咖啡
 *
 * make() 返回 Coffee 对象的 unique_ptr。
 * 可以在此添加咖啡特定的初始化逻辑。
 */
struct CoffeeFactory : HotDrinkFactory {
  unique_ptr<HotDrink> make() const override {
    // 同样的模式 —— 可以添加咖啡特定的初始化
    return make_unique<Coffee>();
  }
};
