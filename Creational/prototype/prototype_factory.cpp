/*
 * ===========================================================================
 * 设计模式：Prototype Factory（原型工厂模式）
 * ===========================================================================
 *
 * 【核心思想】
 * 将原型对象与工厂模式结合：工厂持有一个或多个预配置的原型对象，
 * 通过克隆原型来创建新对象。这样既能利用原型的"复制"能力，
 * 又能利用工厂的"集中管理"能力。
 *
 * 【适用场景 —— 通用】
 * - 需要创建多个相似但略有不同的对象（如同一公司的不同员工）
 * - 原型对象需要预配置（默认值设置）
 * - 希望将"如何创建对象"的知识集中管理
 *
 * 【金融工程应用】
 * - 策略模板库：StrategyPrototypeFactory 维护多种预配置策略原型（MACD 策略、均线策略、
 *   网格策略），用户从模板克隆后微调参数，避免从零配置
 * - 订单模板：OrderPrototypeFactory 管理常用订单模板（限价单模板、市价单模板、止损单模板），
 *   每种模板预填了常用参数，交易员克隆后修改 symbol/qty/price 即可
 * - 回测配置模板：BacktestConfigFactory 持有不同场景的配置原型（日内回测、日频回测、高频回测），
 *   克隆模板后微调时间范围或资金规模
 * - 风险报表模板：RiskReportFactory 持有多种报表模板（日报、周报、月报），
 *   原型预定义了报表结构，克隆后填入当天的数据
 *
 * 【UML 关键参与者】
 * - Prototype（原型）：Contact —— 提供拷贝构造实现深拷贝
 * - PrototypeFactory（原型工厂）：EmployeeFactory —— 持有原型并克隆
 * - 工厂使用 static 局部变量持有原型，确保原型只初始化一次
 *
 * 【本例要点】
 * - EmployeeFactory::new_main_office_employee() 使用 static 局部变量 p 作为原型
 * - new_employee() 私有方法执行实际的克隆和定制
 * - 原型 p 的默认值（地址、城市）被复用，只有 name 和 suite 被替换
 */

#include <iostream>
#include <memory>
#include <string>
#include <utility>

using namespace std;

/*
 * Address: 地址类
 *
 * 全部成员都是值类型，默认拷贝/移动即可正确工作。
 * 无需手写拷贝构造（Effective C++ 条款 11）。
 */
class Address {
public:
  string street, city;
  int suite;

  Address(string street, string city, int suite)
      : street(std::move(street)), city(std::move(city)), suite(suite) {}

  friend ostream &operator<<(ostream &os, const Address &address) {
    return os << "street: " << address.street << ", city: " << address.city
              << ", suite: " << address.suite;
  }
};

/*
 * Contact: 联系人（带深拷贝）
 *
 * 【Modern C++ 优化点】
 * - 用 unique_ptr<Address> 替代裸指针，RAII 自动管理内存（Effective C++ 条款 13）
 * - 深拷贝通过自定义拷贝构造实现（unique_ptr 不可拷贝）
 * - 移动语义 = default，转移所有权零拷贝
 */
class Contact {
public:
  string name;
  unique_ptr<Address> address;

  Contact(string name, unique_ptr<Address> address)
      : name(std::move(name)), address(std::move(address)) {}

  // 拷贝构造 —— 深拷贝（原型模式核心）
  Contact(const Contact &other)
      : name(other.name), address(make_unique<Address>(*other.address)) {}

  // 移动语义 —— 转移所有权
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
};

/*
 * EmployeeFactory: 原型工厂
 *
 * 使用预配置的原型对象来创建新员工。
 * 原型工厂从同一个原型员工创建新的员工对象。
 *
 * 设计要点：
 * - static 局部变量 p 确保原型只初始化一次
 * - 原型 p 包含默认的地址信息（"123 East Road", "London"）
 * - new_employee() 先克隆原型，再定制 name 和 suite
 */
class EmployeeFactory {
public:
  /*
   * 创建总部办公室员工
   *
   * 使用 static 局部变量 p 作为原型：
   * - 地址固定为 "123 East Road", "London"
   * - name 和 suite 由调用者指定
   *
   * static 确保原型只创建一次，后续调用复用同一个原型
   */
  static unique_ptr<Contact> new_main_office_employee(const string &name,
                                                      const int suite) {
    // make_unique 分配（Effective C++ 条款 21）
    static Contact p{"", make_unique<Address>("123 East Road", "London", 0)};
    return new_employee(name, suite, p);
  }

private:
  /*
   * 通用的员工创建方法（私有）
   *
   * 工作流程：
   * 1. 拷贝原型对象（深拷贝，包括 Address）
   * 2. 修改新对象的 name
   * 3. 修改新对象的 address->suite
   * 4. 返回 unique_ptr，确保所有权转移给调用者
   */
  static unique_ptr<Contact> new_employee(const string &name, int suite,
                                          const Contact &prototype) {
    // 通过拷贝构造函数深拷贝原型
    auto result = make_unique<Contact>(prototype);
    result->name = name;
    result->address->suite = suite;
    return result;
  }
};

int main() {
  // 从原型工厂创建员工，只需指定姓名和门牌号
  auto john = EmployeeFactory::new_main_office_employee("john", 100);
  cout << *john << endl;

  return 0;
}
