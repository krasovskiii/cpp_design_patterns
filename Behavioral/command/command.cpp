/*
 * =============================================================================
 * 设计模式：命令模式（Command）
 * =============================================================================
 *
 * 【一句话概括】
 * 将请求封装为对象，支持请求排队、记录日志和可撤销操作。
 *
 * 【适用场景 —— 通用】
 * - 需要将操作参数化，以便延迟执行、排队或远程执行
 * - 需要支持撤销（undo）操作
 * - 需要记录操作日志，以便在系统崩溃时恢复
 * - 任务队列/线程池：将任务封装为 Command 放入队列，工作线程依次执行
 *
 * 【金融工程应用】
 * - 订单命令队列：将下单/撤单/改单封装为 OrderCommand，放入队列顺序执行，
 *   支持订单日志记录、延迟执行（定时单）、批量处理
 * - 交易回放系统：将所有交易操作记录为 Command 序列（含时间戳），
 *   盘后按时间顺序回放，用于复盘分析和问题排查
 * - 策略信号队列：策略产生的信号封装为 SignalCommand，顺序传递给执行模块，
 *   支持信号优先级排序和信号合并
 * - 风控操作日志：所有风控操作（加保证金/强平/限制交易）封装为 RiskCommand 记录，
 *   确保操作可审计、可追溯
 *
 * 【关键参与者】
 *   - Command（命令接口）：Command 抽象类，定义 call() 接口
 *   - ConcreteCommand（具体命令）：BankAccountCommand
 *   - Receiver（接收者）：BankAccount，实际执行操作的业务对象
 */

#include <iostream>
#include <vector>

using namespace std;

// 银行账户（Receiver 接收者）：实际执行业务操作的对象
struct BankAccount {
  int balance{0};              // 当前余额
  int overdraft_limit{-500};   // 透支限额（允许负余额不超过 -500）

  // 存款操作
  void deposit(int amount) {
    balance += amount;
    cout << "deposited " << amount << ", balance is now " << balance << "\n";
  }

  // 取款操作：仅在不超过透支限额时才执行
  void withdraw(int amount) {
    if (balance - amount >= overdraft_limit) {
      balance -= amount;
      cout << "withdrew " << amount << ", balance is now " << balance << "\n";
    }
  }

  friend std::ostream &operator<<(std::ostream &os,
                                  const BankAccount &account) {
    os << "balance: " << account.balance;
    return os;
  }
};

// 命令接口（Command）：所有具体命令必须实现 call() 方法
struct Command {
  // 多态基类需要虚析构（Effective C++ 条款 7）
  virtual ~Command() = default;
  virtual void call() = 0; // 纯虚函数：执行命令
};

// 银行账户命令（ConcreteCommand 具体命令）
struct BankAccountCommand : Command {
  BankAccount &account;                       // 命令操作的账户（接收者）
  // 用 enum class 替代旧式 enum，作用域更清晰，避免命名污染（Modern C++）
  enum class Action { deposit, withdraw } action;
  int amount;                                 // 操作金额

  BankAccountCommand(BankAccount &account, Action action, int amount)
      : account{account}, action{action}, amount{amount} {}

  // 执行命令：根据 action 类型调用相应的账户操作
  void call() override {
    switch (action) {
    case Action::deposit:
      account.deposit(amount); // 委托给接收者执行存款
      break;
    case Action::withdraw:
      account.withdraw(amount); // 委托给接收者执行取款
      break;
    }
  }
};

// 客户端：创建命令对象并批量执行
int main() {
  BankAccount ba;
  // 将多个命令封装为对象并放入容器（用 enum class 作用域限定）
  vector<BankAccountCommand> commands{
      BankAccountCommand{ba, BankAccountCommand::Action::deposit, 100},  // 存款 100
      BankAccountCommand{ba, BankAccountCommand::Action::withdraw, 200}, // 取款 200
  };
  cout << ba << endl; // 初始余额：0
  for (auto &cmd : commands) {
    cmd.call(); // 依次执行每个命令
  }
  cout << ba << endl; // 最终余额

  return 0;
}
