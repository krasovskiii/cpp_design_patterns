/*
 * ===========================================================================
 * 设计模式：Prototype Motivation（原型模式 —— 动机与问题引入）
 * ===========================================================================
 *
 * 【核心思想】
 * 原型模式通过复制（克隆）现有对象来创建新对象，而不是通过构造函数。
 * 这在对象创建成本高昂、或者需要从已有对象"微调"出新对象时特别有用。
 *
 * 【适用场景 —— 通用】
 * - 创建对象成本高昂（如需要数据库查询、网络请求、复杂计算）
 * - 需要创建与现有对象仅有少量差异的新对象
 * - 希望避免重复的初始化代码
 * - 对象克隆：需要在运行时动态复制对象状态（如撤销/重做系统中的状态快照）
 *
 * 【金融工程应用】
 * - 策略参数微调：已有策略实例的 Sharpe=2.0，想尝试把止损从 5% 调整到 3%，
 *   原型复制后微调参数，保留所有已验证的默认配置
 *   如果重新构造，需要重新设置 20+ 个参数，容易遗漏
 * - 回测参数网格搜索：遍历参数网格（如 MA 周期 5/10/20，止损 2%/3%/5%），
 *   从模板策略原型克隆出每个参数组合，避免重复初始化
 * - 投资组合状态快照：在风控系统中定期对当前组合持仓做深拷贝快照，
 *   用于事后审计、what-if 分析和回滚操作
 * - 市场数据快照：盘中定时对实时行情对象做深拷贝，保存为历史快照序列，
 *   用于盘后回放和策略复盘
 *
 * 【解决的问题】
 * 本例展示了没有原型模式时的三种对象复制方式及其问题：
 *
 * 1. "不使用原型"：每次都用构造函数创建 → 代码重复，容易出错
 * 2. "浅拷贝（值类型）"：当成员是值类型时，默认拷贝可行
 * 3. "浅拷贝（指针类型）"：当成员是指针时，默认拷贝只复制指针 → 两个对象
 *    共享同一块内存，修改一个会影响另一个！
 *
 * 结论：当对象包含指针成员时，需要原型模式实现深拷贝。
 *
 * 【UML 关键参与者】
 * - Prototype（原型）：Contact —— 提供克隆自身的能力
 * - 本例是"动机说明"，后续文件才展示具体的原型实现
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

/*
 * Address: 地址结构体
 *
 * 一个简单的值类型，包含街道、城市和门牌号。
 */
struct Address {
  string street, city;
  int suite;

  Address(const string &street, const string &city, int suite)
      : street(street), city(city), suite(suite) {}

  friend ostream &operator<<(ostream &os, const Address &address) {
    return os << "street: " << address.street << ", city: " << address.city
              << ", suite: " << address.suite;
  }
};

/*
 * Contact: 联系人（值类型版本）
 *
 * address 成员是值类型（Address 对象直接嵌入，非指针）。
 * 这种情况下，默认拷贝行为是正确的 —— 会复制整个 Address 对象。
 */
struct Contact {
  string name;
  Address address;  // 值类型成员

  Contact(const string &name, const Address &address)
      : name(name), address(address) {}

  friend ostream &operator<<(ostream &os, const Contact &contact) {
    return os << "name: " << contact.name << ", address: " << contact.address;
  }
};

/*
 * ContactPtr: 联系人（指针类型版本）
 *
 * address 成员是指针类型（Address*）。
 * 这种情况下，默认拷贝行为有问题 —— 只复制了指针，两个 Contact 对象
 * 共享同一个 Address 实例！
 *
 * 关键问题：谁拥有这个指针？谁负责释放它？
 * 默认拷贝不处理这些问题，导致悬空指针或双重释放风险。
 */
struct ContactPtr {
  string name;
  Address *address;

  // 注意：这里没有定义谁拥有这个指针的所有权！
  ContactPtr(const string &name, Address *address)
      : name(name), address(address) {}

  friend ostream &operator<<(ostream &os, const ContactPtr &contact) {
    return os << "name: " << contact.name << ", address: " << *contact.address;
  }
};

int main() {
  // ===== 场景一：不使用原型，重复构造 =====
  // 问题：每次都要重复写相同的地址信息
  cout << endl << "Using constructor: " << endl;
  Contact john{"John Doe", Address{"123 East Dr", "London", 123}};
  Contact jane1{"Jane Smith", Address{"123 East Dr", "London", 103}};
  cout << john << endl;
  cout << jane1 << endl;

  // ===== 场景二：使用默认拷贝（值类型成员） =====
  // 结果正确：因为 address 是值类型，拷贝时会复制整个 Address 对象
  cout << endl << "Cloning class without pointers: " << endl;
  Contact jane2 = john;           // 拷贝 john
  jane2.name = "Jane Smith";     // 修改名字
  jane2.address.suite = 103;     // 修改门牌号 —— 只影响 jane2！
  cout << john << endl;          // john 保持不变
  cout << jane2 << endl;

  // ===== 场景三：使用默认拷贝（指针类型成员） =====
  // 问题：浅拷贝只复制了指针，两个对象共享同一个 Address
  // 结果：修改 jane3 的 address 也影响了 john2！
  cout << endl << "Cloning class with ptr: " << endl;
  ContactPtr john2{"John Doe", new Address{"123 East Dr", "London", 100}};
  ContactPtr jane3 = john2;       // 浅拷贝！只复制了指针
  jane3.name = "Jane Smith";
  cout << john2 << endl;
  jane3.address->suite = 99999;  // 修改 jane3 的地址...
  cout << jane3 << endl;
  cout << john2 << endl;          // john2 的地址也被修改了！

  return 0;
}
