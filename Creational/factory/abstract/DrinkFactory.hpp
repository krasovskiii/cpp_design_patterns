/*
 * ===========================================================================
 * 设计模式：Abstract Factory（抽象工厂模式）
 * 文件：DrinkFactory.hpp —— 工厂管理器及函数式工厂
 * ===========================================================================
 *
 * 【核心思想】
 * DrinkFactory 作为工厂的管理器（或"超级工厂"），根据饮品名称字符串
 * 查找对应的具体工厂来创建产品。这提供了两个关键优势：
 * 1. 客户端只需传递字符串即可获得正确的产品
 * 2. 可以在创建产品前后添加通用逻辑（如 prepare）
 *
 * DrinkWithVolumeFactory 展示了另一种函数式（functional）的工厂实现方式，
 * 使用 std::function 替代继承，更加灵活和轻量。
 *
 * 【两种实现对比】
 * - DrinkFactory：基于继承的传统抽象工厂 —— 通过 map<string, unique_ptr<HotDrinkFactory>>
 * - DrinkWithVolumeFactory：函数式工厂 —— 通过 map<string, function<unique_ptr<HotDrink>()>>
 *   函数式方式更灵活，不需要为每种产品创建单独的工厂类
 */

#pragma once

#include "HotDrink.hpp"
#include "HotDrinkFactory.hpp"
#include <functional>
#include <map>
#include <memory>

using namespace std;

/*
 * DrinkFactory: 工厂管理器（基于继承的传统方式）
 *
 * 维护一个工厂注册表（map），根据饮品名称查找对应的工厂。
 *
 * 工作流程：
 * 1. 构造时注册所有已知的饮品工厂
 * 2. make_drink(type) 根据类型字符串查找工厂
 * 3. 调用工厂的 make() 创建产品
 * 4. 调用产品的 prepare() 进行初始化
 *
 * 扩展新饮品：在构造函数中注册新的工厂即可
 */
class DrinkFactory {
  // 工厂注册表：饮品名称 -> 对应的抽象工厂
  map<string, unique_ptr<HotDrinkFactory>> hot_factories;

public:
  // 构造函数：注册所有已知的饮品工厂
  DrinkFactory() {
    hot_factories["coffee"] = make_unique<CoffeeFactory>();
    hot_factories["tea"] = make_unique<TeaFactory>();
  }

  // 根据饮品类型名称创建饮品
  unique_ptr<HotDrink> make_drink(string type) {
    // 1. 查找对应的工厂并创建产品
    auto drink = hot_factories[type]->make();
    // 2. 统一调用 prepare（所有饮品共有的初始化步骤）
    drink->prepare(200);
    return drink;
  }
};

/*
 * DrinkWithVolumeFactory: 函数式工厂实现
 *
 * 使用 std::function 替代继承体系，不依赖于抽象工厂类。
 * 这种方式的优势：
 * 1. 不需要为每种产品创建单独的工厂类
 * 2. 创建逻辑可以直接用 lambda 表达式内联定义
 * 3. 更灵活 —— 可以在 lambda 中做任意初始化
 *
 * 注意：这里只注册了 "tea"，演示了可以按需注册。
 */
class DrinkWithVolumeFactory {
  // 函数注册表：饮品名称 -> 创建函数（lambda）
  map<string, function<unique_ptr<HotDrink>()>> factories;

public:
  DrinkWithVolumeFactory() {
    // 用 lambda 定义茶的创建逻辑（包括 prepare 调用）
    factories["tea"] = [] {
      auto tea = make_unique<Tea>();
      tea->prepare(200);  // 在创建时直接指定容量
      return tea;
    };
  }

  unique_ptr<HotDrink> make_drink(string type) { return factories[type](); }
};
