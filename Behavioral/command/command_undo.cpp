/*
 * =============================================================================
 * 设计模式：命令模式（Command）—— 支持撤销操作
 * =============================================================================
 *
 * 【一句话概括】
 * 在命令接口中增加 undo() 方法，使每个命令不仅能够执行，还能回滚其效果。
 *
 * 【适用场景 —— 通用】
 * - 需要支持撤销操作的应用（如文本编辑器、图形编辑器）
 * - 需要记录操作历史以便回滚
 * - 需要实现事务性操作
 *
 * 【金融工程应用】
 * - 订单撤销：已提交的订单通过 undo() 实现撤单，保留完整的操作历史
 * - 策略参数回滚：修改策略参数后如果表现恶化，通过 undo() 快速回退到上一版本参数
 * - 交易对冲：将买入命令的 undo() 设计为卖出，实现简单的对冲操作
 *
 * 【关键参与者】
 *   - Command（命令接口）：定义 call() 和 undo() 两个纯虚函数
 *   - ConcreteCommand（具体命令）：BankAccountCommand
 *   - Receiver（接收者）：BankAccount
 */

#include <iostream>
#include <vector>

using namespace std;

// 银行账户（Receiver 接收者）
struct BankAccount {
  int balance{0};
  int overdraft_limit{-500};

  // 存款操作（始终成功）
  void deposit(int amount) {
    balance += amount;
    cout << "deposited " << amount << ", balance is now " << balance << "\n";
  }

  // 取款操作：返回 bool 表示操作是否成功（撤销时需要知道这一点）
  bool withdraw(int amount) {
    if (balance - amount >= overdraft_limit) {
      balance -= amount;
      cout << "withdrew " << amount << ", balance is now " << balance << "\n";
      return true; // 取款成功
    }
    return false;  // 取款失败（超出透支限额）
  }

  friend std::ostream &operator<<(std::ostream &os,
                                  const BankAccount &account) {
    os << "balance: " << account.balance;
    return os;
  }
};

// 命令接口：支持执行和撤销
struct Command {
  bool succeeded{false};     // 记录命令是否成功执行
  virtual void call() = 0;   // 执行命令
  virtual void undo() = 0;   // 撤销命令
};

// 银行账户命令：支持撤销
struct BankAccountCommand : Command {
  BankAccount &account;
  enum Action { deposit, withdraw } action;
  int amount;

  BankAccountCommand(BankAccount &account, Action action, int amount)
      : account{account}, action{action}, amount{amount} {}

  void call() override {
    switch (action) {
    case deposit:
      account.deposit(amount);
      succeeded = true; // 存款始终成功
      break;
    case withdraw:
      succeeded = account.withdraw(amount); // 取款可能失败，记录结果
      break;
    }
  }

  void undo() override {
    if (!succeeded) {
      return; // 如果原始操作未成功，撤销无意义，直接返回
    }

    switch (action) {
    case deposit:
      // 撤销存款 = 取款
      account.withdraw(amount);
      break;
    case withdraw:
      // 撤销取款 = 存款
      // 注意：这里打破了开闭原则，因为撤销逻辑需要知道操作是否成功
      // 这可以通过在账户上维护一个布尔值或通过 succeeded 字段来实现
      account.deposit(amount);
      break;
    }
  }
};

int main() {
  BankAccount ba;
  vector<BankAccountCommand> commands{
      BankAccountCommand{ba, BankAccountCommand::deposit, 100},   // 存款 100
      BankAccountCommand{ba, BankAccountCommand::withdraw, 200},  // 取款 200（会因透支失败）
  };
  cout << ba << endl; // 初始余额：0

  // 执行所有命令
  for (auto &cmd : commands) {
    cmd.call();
  }

  // 按相反顺序执行撤销操作（后进先出，LIFO）
  for (auto it = commands.rbegin(); it != commands.rend(); ++it) {
    it->undo();
  }

  cout << ba << endl; // 撤销后余额应回到 0

  return 0;
}
