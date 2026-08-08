/*
 * ===========================================================================
 * 设计模式：Factory Exercise（工厂模式练习）
 * ===========================================================================
 *
 * 【核心思想】
 * 使用工厂模式为 Person 对象分配唯一的自增 ID。这是工厂模式在
 * 实际开发中的典型应用 —— 将对象创建与 ID 生成、资源分配等逻辑封装在一起。
 *
 * 【适用场景】
 * - 需要为创建的对象分配唯一标识符（ID）
 * - 对象创建涉及额外的初始化逻辑（如注册、计数、日志）
 * - 希望统一管理对象的创建流程
 *
 * 【金融工程应用】
 * - 订单 ID 生成：OrderFactory 创建订单时自动分配全局唯一的 order_id，
 *   确保在分布式交易系统中订单 ID 不冲突，便于追踪和风控审计
 * - 交易流水号生成：TradeFactory 为每笔成交自动生成递增的 trade_id，
 *   配合时间戳和系统标识组成全局唯一的交易流水号
 * - 策略实例 ID 分配：StrategyFactory 为每个策略实例分配唯一编号，
 *   方便在组合管理中追踪每个策略的贡献和风险
 *
 * 【UML 关键参与者】
 * - Product（产品）：Person —— 被创建的对象（id + name）
 * - Factory（工厂）：PersonFactory —— 封装 Person 的创建和 ID 分配
 *
 * 【本例要点】
 * - PersonFactory 使用 static 局部变量 idxs 维护自增 ID
 * - 每次调用 create_person() 都会分配一个新的唯一 ID
 * - static 局部变量保证了即使创建多个 PersonFactory 实例，ID 序列也是全局唯一的
 *   （注意：实际上不同 PersonFactory 实例共享同一个 static idxs）
 */

#include <iostream>
#include <string>
using namespace std;

/*
 * Person: Product（产品）角色
 *
 * 包含 id 和 name 两个属性。
 * 构造函数在创建时打印日志，便于观察工厂的调用过程。
 */
struct Person {
  int id;
  string name;

  Person(int id, string name) : id(id), name(name) {
    cout << "Constructed person #" << id << " - " << name << endl;
  }
};

/*
 * PersonFactory: Factory（工厂）角色
 *
 * 封装 Person 的创建逻辑，自动分配唯一 ID。
 *
 * 关键设计：使用 static 局部变量 idxs 维护自增 ID。
 * static 局部变量在第一次调用时初始化，之后保持其值。
 * 注意：由于 static 局部变量在函数内共享，即使创建多个
 * PersonFactory 实例，它们也会共享同一个 idxs。
 */
class PersonFactory {
public:
  Person create_person(const string &name) {
    static int idxs = 0;  // 静态局部变量，在多次调用间保持值
    Person p(idxs++, name);
    return p;
  }
};

int main() {
  PersonFactory pf;
  // 每次调用都会自动分配递增的 ID
  auto p0 = pf.create_person("A");  // id = 0
  auto p1 = pf.create_person("B");  // id = 1
  auto p2 = pf.create_person("C");  // id = 2
  return 0;
}
