/*
 * =============================================================================
 * 设计模式：备忘录模式（Memento）—— 完整的撤销/重做实现
 * =============================================================================
 *
 * 【一句话概括】
 * 通过保存操作历史的备忘录列表，实现多次撤销（undo）和重做（redo）。
 *
 * 【适用场景 —— 通用】
 * - 需要支持多级撤销/重做的应用（如文本编辑器、图形编辑器）
 *
 * 【金融工程应用】
 * - 策略参数版本管理：每次修改策略参数后保存快照，支持多级回退，
 *   参数调优过程可追溯、可回退到任意历史版本
 * - 订单操作历史：保存每次订单修改前的状态，支持多级撤销（撤单→恢复原单→再撤销），
 *   完整的操作审计链
 * - 投资组合变更历史：保存每次调仓前后的组合状态，支持回溯任意历史时点的组合构成
 *
 * 【关键参与者】
 *   - Originator（发起人）：BankAccount，创建备忘录并支持 undo/redo
 *   - Memento（备忘录）：Memento 类，存储余额快照
 *   - Caretaker（管理者）：BankAccount 自身管理备忘录历史列表
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
using namespace std;

// 备忘录（Memento）：存储余额快照
class Memento {
  int balance;

public:
  explicit Memento(int balance) : balance(balance) {}
  friend class BankAccount; // 友元：允许 BankAccount 访问私有数据
};

// 发起人（Originator）：支持完整 undo/redo 的银行账户
class BankAccount {
  int balance = 0;

  // 保存每次操作的快照（备忘录列表）
  vector<shared_ptr<Memento>> changes;
  size_t current; // 当前在历史记录中的位置索引

public:
  explicit BankAccount(const int balance) : balance(balance) {
    changes.emplace_back(make_shared<Memento>(balance)); // 保存初始状态
    current = 0;
  }

  // 存款操作：创建并保存备忘录，返回新备忘录
  shared_ptr<Memento> deposit(int amount) {
    balance += amount;
    auto m = make_shared<Memento>(balance);
    changes.push_back(m); // 将新状态加入历史
    ++current;            // 当前位置前移
    return m;
  }

  // 从指定备忘录恢复状态
  void restore(const shared_ptr<Memento> &m) {
    if (m) {
      balance = m->balance;
      changes.push_back(m);             // 恢复操作也记录在历史中
      current = changes.size() - 1;     // 当前位置更新为最新
    }
  }

  // 撤销（Undo）：回退到上一个历史状态
  shared_ptr<Memento> undo() {
    if (current > 0) {                  // 确保有可撤销的状态
      --current;                        // 位置回退
      auto m = changes[current];        // 获取上一个备忘录
      balance = m->balance;             // 恢复余额
      return m;
    }
    return {}; // 无法继续撤销
  }

  // 重做（Redo）：前进到下一个历史状态
  shared_ptr<Memento> redo() {
    if (current + 1 < changes.size()) { // 确保有可重做的状态
      ++current;                        // 位置前进
      auto m = changes[current];        // 获取下一个备忘录
      balance = m->balance;             // 恢复余额
      return m;
    }
    return {}; // 无法继续重做
  }

  friend ostream &operator<<(ostream &os, const BankAccount &obj) {
    return os << "balance: " << obj.balance;
  }
};

// 客户端：演示完整的撤销/重做流程
int main() {
  BankAccount ba{100};      // 初始余额: 100
  ba.deposit(50);           // 余额: 150
  ba.deposit(25);           // 余额: 175
  cout << ba << "\n";       // 输出: 175

  ba.undo();                // 撤销到 150
  cout << "Undo 1: " << ba << "\n";
  ba.undo();                // 撤销到 100
  cout << "Undo 2: " << ba << "\n";
  ba.redo();                // 重做到 150
  cout << "Redo 2: " << ba << "\n";
  ba.undo();                // 再次撤销到 100
  cout << "Undo 3: " << ba << "\n";
  return 0;
}
