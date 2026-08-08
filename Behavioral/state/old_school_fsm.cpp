/*
 * =============================================================================
 * 设计模式：状态模式（State）—— 传统有限状态机
 * =============================================================================
 *
 * 【一句话概括】
 * 允许对象在其内部状态改变时改变其行为，看起来就像对象的类发生了变化。
 *
 * 【适用场景 —— 通用】
 * - 对象的行为取决于其状态，且必须在运行时根据状态改变行为
 * - 操作包含大量与状态相关的多分支条件语句
 *
 * 【金融工程应用】
 * - 策略运行状态：策略在不同状态下（就绪/运行/暂停/停止/错误）响应相同命令行为不同，
 *   start() 在"就绪"状态有效，在"运行"状态忽略
 * - 订单执行状态：订单在不同状态下（新建/已报/部分成交/全部成交/已撤销）
 *   对 cancel() 的响应不同，已成交订单无法撤销
 *
 * 【注意】这种老式实现使用 "delete this"，是危险的反模式，不推荐。
 *
 * 【关键参与者】
 *   - Context（上下文）：LightSwitch，持有当前状态引用
 *   - State（状态接口）：State 基类，定义 on() 和 off() 行为
 *   - ConcreteState（具体状态）：OnState、OffState
 */

#include <iostream>
#include <string>
using namespace std;

// 前置声明上下文类
class LightSwitch;

// 状态接口（State）：定义电灯的行为
struct State {
  virtual ~State(){};
  // 默认行为：如果已在目标状态，打印提示
  virtual void on(LightSwitch *) { cout << "Light is already on\n"; }
  virtual void off(LightSwitch *) { cout << "Light is already off\n"; }
};

// 具体状态：开状态（OnState）
struct OnState : State {
  OnState() { cout << "Light turned on\n"; }

  // 仅重写 off() 方法（开状态只需要处理关灯操作）
  void off(LightSwitch *ls) override;
};

// 具体状态：关状态（OffState）
struct OffState : State {
  OffState() { cout << "Light turned off\n"; }

  // 仅重写 on() 方法（关状态只需要处理开灯操作）
  void on(LightSwitch *ls) override;
};

// 上下文（Context）：电灯开关
class LightSwitch {
  State *state; // 当前状态

public:
  explicit LightSwitch() { state = new OffState(); } // 初始状态为关
  void set_state(State *state) { this->state = state; } // 切换状态
  void on() { state->on(this); }   // 委托给当前状态处理
  void off() { state->off(this); } // 委托给当前状态处理
};

// 开状态 -> 关状态的转换
void OnState::off(LightSwitch *ls) {
  cout << "Switching light off...\n";
  ls->set_state(new OffState()); // 创建新状态并设置
  delete this; // 删除自身：这是危险的做法！red flag!
}

// 关状态 -> 开状态的转换
void OffState::on(LightSwitch *ls) {
  cout << "Switching light on...\n";
  ls->set_state(new OnState()); // 创建新状态并设置
  delete this; // 删除自身：这是危险的做法！
}

// 客户端：演示状态机
int main() {
  LightSwitch ls;
  ls.on();   // 关 -> 开
  ls.off();  // 开 -> 关
  ls.off();  // 已在关状态，打印提示
  return 0;
}
