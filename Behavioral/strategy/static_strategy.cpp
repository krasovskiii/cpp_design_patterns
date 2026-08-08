/*
 * =============================================================================
 * 设计模式：策略模式（Strategy）—— 静态策略（编译期多态）
 * =============================================================================
 *
 * 【一句话概括】
 * 使用 C++ 模板在编译期确定策略类型，消除虚函数调用的运行时开销。
 *
 * 【适用场景 —— 通用】
 * - 策略在编译期就已确定，不需要运行时切换
 * - 对性能敏感，希望避免虚函数调用的开销
 *
 * 【金融工程应用】
 * - 高频交易信号：编译期绑定策略类型，零虚函数开销，适合微秒级延迟敏感场景
 * - 编译期因子计算：因子类型（MA/RSI/MACD）在编译期确定，模板展开为内联代码，
 *   避免虚函数调用开销，提升回测和实时计算速度
 *
 * 【与动态策略的区别】
 * - 动态策略：使用 unique_ptr<ListStrategy>，运行时切换
 * - 静态策略：使用模板参数 LS，编译期确定，零运行时开销
 *
 * 【关键参与者】
 *   - Strategy（策略接口）：ListStrategy
 *   - ConcreteStrategy（具体策略）：MarkdownListStrategy、HtmlListStrategy
 *   - Context（上下文）：TextProcessor<LS>，模板类在编译期绑定策略类型
 */

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

enum class OutputFormat { Markdown, Html };

// 策略接口（Strategy）：纯虚函数接口，强制子类实现所有方法
struct ListStrategy {
  virtual ~ListStrategy() = default;
  virtual void add_list_item(ostringstream &oss, const string &item) = 0;
  virtual void start(ostringstream &oss) = 0;
  virtual void end(ostringstream &oss) = 0;
};

// 具体策略：Markdown 格式
struct MarkdownListStrategy : ListStrategy {
  // 注意：由于基类方法是纯虚函数，静态多态下我们被迫实现 start() 和 end()
  // 即使 Markdown 不需要这些标记

  void start(ostringstream &) override {} // 空实现：Markdown 不需要开始标记
  void end(ostringstream &) override {}   // 空实现：Markdown 不需要结束标记
  void add_list_item(ostringstream &oss, const string &item) override {
    oss << " * " << item << endl;
  }
};

// 具体策略：HTML 格式
struct HtmlListStrategy : ListStrategy {
  void start(ostringstream &oss) override { oss << "<ul>" << endl; }
  void end(ostringstream &oss) override { oss << "</ul>" << endl; }
  void add_list_item(ostringstream &oss, const string &item) override {
    oss << "<li>" << item << "</li>" << endl;
  }
};

// 上下文（Context）：模板允许在编译期定义策略类型
// 缺点：无法在运行时切换策略
template <typename LS> struct TextProcessor {
  void clear() {
    oss.str("");
    oss.clear();
  }

  // 追加列表：使用编译期确定的策略类型
  void append_list(const vector<string> &items) {
    list_strategy.start(oss);
    for (auto &item : items)
      list_strategy.add_list_item(oss, item);
    list_strategy.end(oss);
  }
  string str() const { return oss.str(); }

private:
  ostringstream oss;

  // 不再需要指针/引用——策略对象直接作为成员
  LS list_strategy;
};

// 客户端：演示静态策略模式
int main() {
  // Markdown 格式（编译期确定策略类型）
  TextProcessor<MarkdownListStrategy> tpm;
  tpm.append_list({"foo", "bar", "baz"});
  cout << tpm.str() << endl;

  // HTML 格式（编译期确定策略类型）
  TextProcessor<HtmlListStrategy> tph;
  tph.append_list({"foo", "bar", "baz"});
  cout << tph.str() << endl;

  return 0;
}
