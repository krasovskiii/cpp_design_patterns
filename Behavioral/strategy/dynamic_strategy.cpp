/*
 * =============================================================================
 * 设计模式：策略模式（Strategy）—— 动态策略
 * =============================================================================
 *
 * 【一句话概括】
 * 定义一系列算法，将每个算法封装起来并使其可以相互替换，使算法独立于客户端变化。
 *
 * 【适用场景 —— 通用】
 * - 需要在运行时根据不同情况选择不同算法
 * - 许多相关类仅在行为上有所区别
 * - 需要避免暴露复杂、与算法相关的数据结构
 *
 * 【金融工程应用】
 * - 交易信号策略切换：SignalStrategy 接口下有 TrendFollowingStrategy、MeanReversionStrategy、
 *   BreakoutStrategy 等具体策略，运行时根据市场状态动态切换策略
 * - 撮合模型切换：MatchingStrategy 接口下有 SimpleMatch（对手价撮合）、OrderBookMatch（订单簿撮合）、
 *   TWAPMatch（TWAP撮合），回测和实盘使用不同策略
 * - 滑点模型切换：SlippageStrategy 接口下有 FixedSlippage、ProportionalSlippage、
 *   VolatilityAdjustedSlippage，根据品种和流动性动态选择
 * - 绩效指标计算：PerformanceMetric 接口下有 SharpeRatio、SortinoRatio、CalmarRatio、
 *   InformationRatio，用户可按需选择不同的绩效评估策略
 * - 手续费计算：CommissionStrategy 接口下有 FixedCommission、TieredCommission、
 *   ExchangeStandardCommission，不同交易所/品种自动切换费率策略
 *
 * 【关键参与者】
 *   - Strategy（策略接口）：ListStrategy，定义文本列表处理接口
 *   - ConcreteStrategy（具体策略）：MarkdownListStrategy、HtmlListStrategy
 *   - Context（上下文）：TextProcessor，持有策略引用并使用它
 *
 * HTML 输出:
 * <ul>
 *  <li> Foo
 *  <li> Bar
 * <ul>
 *
 * Markdown 输出:
 * * Foo
 * * Bar
 */

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// 输出格式枚举
enum class OutputFormat { Markdown, Html };

// 策略接口（Strategy）：定义列表处理的标准接口
struct ListStrategy {
  virtual ~ListStrategy() = default;

  // 公共行为接口
  virtual void add_list_item(ostringstream &, const string &){};
  virtual void start(ostringstream &){}; // 列表开始标记
  virtual void end(ostringstream &){};   // 列表结束标记
};

// 具体策略：Markdown 格式列表
struct MarkdownListStrategy : ListStrategy {
  // Markdown 列表项：以 " * " 开头
  void add_list_item(ostringstream &oss, const string &item) override {
    oss << " * " << item << endl;
  }

  // 注意：动态多态允许我们不实现 start() 和 end() 方法
  // 因为基类提供了默认空实现（Markdown 不需要列表的起止标记）
};

// 具体策略：HTML 格式列表
struct HtmlListStrategy : ListStrategy {
  // HTML 列表开始标记：<ul>
  void start(ostringstream &oss) override { oss << "<ul>" << endl; }
  // HTML 列表结束标记：</ul>
  void end(ostringstream &oss) override { oss << "</ul>" << endl; }
  // HTML 列表项：<li>item</li>
  void add_list_item(ostringstream &oss, const string &item) override {
    oss << "<li>" << item << "</li>" << endl;
  }
};

// 上下文（Context）：文本处理器，使用策略对象来格式化输出
struct TextProcessor {
  // 清空输出缓冲区
  void clear() {
    oss.str("");
    oss.clear();
  }

  // 追加列表：使用当前策略处理列表项
  void append_list(const vector<string> &items) {
    // 客户端调用通用接口，不依赖具体实现
    list_strategy->start(oss);
    for (auto &item : items)
      list_strategy->add_list_item(oss, item);
    list_strategy->end(oss);
  }

  // 动态切换输出格式（运行时选择策略）
  void set_output_format(const OutputFormat format) {
    switch (format) {
    case OutputFormat::Markdown:
      list_strategy = make_unique<MarkdownListStrategy>(); // 切换到 Markdown 策略
      break;
    case OutputFormat::Html:
      list_strategy = make_unique<HtmlListStrategy>();     // 切换到 HTML 策略
      break;
    default:
      throw runtime_error("Unsupported strategy.");
    }
  }

  string str() const { return oss.str(); }

private:
  ostringstream oss; // 输出缓冲区

  // 使用指针/引用以实现动态多态（运行时绑定）
  unique_ptr<ListStrategy> list_strategy;
};

// 客户端：演示动态策略模式
int main() {
  // Markdown 格式
  TextProcessor tp;
  tp.set_output_format(OutputFormat::Markdown); // 选择 Markdown 策略
  tp.append_list({"foo", "bar", "baz"});        // 使用 Markdown 格式输出
  cout << tp.str() << endl;

  // HTML 格式
  tp.clear();
  tp.set_output_format(OutputFormat::Html);     // 运行时切换到 HTML 策略
  tp.append_list({"foo", "bar", "baz"});        // 使用 HTML 格式输出
  cout << tp.str() << endl;

  return 0;
}
