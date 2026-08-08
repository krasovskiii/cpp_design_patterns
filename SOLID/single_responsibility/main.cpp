/*
 * =============================================================================
 * SOLID 设计原则 - 单一职责原则 (Single Responsibility Principle, SRP)
 * =============================================================================
 *
 * 【一句话概括】
 * 一个类应该只有一个引起它变化的原因。即：每个类只负责一件事。
 *
 * 【适用场景 —— 通用】
 * - 当一个类承担了过多不相关的职责时，应该将其拆分为多个专注于单一职责的类
 * - 当修改某个功能时，不希望影响到其他无关的功能
 * - 希望代码更容易测试、维护和复用
 * - Web 应用：Controller 只处理请求路由，Service 只处理业务逻辑，Repository 只处理数据访问
 * - 数据处理管道：Reader（读取）→ Parser（解析）→ Transformer（转换）→ Writer（写入），每个环节独立
 * - 文档处理：DocumentLoader（加载）→ DocumentRenderer（渲染）→ DocumentSaver（保存），各司其职
 * - 日志系统：Logger（记录日志）、LogFormatter（格式化）、LogRotator（日志轮转）各自独立
 *
 * 【金融工程应用】
 * - 策略类职责分离：Strategy 只负责生成信号（何时买卖），PositionManager 只负责仓位计算，
 *   RiskManager 只负责风险检查，OrderExecutor 只负责下单执行
 *   修改风控规则时，策略逻辑和下单逻辑完全不受影响
 * - 回测引擎拆分：DataLoader（数据加载）、Simulator（回测模拟）、Analyzer（绩效分析）、
 *   ReportGenerator（报表生成）四个独立类
 *   修改绩效指标计算方式（如从夏普比率改为索提诺比率），不影响回测模拟核心逻辑
 * - 行情数据处理：QuoteReceiver（行情接收）、QuoteNormalizer（标准化）、QuoteValidator（校验）、
 *   QuoteDistributor（分发）各司其职
 *   新增交易所数据源只需修改 Receiver，校验逻辑和分发逻辑完全不受影响
 * - 订单管理系统（OMS）：OrderValidator（验证）、OrderRouter（路由）、OrderStateMachine（状态机）、
 *   OrderPersister（持久化）分离
 *   修改订单状态转换逻辑（如增加部分成交状态），不影响路由和持久化
 * - 风险报表系统：VaRCalculator（VaR 计算）、StressTester（压力测试）、SensitivityAnalyzer（敏感性分析）、
 *   ReportFormatter（报表格式化）
 *   新增 CVaR 计算方法只需修改 VaRCalculator，压力测试模块保持独立
 * - 实时风控引擎：PreTradeRiskCheck（事前风控）、PositionLimitCheck（持仓限制）、
 *   CircuitBreaker（熔断器）、AlertNotifier（告警通知）
 *   新增熔断规则不影响持仓检查和告警逻辑
 *
 * 【反例 / 不遵守的后果】
 * - 修改持久化逻辑时可能破坏业务逻辑（反之亦然）
 * - 类变得庞大且难以理解
 * - 单元测试困难，因为需要 mock 太多不相关的依赖
 * - 量化系统中：Strategy 类同时包含信号生成、仓位计算、风控检查、订单执行，5000 行代码难以维护
 *   修改风控参数时不小心影响了信号逻辑，导致实盘亏损
 *
 * 【本例说明】
 * Journal 类只负责日记条目的管理（添加条目），
 * PersistenceManager 类只负责数据的持久化（保存到文件）。
 * 这样，修改保存逻辑或日记管理逻辑时，互不影响。
 */

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// ---------------------------------------------------------------------------
// Journal 类：只负责管理日记条目（单一职责 - 领域逻辑）
// 它不需要知道如何保存到文件、数据库或网络
// ---------------------------------------------------------------------------
struct Journal {
  string title;                // 日记标题
  vector<string> entries;      // 日记条目列表

  Journal(const string &title) : title(title){};

  // 添加一条日记，自动生成递增的序号
  void add_entry(const string &entry) {
    static int count = 1;    // static 变量：在整个程序生命周期中持续递增
    entries.push_back(to_string(count++) + ": " + entry);
  }
};

// ---------------------------------------------------------------------------
// PersistenceManager 类：只负责数据持久化（单一职责 - 基础设施逻辑）
// 它不需要关心 Journal 内部的业务逻辑
// 如果将来需要保存到数据库或网络，只需修改这个类
// ---------------------------------------------------------------------------
struct PersistenceManager {
  // 将 Journal 的内容保存到文件
  static void save(const Journal &j, const string &filename) {
    ofstream ofs(filename);
    for (auto &e : j.entries) {
      ofs << e << endl;
    }
  }
};

// ---------------------------------------------------------------------------
// 主函数：展示关注点分离带来的好处
// ---------------------------------------------------------------------------
int main() {
  // 创建日记并添加条目 —— 业务逻辑
  Journal journal{"Dear Diary"};
  journal.add_entry("I ate a bug.");
  journal.add_entry("I also ate a spider.");

  // 保存日记到文件 —— 持久化逻辑（完全独立于 Journal）
  PersistenceManager pm;
  pm.save(journal, "diary.txt");

  return 0;
}
