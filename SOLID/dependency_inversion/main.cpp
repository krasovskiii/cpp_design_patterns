/*
 * =============================================================================
 * SOLID 设计原则 - 依赖倒置原则 (Dependency Inversion Principle, DIP)
 * =============================================================================
 *
 * 【一句话概括】
 * 高层模块不应该依赖于低层模块，两者都应该依赖于抽象。
 * 抽象不应该依赖于具体实现，具体实现应该依赖于抽象。
 *
 * 【适用场景 —— 通用】
 * - 当高层业务逻辑直接依赖于低层数据存储/访问实现时
 * - 当修改底层实现会导致高层代码也需要修改时
 * - 想要方便地替换底层组件（如切换数据库、mock 测试）
 * - 插件式架构：主程序依赖插件接口，插件实现接口，运行时动态加载
 * - 跨平台开发：业务层依赖平台抽象接口（IFileSystem），不同平台（Windows/Linux/macOS）提供具体实现
 * - 消息中间件集成：业务层依赖 IMessageQueue 接口，底层可切换 RabbitMQ/Kafka/Redis Pub-Sub
 * - 支付网关集成：订单系统依赖 IPaymentGateway，可灵活接入支付宝/微信/Stripe/PayPal
 *
 * 【金融工程应用】
 * - 行情数据源切换：策略引擎依赖 IMarketDataFeed 接口，底层可从 Bloomberg/Reuters/Wind/交易所直连
 *   切换数据源时，策略代码完全不受影响，只需替换 IMarketDataFeed 的实现
 * - 交易执行通道：算法交易系统依赖 IExecutionBroker 接口，具体实现可以是 CTP/FIX协议/模拟撮合器
 *   回测时注入模拟撮合器（Mock），实盘时注入真实券商接口，策略逻辑无需任何改动
 * - 定价模型替换：风险管理系统依赖 IPricingModel 接口，可灵活切换 Black-Scholes / Heston / Monte Carlo
 *   新增定价模型只需实现接口，不会影响风险报表、VaR 计算等高层模块
 * - 风控规则引擎：风控模块依赖 IRiskRule 接口，不同品种（股票/期货/期权）有不同规则实现
 *   新增风控规则（如科创板涨跌幅限制、熔断规则）无需修改风控框架
 * - 数据库中间层：量化平台依赖 IDataRepository 接口，底层可以是 MySQL/TickData 文件/内存缓存
 *   回测使用内存缓存提升速度，实盘切换为持久化存储，上层分析逻辑不变
 *
 * 【反例 / 不遵守的后果】
 * - 高层模块与低层实现紧耦合，修改底层可能破坏业务逻辑
 * - 无法对高层模块进行独立的单元测试
 * - 更换底层实现（如换数据库）需要大量修改业务代码
 * - 量化系统中：策略代码直接依赖 CTP API，切换券商或做回测时需要重写策略逻辑
 *
 * 【本例说明】
 * 坏方案：BadResearch（高层）直接访问 BadRelationships（低层）的内部数据结构
 * 好方案：引入 RelationshipBrowser 抽象接口，Research 只依赖此接口
 */

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

// 关系类型枚举
enum class Relationship { parent, child, sibling };

struct Person {
  string name;
};

// ===========================================================================
// 方案一：不好的实现 —— 违反依赖倒置原则
// ===========================================================================
// 低层模块：负责数据存储
struct BadRelationships {
  vector<tuple<Person, Relationship, Person>> relations;  // 公开的内部数据

  void add_parent_and_child(const Person &parent, const Person &child) {
    // emplace_back 直接在容器内构造，避免 tuple 的重复拷贝（Effective C++ 条款 21）
    relations.emplace_back(parent, Relationship::parent, child);
    relations.emplace_back(child, Relationship::child, parent);
  }
};

// 高层模块：依赖低层模块的具体实现和内部数据结构
// 问题1：Research 直接访问 Relations 的内部成员 relations
// 问题2：如果 relations 的数据结构变了（如换成 map），Research 也要改
// 问题3：无法对 Research 进行独立测试（必须依赖真实的 Relationships 数据）
struct BadResearch {
  BadResearch(BadRelationships &relationships) {
    auto &relations = relationships.relations;   // 直接访问低层模块的内部数据！
    for (auto &&[first, rel, second] : relations)
      if (first.name == "John" && rel == Relationship::parent)
        cout << "John has a child called " << second.name << endl;
  }
};

// ===========================================================================
// 方案二：好的实现 —— 遵守依赖倒置原则
// ===========================================================================
// 关键改变：引入抽象接口，高层模块只依赖接口

// 抽象接口：定义"查找关系"的能力（高层模块只需要知道这个）
struct RelationshipBrowser {
  // 多态基类需要虚析构（Effective C++ 条款 7）
  virtual ~RelationshipBrowser() = default;
  virtual vector<Person> find_all_children_of(const string &name) = 0;
};

// 低层模块：实现抽象接口
// 注意：relations 仍然是私有的，外界只能通过接口方法访问
struct Relationships : RelationshipBrowser {
  vector<tuple<Person, Relationship, Person>> relations;  // 低层的具体实现

  void add_parent_and_child(const Person &parent, const Person &child) {
    relations.emplace_back(parent, Relationship::parent, child);
    relations.emplace_back(child, Relationship::child, parent);
  }

  // 实现接口方法：将数据查找逻辑封装在低层模块内部
  vector<Person> find_all_children_of(const string &name) override {
    vector<Person> result;
    for (auto &&[first, rel, second] : relations)
      if (first.name == name && rel == Relationship::parent)
        result.emplace_back(second);
    return result;
  }
};

// 高层模块：只依赖抽象接口 RelationshipBrowser
// 不再关心数据如何存储（vector/map/list），只关心"能查找孩子"
struct Research {
  Research(RelationshipBrowser &browser) {   // 依赖抽象，而非具体实现！
    for (auto &child : browser.find_all_children_of("John"))
      cout << "John has a child called " << child.name << endl;
  }
};

// ---------------------------------------------------------------------------
// 主函数
// ---------------------------------------------------------------------------
int main() {
  Person parent{"John"};
  Person child1{"Chris"}, child2{"Matt"};

  // 使用新的设计：高层模块通过抽象接口与低层模块交互
  Relationships r;
  r.add_parent_and_child(parent, child1);
  r.add_parent_and_child(parent, child2);

  // Research 只需要知道 RelationshipBrowser 接口，不关心内部实现
  Research research(r);
  return 0;
}
