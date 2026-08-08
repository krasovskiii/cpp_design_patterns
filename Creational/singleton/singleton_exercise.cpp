/*
 * ===========================================================================
 * 设计模式：Singleton Exercise（单例模式 —— 测试练习）
 * ===========================================================================
 *
 * 【核心思想】
 * 演示如何通过代码检测一个类是否为单例。通过模板函数 is_singleton()，
 * 我们可以验证某个工厂函数是否每次返回同一个实例。
 *
 * 【适用场景】
 * - 需要验证单例实现的正确性
 * - 在单元测试中检测单例行为
 *
 * 【UML 关键参与者】
 * - Singleton（单例）：RealSingleton —— 真正的单例实现
 * - Non-Singleton（非单例）：NoSingleton —— 普通类，每次创建新实例
 * - Tester（测试者）：SingletonTester —— 提供 is_singleton 检测方法
 *
 * 【本例要点】
 * - SingletonTester::is_singleton() 通过比较两次调用工厂返回的指针是否相同来判断
 * - RealSingleton 使用标准 Meyers' Singleton 实现
 * - NoSingleton 是普通类，每次构造都创建新实例
 * - 模板函数 + std::function 实现了通用的单例检测机制
 */

#include <functional>
#include <iostream>
#include <string>

using namespace std;

/*
 * RealSingleton: 真正的单例类
 *
 * 标准 Meyers' Singleton 实现：
 * - 私有构造函数
 * - 删除拷贝和赋值
 * - 静态 get() 返回唯一实例的引用
 */
class RealSingleton {
private:
  // 私有构造函数 —— 防止外部直接创建
  RealSingleton() { cout << "Creating Real Singleton." << endl; }

public:
  // 禁止拷贝和赋值
  RealSingleton(const RealSingleton &) = delete;
  void operator=(const RealSingleton &) = delete;

  // 获取唯一实例（Meyers' Singleton）
  static RealSingleton &get() {
    static RealSingleton s;
    return s;
  }
};

/*
 * NoSingleton: 普通类（非单例）
 *
 * 每次调用构造函数都会创建一个全新的实例。
 * 用于对比演示 is_singleton() 的检测能力。
 */
class NoSingleton {
public:
  NoSingleton() { cout << "Creating normal object instance." << endl; }
};

/*
 * SingletonTester: 单例检测器
 *
 * 通过模板函数 is_singleton() 检测一个工厂函数是否返回单例。
 *
 * 检测原理：
 * 1. 调用 factory() 两次，获取两个指针 _1 和 _2
 * 2. 比较两个指针是否相等
 * 3. 如果相等 → 单例（每次返回同一个对象）
 * 4. 如果不等 → 非单例（每次创建新对象）
 */
struct SingletonTester {
  template <typename T> bool is_singleton(function<T *()> factory) {
    T *_1 = factory();
    T *_2 = factory();
    if (_1 != _2) {
      cout << "Is not singleton." << endl;
      return false;
    }
    cout << "Is singleton." << endl;
    return true;
  }
};

int main() {
  // 为 RealSingleton 创建工厂 lambda —— 返回 get() 的地址
  auto singleton_factory = [&]() -> RealSingleton * {
    return &RealSingleton::get();
  };

  // 为 NoSingleton 创建工厂 lambda —— 每次 new 一个新对象
  auto object_factory = [&]() -> NoSingleton * { return new NoSingleton(); };

  SingletonTester tester;
  // 检测 RealSingleton —— 应该输出 "Is singleton."
  tester.is_singleton<RealSingleton>(singleton_factory);
  // 检测 NoSingleton —— 应该输出 "Is not singleton."
  tester.is_singleton<NoSingleton>(object_factory);

  return 0;
}
