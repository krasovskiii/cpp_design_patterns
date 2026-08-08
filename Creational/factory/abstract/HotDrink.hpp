/*
 * ===========================================================================
 * 设计模式：Abstract Factory（抽象工厂模式）
 * 文件：HotDrink.hpp —— 抽象产品及具体产品
 * ===========================================================================
 *
 * 【核心思想】
 * 抽象工厂模式提供一个接口，用于创建一系列相关或相互依赖的对象，
 * 而无需指定它们的具体类。与工厂方法模式不同，抽象工厂创建的是
 * "产品族"（family of products），而不是单个产品。
 *
 * 【适用场景】
 * - 系统需要独立于产品的创建、组合和表示
 * - 系统需要配置多个产品族中的一个（如不同主题的 UI 组件）
 * - 一组相关的产品对象需要一起使用
 *
 * 【UML 关键参与者】
 * - AbstractProduct（抽象产品）：HotDrink —— 定义产品的接口
 * - ConcreteProduct（具体产品）：Tea, Coffee —— 实现具体产品
 * - AbstractFactory（抽象工厂）：HotDrinkFactory —— 声明创建产品的接口
 * - ConcreteFactory（具体工厂）：TeaFactory, CoffeeFactory
 * - Client（客户端）：DrinkFactory —— 使用抽象工厂创建产品
 *
 * 【本例要点】
 * - HotDrink 是抽象产品基类，定义了 prepare(volume) 接口
 * - Tea 和 Coffee 是具体产品，实现各自的 prepare 逻辑
 */

#pragma once
#include <iostream>
#include <memory>
using namespace std;

/*
 * HotDrink: 抽象产品（Abstract Product）
 *
 * 定义所有热饮的公共接口。
 * 虚析构函数确保通过基类指针删除派生类对象时行为正确。
 * prepare() 是纯虚函数，每个具体热饮类必须实现自己的制备逻辑。
 */
struct HotDrink {
  virtual ~HotDrink() = default;

  // 制备热饮，volume 参数指定容量（毫升）
  virtual void prepare(int volume) = 0;
};

/*
 * Tea: 具体产品 —— 茶
 *
 * 实现茶的制备过程：取茶包、烧水、倒入指定容量、加柠檬
 */
struct Tea : HotDrink {
  void prepare(int volume) override {
    cout << "Take tea bag, boil water, pour " << volume << "ml, add some lemon"
         << endl;
  }
};

/*
 * Coffee: 具体产品 —— 咖啡
 *
 * 实现咖啡的制备过程：研磨咖啡豆、烧水、倒入指定容量、加奶、享用
 */
struct Coffee : HotDrink {
  void prepare(int volume) override {
    cout << "Grind some beans, boil water, pour " << volume
         << "ml, add cream, enjoy!" << endl;
  }
};
