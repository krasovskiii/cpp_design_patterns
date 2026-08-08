/*
 * =============================================================================
 * 设计模式：享元模式（Flyweight Pattern）—— 手动实现
 * =============================================================================
 *
 * 【一句话概括】
 * 使用数值索引替代字符串存储，通过双向映射实现字符串共享，节省内存。
 *
 * 【适用场景 —— 通用】
 * - 需要大量存储重复字符串且内存敏感时，用小型整数键替代完整字符串存储
 *
 * 【金融工程应用】
 * - 合约代码驻留：沪深两市 5000+ 只股票代码（如 "000001.SZ"），系统中可能存储数十万次，
 *   享元模式将合约代码驻留为整数索引，每份持仓/订单/行情引用只存 2 字节索引而非 10+ 字节字符串
 * - 交易账户驻留：大量订单关联的账户 ID 使用享元索引，减少内存占用
 * - 策略名称驻留：回测系统中大量策略实例引用相同的策略名，享元模式大幅减少内存
 * - 交易所名称驻留：SHFE/DCE/CZCE/CFFEX 等交易所名称反复出现，驻留为枚举值
 *
 * 【本示例说明】
 * User 类使用 uint16_t 键值存储 first_name/last_name，字符串集中存储在静态 map 中。
 * 每个 User 对象仅 4 字节（两个 uint16_t），而非两个 std::string。
 */

#include <iostream>
#include <string>
#include <unordered_map>
using namespace std;

// 使用数值键（key）替代完整字符串存储
// uint16_t 最多支持 65535 个唯一字符串
using key = uint16_t;  // 用 using 而非 typedef（C++11 推荐）

struct User {
private:
  // 双向映射表（替代 boost::bimap）
  static unordered_map<key, string> forward_;  // key -> 字符串
  static unordered_map<string, key> reverse_;  // 字符串 -> key
  static key seed;                              // 自增 ID 生成器

public:
  // User 对象只存储两个 key，而非完整的 string
  key first_name, last_name;

  // 构造函数：将字符串转换为 key 存储
  User(const string &first_name, const string &last_name)
      : first_name{add(first_name)}, last_name{add(last_name)} {}

  // 通过 key 查找原始字符串
  const string &get_first_name() const {
    return forward_.find(first_name)->second;
  }

  const string &get_last_name() const {
    return forward_.find(last_name)->second;
  }

private:
  // 字符串驻留（String Interning）的核心方法
  // 如果字符串已存在，返回已有 key；否则分配新 key
  static key add(const string &s) {
    auto it = reverse_.find(s);
    if (it == reverse_.end()) {
      // 新字符串：分配新 key 并建立双向映射
      key id = ++seed;
      forward_[id] = s;
      reverse_[s] = id;
      return id;
    }
    // 已存在的字符串：直接返回已有 key
    return it->second;
  }
};

// 静态成员初始化
key User::seed{0};
unordered_map<key, string> User::forward_{};
unordered_map<string, key> User::reverse_{};

ostream &operator<<(ostream &os, const User &user) {
  os << "User: " << user.get_first_name() << " " << user.get_last_name() << " ("
     << user.first_name << ", " << user.last_name << ")";
  return os;
}

int main() {
  User user1{"John", "Doe"};
  User user2{"John", "Smith"};

  // 可以看到 user1 和 user2 的 first_name key 相同（都是 "John" 对应的 key）
  cout << user1 << endl;
  cout << user2 << endl;
  return 0;
}
