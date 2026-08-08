/*
 * =============================================================================
 * 设计模式：观察者模式（Observer）—— 基础观察者示例
 * =============================================================================
 *
 * 【一句话概括】
 * Person 作为被观察者，当年龄变化时通知所有观察者。
 *
 * 【适用场景 —— 通用】
 * - 对象状态变化时需要自动通知其他对象
 * - 需要一对多的依赖关系，一个对象变化影响多个对象
 * - GUI 事件处理、数据绑定、发布-订阅系统
 *
 * 【金融工程应用】
 * - 行情推送订阅：QuoteObservable 价格变动时通知所有订阅的策略 Observer，
 *   策略收到行情后计算信号，实现行情驱动的策略执行
 * - 订单状态通知：OrderObservable 状态变更（已提交→部分成交→全部成交）时通知
 *   风控模块和日志模块，各模块独立响应
 * - 账户权益监控：AccountObservable 权益/保证金变动时通知风控和通知模块，
 *   触发追加保证金告警或强平检查
 * - 市场状态广播：MarketSessionObservable 开盘/收盘/午休状态变化时通知所有策略，
 *   策略根据市场状态切换行为
 *
 * 【关键参与者】
 *   - Subject（被观察者/主题）：Person，继承 Observable<Person>
 *   - Observer（观察者）：ConsolePersonObserver，实现 field_changed() 回调
 */

#include "observable.h"
#include "observer.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// 被观察者（Observable/Subject）：Person 在年龄变化时通知观察者
class Person : public Observable<Person> {
  int age{0};

public:
  Person() {}
  explicit Person(int age) : age(age) {}

  int get_age() const { return age; }

  // 设置年龄：如果年龄确实变化了，通知观察者
  void set_age(int age) {
    if (this->age == age)
      return; // 没有变化，无需通知

    auto old_can_vote = get_can_vote(); // (1) 保存旧的 can_vote 状态
    this->age = age;                    // 更新年龄
    notify(*this, "age");              // 通知 "age" 字段变化

    if (old_can_vote != get_can_vote()) // (2) 比较 can_vote 前后差异
      notify(*this, "can_vote");        // (3) 如有变化，通知 "can_vote" 字段变化
  }

  // 依赖属性（没有 setter 的计算属性）：
  // 这暴露了观察者模式的一个复杂性问题——
  // 当字段依赖于其他字段且没有独立 setter 时，通知变得困难。
  // 观察 set_age() 方法的复杂度：
  // - (1): 保存旧状态
  // - (2): 比较新旧状态差异
  // - (3): 如有变化则通知
  bool get_can_vote() const { return age >= 16; } // 计算属性：年满 16 岁可投票
};

// 具体观察者（ConcreteObserver）：控制台观察者
// 当 Person 的年龄或投票资格变化时打印信息
struct ConsolePersonObserver : public Observer<Person> {
private:
  // 实现 field_changed() 回调：根据字段名打印不同信息
  void field_changed(Person &source, const std::string &field_name) override {
    cout << "[ConsolePersonObserver] Person's " << field_name
         << " has changed to ";
    if (field_name == "age")
      cout << source.get_age();
    if (field_name == "can_vote")
      cout << boolalpha << source.get_can_vote();
    cout << ".\n";
  }
};

// 客户端：演示观察者模式的订阅、通知和取消订阅
int main() {
  Person p;
  ConsolePersonObserver cpo;

  p.subscribe(cpo); // 注册观察者

  p.set_age(15);    // 年龄变化通知
  p.set_age(16);    // 年龄变化 + can_vote 变化通知
  p.set_age(17);    // 仅年龄变化通知

  p.unsubscribe(cpo); // 取消注册

  p.set_age(18);    // 没有观察者，不打印任何内容

  return 0;
}
