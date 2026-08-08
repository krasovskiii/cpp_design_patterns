/*
 * =============================================================================
 * 设计模式：享元模式（Flyweight Pattern）—— 模板实现
 * =============================================================================
 *
 * 【一句话概括】
 * 通过共享相同状态来减少内存使用，将对象的内在状态（不变）和外在状态（可变）分离。
 *
 * 【适用场景 —— 通用】
 * - 需要创建大量相似对象，且这些对象的大部分状态可以共享时
 *
 * 【金融工程应用】
 * - 行情快照共享：同一时刻数千只股票的行情快照中，交易日期/市场状态等字段完全相同，
 *   享元模式让所有快照共享同一份日期和状态对象
 * - 期权合约参数驻留：大量期权合约共享标的/交易所/合约乘数等不变属性，
 *   只有行权价和到期日等可变属性独立存储
 * - 费率表共享：所有同品种订单共享同一份费率表引用，而非每个订单复制一份
 *
 * 【本示例说明】
 * Flyweight<T> 模板维护静态 unordered_map，相同值复用已有实例。
 * 多个 User 对象相同名字共享同一块内存。
 */

#include <iostream>
#include <string>
#include <unordered_map>
using namespace std;

// ============================================================================
// 自定义享元模板类（替代 boost::flyweight）
// 使用 std::unordered_map 实现字符串驻留，相同值共享同一个实例
// ============================================================================
template <typename T> class Flyweight {
public:
  Flyweight() {}

  // 构造函数：如果值已存在则复用，否则存入静态存储
  Flyweight(const T &value) {
    auto it = store_.find(value);
    if (it == store_.end()) {
      // 值不存在：插入到 map 中，ptr_ 指向 map 中存储的副本
      auto [inserted, _] = store_.emplace(value, value);
      ptr_ = &inserted->second;
    } else {
      // 值已存在：ptr_ 直接指向已有实例
      ptr_ = &it->second;
    }
  }

  // 获取底层值的常量引用
  const T &get() const { return *ptr_; }

  // 隐式类型转换：使 Flyweight<T> 可以当作 const T& 使用
  operator const T &() const { return *ptr_; }

private:
  // 静态存储：所有 Flyweight 实例共享同一个 map
  static unordered_map<T, T> store_;
  const T *ptr_ = nullptr;
};

// 静态成员定义
template <typename T> unordered_map<T, T> Flyweight<T>::store_;

// ============================================================================
// 使用享元模式的 User 类
// first_name 和 last_name 使用 Flyweight<string> 实现共享存储
// ============================================================================
struct User {
  Flyweight<string> first_name;
  Flyweight<string> last_name;

  User(const string &first_name, const string &last_name)
      : first_name{first_name}, last_name{last_name} {}
};

ostream &operator<<(ostream &os, const User &user) {
  os << "User: " << user.first_name.get() << " " << user.last_name.get();
  return os;
}

int main() {
  User user1{"John", "Doe"};
  User user2{"John", "Smith"};

  cout << user1 << endl;
  cout << user2 << endl;

  // 验证享元效果：比较引用地址，检查是否指向同一对象
  cout << boolalpha;
  // "John" 相同，应指向同一地址（true）
  cout << (&user1.first_name.get() == &user2.first_name.get()) << endl;
  // "Doe" 和 "Smith" 不同，地址不同（false）
  cout << (&user1.last_name.get() == &user2.last_name.get()) << endl;
  return 0;
}
