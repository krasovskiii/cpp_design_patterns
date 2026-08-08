/*
 * ===========================================================================
 * 设计模式：Multiton（多例模式）
 * ===========================================================================
 *
 * 【核心思想】
 * Multiton 是单例模式的泛化：不是全局只有一个实例，而是每个"键"（key）
 * 对应一个实例。类似于一个受控的实例注册表，确保相同 key 返回相同的实例。
 *
 * 【适用场景 —— 通用】
 * - 需要按类别管理的单例（如按打印机重要性级别管理不同的打印机实例）
 * - 对象池管理 —— 每种类型只需要一个代表实例
 * - 需要有限数量的、可区分的全局实例
 *
 * 【金融工程应用】
 * - 交易所连接管理：每个交易所（SHFE/DCE/CZCE/CFFEX/SSE/SZSE）需要独立的连接实例，
 *   ExchangeManager::get("SHFE") 始终返回上期所连接，get("DCE") 返回大商所连接
 *   既保证了每个交易所只有一个连接（避免重复连接），又区分了不同交易所
 * - 合约管理器：每个合约代码（IF2406/IC2406/IH2406）对应唯一的 Contract 实例，
 *   避免同一合约在内存中存在多个副本导致数据不一致
 * - 策略实例注册表：多策略组合中，每个策略名对应唯一的策略实例，
 *   StrategyRegistry::get("MA_Trend") 始终返回同一策略对象
 * - 风控规则注册表：按风控类型（持仓限制/止损/保证金/熔断）管理唯一规则实例
 *   RiskRegistry::get("PositionLimit") 确保全局使用同一持仓限制规则
 *
 * 【Singleton vs Multiton】
 * - Singleton：一个键 → 一个实例（全局唯一）
 * - Multiton：N 个键 → N 个实例（每个键唯一）
 * - Multiton 本质上是一个 map<Key, Instance> 的包装
 *
 * 【UML 关键参与者】
 * - Multiton（多例管理器）：Multiton<T, Key> —— 模板类，管理 key → instance 映射
 * - Instance（实例类）：Printer —— 具体的业务类
 * - Key（键类型）：Importance 枚举 —— 区分不同实例的键
 *
 * 【本例要点】
 * - Multiton 是模板类，泛化了"按 key 管理实例"的逻辑
 * - get() 方法：如果 key 已存在，返回已有实例；否则创建新实例
 * - 使用 shared_ptr 管理实例生命周期
 * - Printer 演示了按重要性级别（primary/secondary/tertiary）区分的多例
 */

#include <iostream>
#include <map>
#include <memory>

using namespace std;

// 重要性级别枚举，作为 Multiton 的键类型
enum class Importance { primary, secondary, tertiary };

/*
 * Multiton: 多例管理器模板类
 *
 * Multiton 类本身不应当被实例化，只用于生成和管理各 key 对应的单例。
 *
 * 模板参数：
 * - T:   实例类型
 * - Key: 键类型（默认为 string）
 *
 * 核心机制：
 * - 内部维护一个 static map<Key, shared_ptr<T>>
 * - get(key) 查找或创建对应 key 的实例
 * - 构造函数为 protected，防止直接实例化 Multiton 本身
 */
template <typename T, typename Key = std::string> class Multiton {
public:
  /*
   * get(): 获取指定 key 对应的实例
   *
   * 逻辑：
   * 1. 在 instances map 中查找 key
   * 2. 如果找到 → 返回已有实例
   * 3. 如果未找到 → 创建新实例，存入 map，然后返回
   *
   * 使用 shared_ptr 管理生命周期，实例在没有任何引用时自动销毁。
   */
  static shared_ptr<T> get(const Key &key) {
    if (const auto it = instances.find(key); it != instances.end()) {
      return it->second;  // 返回已有实例
    }

    // 创建新实例并注册
    auto instance = make_shared<T>();
    instances[key] = instance;
    return instance;
  }

protected:
  Multiton() = default;
  virtual ~Multiton() = default;

private:
  // 实例注册表：key → 对应的单例实例
  static map<Key, shared_ptr<T>> instances;
};

// 静态成员变量定义（模板类需要显式定义）
template <typename T, typename Key>
map<Key, shared_ptr<T>> Multiton<T, Key>::instances;

/*
 * Printer: 打印机类（多例的实例类型）
 *
 * 我们希望为不同的重要性级别创建不同的打印机单例。
 * Printer 本身不继承 Multiton —— Multiton 是外部管理器。
 *
 * static totalInstanceCount 用于验证：相同 key 只创建一次 Printer。
 */
class Printer {
public:
  Printer() {
    ++Printer::totalInstanceCount;
    cout << "A total of " << Printer::totalInstanceCount
         << " instances created so far." << endl;
  }

private:
  static int totalInstanceCount;
};
int Printer::totalInstanceCount = 0;

int main() {
  // 类型别名：Multiton<Printer, Importance>
  // 用 using 而非 typedef（C++11 推荐语法，可读性更好，也支持模板别名）
  using mt = Multiton<Printer, Importance>;

  // primary 键 → 创建第一个 Printer 实例
  auto main = mt::get(Importance::primary);

  // secondary 键 → 创建第二个 Printer 实例
  auto sec1 = mt::get(Importance::secondary);

  // secondary 键再次请求 → 返回已有的实例（不创建新的）
  auto sec2 = mt::get(Importance::secondary);

  // 输出应该显示只创建了 2 个实例（primary 和 secondary 各一个）
  return 0;
}
