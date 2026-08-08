/*
 * ===========================================================================
 * 设计模式：Singleton in DI Container（依赖注入容器中的单例）
 * ===========================================================================
 *
 * 【核心思想】
 * 使用依赖注入（DI）容器来管理对象的生命周期，让 DI 容器来保证单例行为，
 * 而不是在类自身中实现单例逻辑。这种方式将"单例"从类的职责中剥离，
 * 使类本身保持简单、可测试。
 *
 * 【适用场景 —— 通用】
 * - 希望将单例的"唯一性"管理与业务逻辑分离
 * - 需要灵活的依赖注入和生命周期管理
 * - 在测试中需要替换单例为模拟对象
 *
 * 【金融工程应用】
 * - 策略依赖管理：DI 容器管理策略生命周期，回测场景中每个策略需要独立实例，
 *   实盘场景中某些组件（行情总线、风控引擎）需要单例，DI 容器统一管理
 * - 回测/实盘环境切换：DI 容器根据配置注入不同实现，回测环境注入模拟行情/模拟撮合，
 *   实盘环境注入真实行情/CTP 接口，策略代码完全不需要知道当前环境
 * - 分布式组件管理：DI 容器管理各模块的生命周期（单例/瞬态/作用域），
 *   行情模块单例、策略模块多实例、风控模块单例，DI 容器统一编排
 *
 * 【传统单例 vs DI 单例】
 * - 传统单例：类自身负责保证唯一性（私有构造 + static get()）
 * - DI 单例：类本身是普通的，DI 容器负责保证只创建一个实例
 *   - 优点：类更简单、更易测试、更灵活
 *   - 缺点：依赖 DI 框架
 *
 * 【UML 关键参与者】
 * - Interface（接口）：IFoo —— 定义服务接口
 * - Implementation（实现）：Foo —— 普通的服务实现（不需要单例逻辑）
 * - Client（客户端）：Bar —— 通过 DI 接收 IFoo 实例
 * - DI Container（DI 容器）：管理生命周期，保证 Foo 的单例行为
 *
 * 【本例要点】
 * - Foo 是一个普通类，不需要私有构造、static get() 等单例代码
 * - DI 容器通过 .in(di::singleton) 配置保证 Foo 只创建一次
 * - Bar 通过 shared_ptr<IFoo> 接收注入，两个 Bar 实例共享同一个 Foo
 */

#include <iostream>
#include <memory>
#include <string>

#include "di.hpp"

using namespace std;
namespace di = boost::ext::di::v1_2_0;

/*
 * IFoo: 单例的服务接口
 *
 * 定义服务的抽象接口，客户端通过接口依赖，而非具体实现。
 */
struct IFoo {
  virtual string name() = 0;
};

/*
 * Foo: 实际的单例实现
 *
 * 注意：Foo 不需要显式定义单例逻辑！
 * 不需要隐藏构造函数，不需要提供 get() 方法。
 * DI 容器会负责确保 Foo 只被创建一次。
 *
 * static int id 用于验证 Foo 确实只被创建了一次：
 * 如果 Foo 被多次构造，id 会递增。
 */
struct Foo : IFoo {
  // 使用静态 id 来验证对象只被创建一次
  static int id;
  Foo() { ++id; }

  string name() override { return "foo "s + to_string(id); }
};
int Foo::id = 0;

/*
 * Bar: 客户端类
 *
 * Bar 将自动从 DI 容器获得 Foo 作为单例注入。
 * shared_ptr<IFoo> 确保 Foo 的生命周期被正确管理。
 */
struct Bar {
  std::shared_ptr<IFoo> foo;
};

int main() {
  /*
   * 使用 DI 容器管理 Foo 作为单例的生命周期
   *
   * di::bind<IFoo>().to<Foo>().in(di::singleton) 的含义：
   * - 当需要 IFoo 时，提供 Foo 的实例
   * - .in(di::singleton) 确保 Foo 作为单例管理（只创建一次）
   */
  auto injector =
      di::make_injector(di::bind<IFoo>().to<Foo>().in(di::singleton));

  // 创建两个 Bar，DI 容器将同一个 Foo 单例注入到两者中
  auto bar1 = injector.create<std::shared_ptr<Bar>>();
  auto bar2 = injector.create<std::shared_ptr<Bar>>();

  cout << bar1->foo->name() << endl;
  cout << bar2->foo->name() << endl;

  // 验证：两个 Bar 对象引用的是同一个 Foo 实例
  cout << boolalpha << (bar1->foo.get() == bar2->foo.get()) << endl;

  return 0;
}
