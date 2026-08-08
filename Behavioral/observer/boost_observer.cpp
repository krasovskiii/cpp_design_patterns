/*
 * =============================================================================
 * 设计模式：观察者模式（Observer）—— 基于 Signal/Slot 的实现
 * =============================================================================
 *
 * 【一句话概括】
 * 使用 Signal/Slot 机制替代传统的 Observer/Observable 接口，更灵活解耦。
 *
 * 【适用场景 —— 通用】
 * - 希望观察者无需继承特定接口（更灵活的解耦）
 * - 需要动态连接和断开回调函数
 * - 需要支持 lambda 表达式作为回调
 *
 * 【金融工程应用】
 * - 实时行情推送：QuoteSignal 价格更新时触发所有连接的槽函数（策略/风控/展示），
 *   各槽函数可以是 lambda/函数指针/成员函数，无需继承 Observer 基类
 * - 交易信号广播：SignalGenerator 产生信号时 Signal 广播，多个执行器同时接收，
 *   支持动态连接和断开，灵活配置信号流向
 * - 风控告警信号：RiskAlertSignal 触发时连接多个告警通道（短信/邮件/微信/声光），
 *   可动态添加新告警通道
 *
 * 【关键参与者】
 *   - Signal（信号）：模板类，管理槽函数列表并广播
 *   - Slot（槽/观察者）：通过 connect() 注册的回调函数
 */

#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

// 简单的信号/槽实现（替代 boost::signals2）
// 支持连接多个槽函数，调用时依次执行
template <typename... Args> class Signal {
public:
  using Slot = function<void(Args...)>;

  // 连接句柄：持有连接 ID 和信号指针
  struct Connection {
    int id;
    Signal *signal;

    // 断开此连接
    void disconnect() const {
      if (signal)
        signal->disconnect(id);
    }
  };

  // 连接槽函数：返回连接句柄用于后续断开
  Connection connect(Slot slot) {
    int id = next_id_++;
    slots_[id] = move(slot);
    return Connection{id, this};
  }

  // 调用信号：依次执行所有已连接的槽函数
  void operator()(Args... args) const {
    for (const auto &[id, slot] : slots_) {
      slot(args...);
    }
  }

private:
  void disconnect(int id) { slots_.erase(id); }

  int next_id_ = 0;
  unordered_map<int, Slot> slots_;
};

// 被观察者模板：持有 field_changed 信号
template <typename T> struct Observable {
  Signal<T &, const string &> field_changed; // 字段变化信号
};

// 具体被观察者：Person
class Person : public Observable<Person> {
  int age{0};

public:
  Person() {}
  explicit Person(int age) : age(age) {}

  int get_age() const { return age; }

  // 设置年龄：变化时触发 field_changed 信号
  void set_age(int age) {
    if (this->age == age)
      return;
    this->age = age;

    field_changed(*this, "age"); // 触发信号，通知所有连接的槽
  }
};

// 客户端：演示 Signal/Slot 模式的连接、通知和断开
int main() {
  Person p;

  // 订阅：连接一个 lambda 作为槽函数
  auto conn =
      p.field_changed.connect([](const Person &p, const string &field_name) {
        cout << field_name << " has changed to: " << p.get_age() << endl;
      });

  p.set_age(18); // 触发信号，lambda 被调用
  p.set_age(19);
  p.set_age(20);

  // 取消订阅：断开连接
  conn.disconnect();

  p.set_age(21); // 不再有回调

  return 0;
}
