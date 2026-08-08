/*
 * ===========================================================================
 * 设计模式：Prototype（原型模式 —— 拷贝构造函数实现）
 * ===========================================================================
 *
 * 【核心思想】
 * 通过实现拷贝构造函数（Copy Constructor）来实现深拷贝，解决指针成员
 * 的浅拷贝问题。当对象包含指针成员时，拷贝构造函数负责为新对象分配
 * 独立的内存，并将原对象的数据复制过去。
 *
 * 【适用场景 —— 通用】
 * - 对象包含指针或动态分配的资源
 * - 需要从现有对象创建独立副本
 * - 默认的浅拷贝（成员逐一复制）不够用时
 *
 * 【金融工程应用】
 * - 策略实例深拷贝：量化策略对象包含信号指标（MA、RSI）、持仓管理器、风控规则等复杂子对象，
 *   深拷贝确保克隆策略与原始策略完全独立，修改克隆不影响原始
 * - 投资组合深拷贝：Portfolio 包含多个 Position 和 Order 对象，深拷贝用于
 *   并行回测场景（每个线程持有独立副本）和情景分析
 * - 订单快照：实时订单包含动态状态（已成交/部分成交/已撤销），
 *   深拷贝保存订单状态快照用于审计和异常恢复
 *
 * 【UML 关键参与者】
 * - Prototype（原型）：Contact —— 提供拷贝构造函数实现深拷贝
 * - 原型模式的核心是 Clone 操作，在 C++ 中拷贝构造函数天然支持这一操作
 *
 * 【与 motivation.cpp 的对比】
 * - motivation.cpp 中的 ContactPtr 使用默认拷贝 → 浅拷贝 → 问题
 * - 本文件中的 Contact 实现拷贝构造函数 → 深拷贝 → 解决问题
 *
 * 【拷贝构造函数关键代码】
 * Contact(const Contact &other)
 *     : name{other.name}, address{new Address{*other.address}} {}
 *                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *                          关键：为 address 分配新的堆内存，并复制内容
 */

#include <iostream>
#include <memory>
#include <string>

using namespace std;

/*
 * Address: 地址
 *
 * 全部成员都是值类型（string, int），默认拷贝/移动即可正确工作。
 * 无需显式定义拷贝构造（Effective C++ 条款 11：优先利用编译器生成的
 * 特殊成员函数，而不是手写）。
 */
struct Address {
  string street, city;
  int suite;

  // 使用成员初始化列表 + 默认形参，减少拷贝
  Address(string street, string city, int suite)
      : street(std::move(street)), city(std::move(city)), suite(suite) {}

  friend ostream &operator<<(ostream &os, const Address &address) {
    return os << "street: " << address.street << ", city: " << address.city
              << ", suite: " << address.suite;
  }
};

/*
 * Contact: 联系人（带深拷贝的原型实现）
 *
 * 【Modern C++ 优化点】
 * 1. 用 unique_ptr<Address> 替代裸指针 Address*：
 *    - 自动管理内存，无需手写析构函数（RAII，Effective C++ 条款 13）
 *    - 杜绝拷贝构造里手动 new 导致的内存泄漏
 * 2. 深拷贝通过自定义拷贝构造 + 手动克隆完成（unique_ptr 不可拷贝，需要自己实现）
 * 3. 移动构造/移动赋值 = default：
 *    - unique_ptr 是 move-only 的，编译器自动生成正确的移动语义（Effective C++ 条款 17）
 * 4. 拷贝赋值用 copy-and-swap 惯用法，异常安全且简洁（Effective C++ 条款 12）
 */
struct Contact {
  string name;
  unique_ptr<Address> address;  // 拥有所有权的智能指针

  // 传入值语义的 name，配合 std::move 避免多余拷贝
  Contact(string name, unique_ptr<Address> address)
      : name(std::move(name)), address(std::move(address)) {}

  // 拷贝构造 —— 原型模式的核心：深拷贝（unique_ptr 不可拷贝，手动克隆）
  Contact(const Contact &other)
      : name(other.name), address(make_unique<Address>(*other.address)) {}

  // 移动构造/移动赋值 = default —— 转移所有权，零拷贝
  Contact(Contact &&) noexcept = default;
  Contact &operator=(Contact &&) noexcept = default;

  // 拷贝赋值 —— copy-and-swap 惯用法，异常安全
  Contact &operator=(const Contact &other) {
    if (this != &other) {          // 自赋值保护
      Contact tmp(other);          // 先做深拷贝
      swap(tmp, *this);            // 再交换，异常安全（Effective C++ 条款 11/12）
    }
    return *this;
  }

  friend ostream &operator<<(ostream &os, const Contact &contact) {
    return os << "name: " << contact.name << ", address: " << *contact.address;
  }
};

int main() {
  cout << endl << "Cloning class with unique_ptr: " << endl;

  // 创建原始对象 john —— 用 make_unique 分配（Effective C++ 条款 21）
  Contact john{"John Doe",
               make_unique<Address>("123 East Dr", "London", 100)};

  // 拷贝构造 jane —— 深拷贝，jane 拥有独立的 Address
  Contact jane = john;

  // 修改 jane 的名字
  jane.name = "Jane Smith";
  cout << john << endl;

  // 修改 jane 的门牌号 —— 由于是深拷贝，不会影响 john
  jane.address->suite = 99999;
  cout << jane << endl;
  cout << john << endl;  // john 的门牌号保持不变！

  // 移动构造示例 —— 转移所有权，零拷贝
  Contact moved = std::move(jane);
  cout << moved << endl;

  return 0;
}
