/*
 * =============================================================================
 * 设计模式：状态模式（State）—— 基于模板特化的编译期状态机
 * =============================================================================
 *
 * 【一句话概括】
 * 使用 C++ 模板特化实现状态机，编译期验证状态转换的正确性，零运行时开销。
 *
 * 【适用场景】
 * - 需要在编译期验证状态转换的正确性
 * - 希望利用 C++ 模板系统实现零开销抽象
 *
 * 【金融工程应用】
 * - 高频交易订单状态机：编译期验证订单状态转换，零虚函数开销，
 *   适合延迟敏感的高频交易场景
 * - 编译期验证的交易会话状态机：在编译期保证所有状态转换合法，
 *   运行时零开销，避免运行时状态检查的性能损耗
 *
 * 【关键参与者】
 *   - State（状态）：枚举 State，表示电话的各种状态
 *   - Event（事件）：CallDialed、HungUp 等空结构体
 *   - PhoneStateMachine（状态机）：持有当前状态，通过模板特化处理事件
 */

#include <functional>
#include <iostream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>
using namespace std;

// 手工实现的有限状态机（替代 Boost.MSM）
// 实现相同的电话状态机：摘机(OffHook) -> 连接中(Connecting) -> 已接通(Connected) -> 保持中(OnHold) -> 电话销毁(PhoneDestroyed)

// ---------------------------------------------------------------------------
// 状态枚举
enum class State {
  OffHook,          // 摘机
  Connecting,       // 连接中
  Connected,        // 已接通
  OnHold,           // 保持中
  PhoneDestroyed,   // 电话已销毁（终态）
  _Count            // 哨兵值，用于获取状态数量
};

// 状态名称数组，用于打印
vector<string> state_names{"off hook", "connecting", "connected", "on hold",
                           "destroyed"};

// ---------------------------------------------------------------------------
// 事件/转换触发器（使用空结构体作为类型标签）
struct CallDialed {};           // 拨号事件
struct HungUp {};               // 挂断事件
struct CallConnected {};        // 接通事件
struct PlacedOnHold {};         // 保持事件
struct TakenOffHold {};         // 取消保持事件
struct LeftMessage {};          // 留言事件
struct PhoneThrownIntoWall {};  // 摔电话事件

// 类型擦除的事件处理器：可以持有任意事件的处理函数
using EventHandler = function<void()>;

// ---------------------------------------------------------------------------
// 手工状态机类
class PhoneStateMachine {
  State state_{State::OffHook}; // 初始状态：摘机
  bool angry_{true};            // 是否愤怒（影响 PhoneThrownIntoWall 事件的处理）

public:
  bool angry() const { return angry_; }

  State current_state() const { return state_; }

  // 通用事件处理模板：默认情况下，未定义转换的事件会触发 static_assert 编译错误
  template <typename Event> void process_event(const Event &) {
    static_assert(sizeof(Event) == 0,
                  "No transition defined for this event type");
  }
};

// 模板特化：处理 CallDialed 事件（拨号）
// 摘机(OffHook) -> 连接中(Connecting)
template <> void PhoneStateMachine::process_event(const CallDialed &) {
  if (state_ == State::OffHook) {
    state_ = State::Connecting; // 状态转换
  } else {
    cout << "No transition from state " << state_names[static_cast<int>(state_)]
         << " on event CallDialed" << endl;
  }
}

// 模板特化：处理 CallConnected 事件（接通）
// 连接中(Connecting) -> 已接通(Connected)
template <> void PhoneStateMachine::process_event(const CallConnected &) {
  if (state_ == State::Connecting) {
    state_ = State::Connected; // 状态转换
  } else {
    cout << "No transition from state " << state_names[static_cast<int>(state_)]
         << " on event CallConnected" << endl;
  }
}

// 模板特化：处理 PlacedOnHold 事件（保持）
// 已接通(Connected) -> 保持中(OnHold)
template <> void PhoneStateMachine::process_event(const PlacedOnHold &) {
  if (state_ == State::Connected) {
    state_ = State::OnHold; // 状态转换
  } else {
    cout << "No transition from state " << state_names[static_cast<int>(state_)]
         << " on event PlacedOnHold" << endl;
  }
}

// 模板特化：处理 PhoneThrownIntoWall 事件（摔电话）
// 保持中(OnHold) + angry_ == true -> 电话销毁(PhoneDestroyed)
template <> void PhoneStateMachine::process_event(const PhoneThrownIntoWall &) {
  if (state_ == State::OnHold) {
    if (angry_) {
      cout << "Phone breaks into a million pieces" << endl;
      state_ = State::PhoneDestroyed; // 状态转换（终态）
    }
  } else {
    cout << "No transition from state " << state_names[static_cast<int>(state_)]
         << " on event PhoneThrownIntoWall" << endl;
  }
}

// 模板特化：处理 HungUp 事件（挂断）
// 无有效转换，打印提示
template <> void PhoneStateMachine::process_event(const HungUp &) {
  cout << "No transition from state " << state_names[static_cast<int>(state_)]
       << " on event HungUp" << endl;
}

// 模板特化：处理 TakenOffHold 事件（取消保持）
// 无有效转换，打印提示
template <> void PhoneStateMachine::process_event(const TakenOffHold &) {
  cout << "No transition from state " << state_names[static_cast<int>(state_)]
       << " on event TakenOffHold" << endl;
}

// 模板特化：处理 LeftMessage 事件（留言）
// 无有效转换，打印提示
template <> void PhoneStateMachine::process_event(const LeftMessage &) {
  cout << "No transition from state " << state_names[static_cast<int>(state_)]
       << " on event LeftMessage" << endl;
}

// ---------------------------------------------------------------------------

int main() {
  PhoneStateMachine phone;

  // Lambda 辅助函数：打印当前状态
  auto info = [&]() {
    auto i = static_cast<int>(phone.current_state());
    cout << "The phone is currently " << state_names[i] << "\n";
  };

  info();                                      // 摘机
  phone.process_event(CallDialed{});            // 拨号 -> 连接中
  info();
  phone.process_event(CallConnected{});         // 接通 -> 已接通
  info();
  phone.process_event(PlacedOnHold{});          // 保持 -> 保持中
  info();
  phone.process_event(PhoneThrownIntoWall{});   // 愤怒摔电话 -> 电话销毁
  info();

  // 尝试在电话销毁后继续操作
  phone.process_event(CallDialed{}); // 无效转换

  cout << "We are done using the phone"
       << "\n";

  return 0;
}
