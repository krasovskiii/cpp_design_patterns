/*
 * =============================================================================
 * 设计模式：观察者模式（Observer）—— 线程安全与死锁问题
 * =============================================================================
 *
 * 【一句话概括】
 * 演示在线程安全环境下，观察者在 notify() 期间调用 unsubscribe() 可能导致的死锁问题。
 *
 * 【适用场景】
 * - 多线程环境下的观察者模式
 * - 需要理解观察者模式中的重入（re-entrancy）问题
 *
 * 【金融工程应用】
 * - 多线程交易系统：行情推送线程 notify 策略线程，策略在回调中取消订阅时需注意死锁，
 *   使用递归锁或延迟取消订阅机制（标记删除+定期清理）
 * - 实时风控通知：风控线程 notify 告警时，告警回调中修改订阅列表可能导致死锁，
 *   设计时需使用线程安全的 Observer 实现
 *
 * 【关键参与者】
 *   - Subject（被观察者）：Person，继承 SaferObservable<Person>
 *   - Observer（观察者）：TrafficAdministration
 *   - SaferObservable（线程安全被观察者）：使用 mutex 保护操作
 */

#include "observer.h"
#include "safer_observable.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// 被观察者（Observable/Subject）：使用线程安全版本
class Person : public SaferObservable<Person> {
  int age{0};

public:
  Person() {}
  explicit Person(int age) : age(age) {}

  int get_age() const { return age; }

  // 设置年龄：变化时通知观察者
  void set_age(int age) {
    if (this->age == age)
      return;
    this->age = age;
    notify(*this, "age"); // 在持有锁的情况下通知所有观察者
  }
};

// 控制台观察者：当年龄变化时打印信息
struct ConsolePersonObserver : public Observer<Person> {
private:
  void field_changed(Person &source, const std::string &field_name) override {
    cout << "[ConsolePersonObserver] Person's " << field_name
         << " has changed to ";
    if (field_name == "age")
      cout << source.get_age();
    cout << ".\n";
  }
};

// 交通管理局观察者：当年龄变化时检查驾驶资格
// 注意：此观察者在回调中取消订阅，可能导致死锁！
struct TrafficAdministration : Observer<Person> {
  void field_changed(Person &source, const std::string &field_name) override {
    if (field_name == "age") {
      if (source.get_age() < 17)
        cout << "[TrafficAdministration] Whoa there, you're not old enough to "
                "drive!\n";
      else {
        // 取消订阅发生在通知回调内部 -> 可能导致死锁！
        // 因为 notify() 持有 mutex，而 unsubscribe() 也尝试获取同一个 mutex
        cout << "[TrafficAdministration] Oh, ok, we no longer care!\n";
        source.unsubscribe(*this); // 危险操作！在 notify 中调用 unsubscribe
      }
    }
  }
};

// 客户端：演示线程安全观察者模式中的死锁问题
int main() {
  Person p;

  // 注册多个观察者
  // ConsolePersonObserver cpo;
  TrafficAdministration ta;
  p.subscribe(ta);
  // p.subscribe(cpo);

  try {
    p.set_age(15); // TrafficAdministration: 年龄不足，不能驾驶
    p.set_age(16); // TrafficAdministration: 年龄不足，不能驾驶
    p.set_age(17); // TrafficAdministration: 年龄够了，取消订阅（此处死锁！）
    p.set_age(18); // 不会执行到此处
    p.set_age(19);
  } catch (const std::exception &e) {
    cout << "Oops, " << e.what() << "\n";
  }

  return 0;
}
