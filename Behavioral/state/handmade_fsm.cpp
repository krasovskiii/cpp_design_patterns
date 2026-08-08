/*
 * =============================================================================
 * 设计模式：状态模式（State）—— 基于转换表的手工状态机
 * =============================================================================
 *
 * 【一句话概括】
 * 使用 map<State, vector<Transition>> 定义状态转换表，集中管理状态转换规则。
 *
 * 【适用场景 —— 通用】
 * - 状态转换规则较多且需要集中管理
 * - 希望通过数据驱动状态机行为
 * - 需要在不修改代码的情况下调整状态转换规则
 *
 * 【金融工程应用】
 * - 订单状态机：订单生命周期（新建→已提交→部分成交→全部成交→已撤销→已拒绝），
 *   转换表集中管理所有合法状态转换，非法转换自动拒绝
 *   如"已撤销→部分成交"的转换不存在于表中，自动拦截
 * - 交易会话状态机：市场会话（开盘前→集合竞价→连续交易→午休→连续交易→收盘→清算），
 *   每个状态下允许的操作不同，转换表数据驱动
 * - 策略生命周期状态机：策略状态（初始化→运行中→暂停→停止→错误），
 *   转换表定义合法转换，防止策略从"停止"直接到"运行"
 * - 风控状态机：风控级别（正常→预警→限制→熔断→恢复），转换表驱动级别切换
 *
 * 【关键参与者】
 *   - State（状态）：枚举 State，表示电话的各种状态
 *   - Trigger/Event（事件/触发器）：枚举 Trigger，触发状态转换的事件
 *   - Transition（转换）：pair<Trigger, State>
 *   - FSM（状态机）：map<State, Transitions>，状态转换表
 */

#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

// 状态枚举：电话的可能状态
enum class State { OffHook, Connecting, Connected, OnHold, OnHook };

// 触发器/事件枚举：触发状态转换的事件
enum class Trigger {
  CallDialed,       // 拨号
  HungUp,           // 挂断
  CallConnected,    // 接通
  PlacedOnHold,     // 保持
  TakenOffHold,     // 取消保持
  LeftMessage,      // 留言
  StopUsingPhone    // 停止使用电话
};

// 状态输出流操作符
inline ostream &operator<<(ostream &os, const State &s) {
  switch (s) {
  case State::OffHook:
    os << "off the hook";    // 摘机
    break;
  case State::Connecting:
    os << "connecting";      // 连接中
    break;
  case State::Connected:
    os << "connected";       // 已接通
    break;
  case State::OnHold:
    os << "on hold";         // 保持中
    break;
  case State::OnHook:
    os << "on the hook";     // 挂机
    break;
  }
  return os;
}

// 触发器输出流操作符
inline ostream &operator<<(ostream &os, const Trigger &t) {
  switch (t) {
  case Trigger::CallDialed:
    os << "call dialed";          // 拨号
    break;
  case Trigger::HungUp:
    os << "hung up";              // 挂断
    break;
  case Trigger::CallConnected:
    os << "call connected";       // 接通
    break;
  case Trigger::PlacedOnHold:
    os << "placed on hold";       // 保持
    break;
  case Trigger::TakenOffHold:
    os << "taken off hold";       // 取消保持
    break;
  case Trigger::LeftMessage:
    os << "left message";         // 留言
    break;
  case Trigger::StopUsingPhone:
    os << "putting phone on hook"; // 挂机
    break;
  default:
    break;
  }
  return os;
}

// 转换定义：pair<触发器, 目标状态>
using Transition = pair<Trigger, State>;
using Transitions = vector<Transition>;

int main() {

  // 有限状态机（FSM）转换表：每个状态对应一组可能的转换
  map<State, Transitions> rules;

  // 摘机状态的转换规则
  rules[State::OffHook] = {{Trigger::CallDialed, State::Connecting},     // 拨号 -> 连接中
                           {Trigger::StopUsingPhone, State::OnHook}};    // 放回电话 -> 挂机

  // 连接中状态的转换规则
  rules[State::Connecting] = {{Trigger::HungUp, State::OffHook},         // 挂断 -> 摘机
                              {Trigger::CallConnected, State::Connected}}; // 接通 -> 已接通

  // 已接通状态的转换规则
  rules[State::Connected] = {{Trigger::LeftMessage, State::OffHook},     // 留言 -> 摘机
                             {Trigger::HungUp, State::OffHook},          // 挂断 -> 摘机
                             {Trigger::PlacedOnHold, State::OnHold}};    // 保持 -> 保持中

  // 保持中状态的转换规则
  rules[State::OnHold] = {{Trigger::TakenOffHold, State::Connected},     // 取消保持 -> 已接通
                          {Trigger::HungUp, State::OffHook}};            // 挂断 -> 摘机

  State currentState{State::OffHook}; // 初始状态：摘机
  State exitState{State::OnHook};     // 退出状态：挂机

  while (true) {
    cout << "The phone is currently " << currentState << endl;
  select_trigger:

    // 显示当前状态下可用的触发器
    cout << "Select a trigger:"
         << "\n";

    int i = 0;
    for (auto item : rules[currentState]) {
      cout << i++ << ". " << item.first << "\n";
    }

    // 读取用户选择的触发器
    size_t input;
    cin >> input;

    // 错误处理：检查输入是否有效
    if ((input + 1) > rules[currentState].size()) {
      cout << "Incorrect option. Please try again."
           << "\n";
      goto select_trigger; // 重新选择
    }

    // 根据转换表更新当前状态
    currentState = rules[currentState][input].second;
    if (currentState == exitState)
      break; // 到达退出状态，结束循环
  }

  cout << "We are done using the phone"
       << "\n";

  return 0;
}
