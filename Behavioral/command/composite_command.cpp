/*
 * =============================================================================
 * 设计模式：命令模式（Command）—— 组合命令（宏命令）
 * =============================================================================
 *
 * 【一句话概括】
 * 将多个命令组合成一个宏命令，实现复杂的事务性操作，并支持失败时的回滚。
 *
 * 【适用场景 —— 通用】
 * - 需要将多个原子命令组合为一个事务性操作（如转账 = 取款 + 存款）
 * - 需要在组合命令中处理部分失败的情况
 * - 需要批量执行和批量撤销
 *
 * 【金融工程应用】
 * - 组合订单：跨品种套利订单 = 买入 A + 卖出 B，两腿必须全部成交或全部撤销
 *   PairTradeCommand = BuyCommand + SellCommand，失败时自动回滚已成交腿
 * - 展期操作：期货换月 = 平近月 + 开远月，两操作原子执行
 *   RolloverCommand = CloseNearMonth + OpenFarMonth
 * - 组合调仓：多资产再平衡 = 卖出超配资产 + 买入低配资产，组合命令保证一致性
 * - 批量风控操作：触发熔断后批量撤销所有未成交订单，
 *   CancelAllCommand = CancelOrder1 + CancelOrder2 + ...，失败时记录并重试
 *
 * 【关键参与者】
 *   - Command（命令接口）：定义 call() 和 undo()
 *   - CompositeBankAccountCommand（组合命令）：将多个命令组合在一起
 *   - MoneyTransferCommand（转账命令）：由取款+存款组成
 */

#include <initializer_list>
#include <iostream>
#include <vector>

using namespace std;

// 银行账户（Receiver 接收者）
struct BankAccount {
  int balance{0};
  int overdraft_limit{-500};

  void deposit(int amount) {
    balance += amount;
    cout << "deposited " << amount << ", balance is now " << balance << "\n";
  }

  bool withdraw(int amount) {
    if (balance - amount >= overdraft_limit) {
      balance -= amount;
      cout << "withdrew " << amount << ", balance is now " << balance << "\n";
      return true; // 操作成功
    }
    return false;  // 操作失败
  }

  friend std::ostream &operator<<(std::ostream &os,
                                  const BankAccount &account) {
    os << "balance: " << account.balance;
    return os;
  }
};

// 命令接口
struct Command {
  bool succeeded{false};
  virtual void call() = 0;
  virtual void undo() = 0;
};

// 原子命令：单个银行账户操作
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
      succeeded = true;
      break;
    case withdraw:
      succeeded = account.withdraw(amount);
      break;
    }
  }

  void undo() override {
    if (!succeeded) {
      return;
    }

    switch (action) {
    case deposit:
      account.withdraw(amount);
      break;
    case withdraw:
      account.deposit(amount);
      break;
    }
  }
};

// 组合命令：将多个命令打包为一个宏命令（Macro Command）
// 它本身既是命令，又是一个命令容器
struct CompositeBankAccountCommand : vector<BankAccountCommand>, Command {

  explicit CompositeBankAccountCommand(
      const initializer_list<BankAccountCommand> &items)
      : vector(items) {}

  // 执行所有子命令
  void call() override {
    for (auto &cmd : *this) {
      cmd.call();
    }
  }

  // 按相反顺序撤销所有子命令
  void undo() override {
    for (auto it = rbegin(); it != rend(); ++it) {
      it->undo();
    }
  }
};

// 依赖组合命令：处理命令间的失败传播
// 如果前一个命令失败，后续命令不应执行（一致性保证）
struct DependentCompositeCommand : CompositeBankAccountCommand {
  explicit DependentCompositeCommand(
      const initializer_list<BankAccountCommand> &items)
      : CompositeBankAccountCommand(items) {}

  // 改进的 call() 实现：失败时停止流水线
  // 更完善的实现还可以加入回滚语义（rollback），撤销所有已执行的操作
  void call() override {
    bool ok = true;
    for (auto &cmd : *this) {
      if (ok) {
        cmd.call();
        ok = cmd.succeeded; // 更新成功标志
      } else {
        cmd.succeeded = false; // 标记后续命令为失败
      }
    }
  }
};

// 转账命令（具体组合命令）：转账 = 取款（from） + 存款（to）
struct MoneyTransferCommand : DependentCompositeCommand {
  explicit MoneyTransferCommand(BankAccount &from, BankAccount &to, int amount)
      : DependentCompositeCommand({
            BankAccountCommand{from, BankAccountCommand::withdraw, amount}, // 先从 from 账户取款
            BankAccountCommand{to, BankAccountCommand::deposit, amount},   // 再向 to 账户存款
        }) {}
};

int main() {
  BankAccount ba;   // 账户 A
  BankAccount ba2;  // 账户 B

  ba.deposit(100); // 账户 A 初始存入 100
  auto cmd1 = MoneyTransferCommand(ba, ba2, 25); // 转账 25：A -> B
  cmd1.call();
  cmd1.undo(); // 撤销转账
  cout << ba << " - " << ba2 << endl; // A: 100, B: 0

  cout << endl;

  // 尝试转账超过余额的金额（会失败）
  auto cmd2 = MoneyTransferCommand(ba, ba2, 5000);
  cmd2.call(); // 取款 5000 失败，后续存款也被标记为失败
  cout << ba << " - " << ba2 << endl; // 两个账户都不变
  cmd2.undo(); // 撤销无效果（因为没有成功执行的命令）
  cout << ba << " - " << ba2 << endl;

  return 0;
}
