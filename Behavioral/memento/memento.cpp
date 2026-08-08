/*
 * =============================================================================
 * 设计模式：备忘录模式（Memento）
 * =============================================================================
 *
 * 【一句话概括】
 * 在不破坏封装的前提下，捕获并外部化一个对象的内部状态，以便之后恢复。
 *
 * 【适用场景 —— 通用】
 * - 需要保存对象状态的快照以便以后恢复
 * - 直接获取对象状态会破坏封装性
 * - 需要实现撤销（undo）功能
 *
 * 【金融工程应用】
 * - 策略状态快照：在关键时点（开盘/午休/收盘）保存策略的完整状态快照，
 *   系统崩溃后可从最近快照恢复，避免从零重跑
 * - 回测断点续跑：回测中定期保存状态快照（当前K线位置/持仓/权益曲线），
 *   中断后从断点继续，避免重新计算数小时的回测
 * - 风控状态检查点：风控模块在每次检查后保存状态快照，
 *   用于事后分析风控决策是否合理
 * - 投资组合快照：每日收盘后保存组合快照（持仓/成本/盈亏），
 *   用于绩效归因和历史回溯
 *
 * 【关键参与者】
 *   - Originator（发起人）：BankAccount，创建备忘录并从中恢复状态
 *   - Memento（备忘录）：Memento 类，存储内部状态
 *   - Caretaker（管理者）：main() 函数，保存备忘录但不操作其内容
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
using namespace std;

// 备忘录（Memento）：仅存储余额的快照
class Memento {
  int balance; // 被封装的内部状态

public:
  explicit Memento(int balance) : balance(balance) {}

  // 友元类声明：允许 BankAccount 访问私有成员 balance
  // 这是保持封装性的关键设计
  friend class BankAccount;
};

// 发起人（Originator）：银行账户，可以创建和恢复备忘录
class BankAccount {
  int balance = 0; // 内部状态

public:
  explicit BankAccount(const int balance) : balance(balance) {}

  // 存款操作：返回一个包含新余额的备忘录（Token）
  Memento deposit(int amount) {
    balance += amount;
    return Memento{balance}; // 创建并返回当前状态的快照
  }

  // 从备忘录恢复状态
  void restore(const Memento &m) { balance = m.balance; }

  friend ostream &operator<<(ostream &os, const BankAccount &obj) {
    return os << "balance: " << obj.balance;
  }
};

// 客户端（Caretaker 管理者）：保存备忘录并演示恢复
int main() {
  BankAccount ba{100};
  auto m1 = ba.deposit(50); // 余额: 150, 创建备忘录 m1
  auto m2 = ba.deposit(25); // 余额: 175, 创建备忘录 m2
  cout << ba << "\n";       // 输出: 175

  // 撤销到 m1 状态（余额: 150）
  ba.restore(m1);
  cout << ba << "\n";       // 输出: 150

  // 重做到 m2 状态（余额: 175）
  ba.restore(m2);
  cout << ba << "\n";       // 输出: 175

  return 0;
}
