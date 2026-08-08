/*
 * =============================================================================
 * 设计模式：适配器模式（Adapter Pattern）—— 对象适配器
 * =============================================================================
 *
 * 【一句话概括】
 * 通过组合（对象适配器）将多个不兼容的遗留类统一到同一个接口下，实现多态调用。
 *
 * 【适用场景 —— 通用】
 * - 需要将多个接口各异的遗留类统一处理时
 * - 遗留类之间没有公共基类，但需要以统一方式调用它们
 *
 * 【金融工程应用】
 * - 多数据源统一接口：不同行情供应商（万得/同花顺/交易所）的 API 完全异构，
 *   通过对象适配器统一包装为 IMarketDataProvider，上层策略代码以统一方式获取行情
 * - 异构风控系统整合：不同业务线的风控模块（期货/股票/期权）各自独立开发，
 *   通过适配器统一为 IRiskChecker 接口，风控中台统一调用
 *
 * 【本示例说明】
 * 使用模板类 AdapterMapper<T> 作为对象适配器，将 LegacyA、LegacyB、LegacyC 三个
 * 完全不兼容的类分别包装成统一的 AdapterInterface 接口。
 */

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

using namespace std;

// ============================================================================
// 遗留代码（Legacy Code）
// 三个完全不兼容的类，没有任何公共基类，无法直接实现多态
// ============================================================================

class LegacyA {
public:
  // 没有多态的希望（析构函数不是 virtual）
  ~LegacyA() { cout << "LegacyA::destructor" << endl; }
  void doThis() { cout << "LegacyA::doThis()" << endl; }
};

class LegacyB {
public:
  ~LegacyB() { cout << "LegacyB::destructor" << endl; }
  void doThat() { cout << "LegacyB::doThat()" << endl; }
};

class LegacyC {
public:
  ~LegacyC() { cout << "LegacyC::destructor" << endl; }
  void doTheOther() { cout << "LegacyC::doTheOther()" << endl; }
};

// ============================================================================
// 适配器接口（Adapter Interface）
// 定义客户端期望的统一接口
// ============================================================================
class AdapterInterface {
public:
  virtual ~AdapterInterface() {}
  virtual void execute() = 0;
};

// 适配器智能指针类型别名，方便使用
using Adapter = shared_ptr<AdapterInterface>;

// ============================================================================
// 适配器包装器（Adapter Mapper）
// 模板类，通过组合方式包装任意遗留类 T，将其成员函数映射到统一的 execute() 接口
// ============================================================================
template <class T> class AdapterMapper : public AdapterInterface {
public:
  // 构造函数：接收一个遗留类对象和一个成员函数指针
  // 用初始化列表初始化成员（Effective C++ 条款 12）
  AdapterMapper(shared_ptr<T> object, void (T::*m)())
      : object(std::move(object)), method(m) {}
  ~AdapterMapper() = default;  // 空析构用 = default

  // 实现统一接口：调用被包装对象的指定成员函数
  void execute() override { (*object.*method)(); }

private:
  shared_ptr<T> object; // 被包装的遗留类对象
  void (T::*method)();  // 指向遗留类特定函数的成员函数指针
};

// 辅助函数：创建并初始化所有适配器
using Adapters = vector<shared_ptr<AdapterInterface>>;
Adapters initialize() {
  Adapters adapters;
  // 将 LegacyA::doThis 适配到 execute()
  adapters.emplace_back(make_shared<AdapterMapper<LegacyA>>(
      make_shared<LegacyA>(), &LegacyA::doThis));
  // 将 LegacyB::doThat 适配到 execute()
  adapters.emplace_back(make_shared<AdapterMapper<LegacyB>>(
      make_shared<LegacyB>(), &LegacyB::doThat));
  // 将 LegacyC::doTheOther 适配到 execute()
  adapters.emplace_back(make_shared<AdapterMapper<LegacyC>>(
      make_shared<LegacyC>(), &LegacyC::doTheOther));
  return adapters;
}

int main() {
  auto adapters = initialize();
  // 以统一的方式遍历并调用所有遗留类的方法
  for (auto it = adapters.begin(); it != adapters.end(); ++it) {
    (*it)->execute();
  }

  return 0;
}
