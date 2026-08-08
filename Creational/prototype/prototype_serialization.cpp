/*
 * ===========================================================================
 * 设计模式：Prototype with Serialization（序列化原型模式）
 * ===========================================================================
 *
 * 【核心思想】
 * 使用序列化（Serialize）和反序列化（Deserialize）来实现深拷贝，
 * 替代手动编写的拷贝构造函数。当对象结构非常复杂（多层嵌套、多态等）时，
 * 序列化方式比手动拷贝构造函数更容易维护。
 *
 * 【适用场景 —— 通用】
 * - 对象结构复杂，手动编写拷贝构造函数容易出错
 * - 对象层级很深，嵌套多层指针
 * - 希望用一种通用的方式处理所有类型的深拷贝
 * - 需要在不同进程/机器间传递对象副本（分布式系统）
 *
 * 【金融工程应用】
 * - 策略状态持久化：日内交易策略的状态对象（信号历史、持仓信息、当日盈亏）通过序列化
 *   保存到磁盘，系统重启后通过反序列化恢复状态继续运行，确保不丢失盘中状态
 * - 分布式回测状态传递：将回测子任务的状态序列化为 Protobuf/JSON 在集群节点间传递，
 *   子节点反序列化后继续计算，结果再序列化回传给主节点汇总
 * - 交易信号跨系统传递：信号对象序列化后通过消息队列传递到执行系统，
 *   执行系统反序列化后解析信号内容并生成订单
 *
 * 【UML 关键参与者】
 * - Prototype（原型）：Contact —— 提供 serialize/deserialize 方法
 * - 序列化机制替代了拷贝构造函数
 *
 * 【序列化方式 vs 拷贝构造方式】
 * - 拷贝构造：需要为每个类手动编写，但性能更好
 * - 序列化：通用性强，但需要序列化/反序列化开销
 * - 本例使用手动文本序列化（原版使用 Boost.Serialization）
 *
 * 【本例要点】
 * - Address 和 Contact 都实现了 serialize() 和 deserialize()
 * - EmployeeFactory::new_employee() 通过序列化实现深拷贝：
 *   序列化到 ostringstream -> 从 istringstream 反序列化
 * - main() 中的 lambda clone() 演示了通用的序列化拷贝流程
 */

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
using namespace std;

/*
 * Address: 地址类
 *
 * 实现了手动文本序列化方法，替代拷贝构造函数实现深拷贝。
 */
class Address {
public:
  string street, city;
  int suite;

  // 空构造函数用 = default（Effective C++ 条款 12）
  Address() = default;

  Address(string street, string city, int suite)
      : street(std::move(street)), city(std::move(city)), suite(suite) {}

  friend ostream &operator<<(ostream &os, const Address &address) {
    return os << "street: " << address.street << ", city: " << address.city
              << ", suite: " << address.suite;
  }

  /*
   * 序列化：将对象状态写入输出流
   *
   * 格式：每行一个字段
   *   street
   *   city
   *   suite
   */
  void serialize(ostream &os) const {
    os << street << '\n' << city << '\n' << suite << '\n';
  }

  /*
   * 反序列化：从输入流恢复对象状态
   *
   * 按序列化的顺序读取字段
   */
  void deserialize(istream &is) {
    getline(is, street);
    getline(is, city);
    is >> suite;
    is.ignore(); // 消耗 suite 后面的换行符
  }
};

/*
 * Contact: 联系人（序列化原型实现）
 *
 * 使用 serialize/deserialize 替代拷贝构造函数。
 * address 是指针成员，反序列化时重新分配内存。
 */
class Contact {
public:
  string name;
  // 【Modern C++ 优化】
  // 用 unique_ptr<Address> 替代裸指针，RAII 自动管理内存（Effective C++ 条款 13）。
  // 不再需要手写析构函数，避免了 Rule of Three 违反导致的 double-free / 内存泄漏
  // （Effective C++ 条款 5/6）。
  unique_ptr<Address> address;

  // 默认构造：address 用 make_unique 空初始化，或用 nullptr（成员默认初始化）
  Contact() = default;

  Contact(string name, unique_ptr<Address> address)
      : name(std::move(name)), address(std::move(address)) {}

  // 深拷贝构造（unique_ptr 不可拷贝，需手动克隆 Address）
  Contact(const Contact &other)
      : name(other.name),
        address(other.address ? make_unique<Address>(*other.address)
                              : nullptr) {}

  // 移动语义 = default（转移所有权）
  Contact(Contact &&) noexcept = default;
  Contact &operator=(Contact &&) noexcept = default;

  // 拷贝赋值 —— copy-and-swap，异常安全（Effective C++ 条款 12）
  Contact &operator=(const Contact &other) {
    if (this != &other) {
      Contact tmp(other);
      swap(tmp, *this);
    }
    return *this;
  }

  friend ostream &operator<<(ostream &os, const Contact &contact) {
    return os << "name: " << contact.name << ", address: " << *contact.address;
  }

  /*
   * 序列化：先写 name，再委托 address 序列化自身
   */
  void serialize(ostream &os) const {
    os << name << '\n';
    address->serialize(os);
  }

  /*
   * 反序列化：先读 name，再创建新的 Address 并反序列化
   *
   * 用 make_unique 分配（Effective C++ 条款 21），自动管理旧内存
   */
  void deserialize(istream &is) {
    getline(is, name);
    address = make_unique<Address>();  // 自动释放旧地址，分配新地址
    address->deserialize(is);
  }
};

/*
 * EmployeeFactory: 原型工厂（使用序列化进行深拷贝）
 *
 * 原型工厂从同一个原型员工创建新的员工对象。
 * new_employee() 使用序列化/反序列化实现深拷贝，
 * 而不是拷贝构造函数。
 */
class EmployeeFactory {
public:
  static unique_ptr<Contact> new_main_office_employee(const string &name,
                                                      const int suite) {
    static Contact p{"", make_unique<Address>("123 East Road", "London", 0)};
    return new_employee(name, suite, p);
  }

private:
  static unique_ptr<Contact> new_employee(const string &name, int suite,
                                          const Contact &prototype) {
    // 通过手动序列化实现深拷贝
    ostringstream oss;
    prototype.serialize(oss);   // 序列化原型
    string data = oss.str();

    istringstream iss(data);
    auto result = make_unique<Contact>();
    result->deserialize(iss);   // 反序列化创建副本
    result->name = name;
    result->address->suite = suite;
    return result;
  }
};

int main() {
  /*
   * 通用克隆 lambda：演示序列化深拷贝的通用模式
   *
   * 步骤：
   * 1. 序列化对象到 stringstream
   * 2. 从 stringstream 反序列化创建新对象
   * 3. 返回新对象（深拷贝）
   *
   * 这种方式的优点：对于任何实现了 serialize/deserialize 的对象都适用
   */
  auto clone = [](const Contact &c) {
    // 序列化
    ostringstream oss;
    c.serialize(oss);
    string s = oss.str();
    cout << s << endl;

    // 反序列化
    istringstream iss(s);
    Contact result;
    result.deserialize(iss);
    return result;
  };

  auto john = EmployeeFactory::new_main_office_employee("john", 123);
  auto jane = clone(*john);  // 通过序列化克隆 john
  jane.name = "Jane";
  cout << *john << endl;
  cout << jane << endl;

  return 0;
}
