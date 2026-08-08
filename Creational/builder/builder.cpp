/*
 * =============================================================================
 * 设计模式：建造者模式 (Builder Pattern)
 * =============================================================================
 *
 * 【一句话概括】
 * 将复杂对象的构建过程与它的表示分离，使得同样的构建过程可以创建不同的表示。
 *
 * 【适用场景 —— 通用】
 * - 创建复杂对象，其构建过程需要多个步骤
 * - 希望避免"重叠构造函数"（telescoping constructor）反模式
 * - 生成 HTML/XML 等结构化文档
 * - 对象的构建步骤需要灵活组合
 * - 构建配置对象：如 HTTP 请求配置、数据库连接字符串、服务器配置
 * - UI 组件构建：复杂的对话框/表单需要逐步添加控件、设置样式、绑定事件
 * - 协议消息构建：如 FIX 协议消息、gRPC 请求等需要逐步填充多个字段
 *
 * 【金融工程应用】
 * - 订单构建：复杂的算法订单（AlgoOrder）包含品种、方向、数量、限价、止损、有效期、冰山参数等
 *   使用 Builder 逐步设置，避免 15 个参数的构造函数，订单验证在 build() 中统一执行
 *   AlgoOrder::Builder().symbol("IF2406").qty(10).limit(3500.0).iceberg(5).ttl(300).build()
 * - 回测配置构建：回测参数包含时间范围、初始资金、手续费模型、滑点模型、基准指数等
 *   BacktestConfig::Builder().from("2023-01-01").to("2024-01-01").capital(1e7)
 *     .commission(FixedCommission(5.0)).slippage(ProportionalSlippage(0.001)).build()
 * - 金融产品定义：结构化产品（如雪球期权）包含敲入/敲出价格、票息率、期限、观察日等复杂参数
 *   SnowballBuilder 确保所有必填参数完整、参数一致性验证（敲入价 < 敲出价），避免无效产品被创建
 * - FIX 协议消息构建：FIX 消息有数十个 tag-value 对，按场景（新订单/撤单/修改）分步填充
 *   FixMessage::Builder().type("D").sender("CLIENT1").target("BROKER").tag(44, 100.5).build()
 * - 风险报表构建：风险报表包含 VaR/CVaR/希腊字母/压力测试等多模块，按需添加计算模块
 *   RiskReport::Builder().addVar(0.99).addGreeks().addStressTest("2008").addStressTest("2020").build()
 *
 * 【关键参与者】
 * - Builder：定义创建产品各个部件的抽象接口
 * - ConcreteBuilder：实现 Builder 接口，构造并装配产品部件
 * - Product：被构建的复杂对象
 * - Director（可选）：使用 Builder 来构建最终产品
 *
 * 【本例说明】
 * 用最简单的建造者模式构建一个 HTML 列表：
 * HtmlBuilder 负责逐步构建 HtmlElement（含 <ul> 和 <li> 标签）。
 * 对比 naive_html_builder（直接拼接字符串）展示了建造者模式的优势。
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// ---------------------------------------------------------------------------
// 朴素的方式：直接拼接 HTML 字符串
// 问题：容易出错，嵌套复杂时很难维护，没有结构化表示
// ---------------------------------------------------------------------------
void naive_html_builder() {
  // 单个段落：勉强可读
  auto text = "hello";
  string output;
  output += "<p>";
  output += text;
  output += "</p>";
  cout << output << endl;

  // 列表：嵌套时已显得混乱
  string words[] = {"hello", "world"};
  ostringstream oss;
  oss << "<ul>" << endl;
  for (auto w : words)
    oss << "  <li>" << w << "</li>" << endl;
  oss << "</ul>" << endl;
  cout << oss.str() << endl;
}

// ---------------------------------------------------------------------------
// Product：HTML 元素 —— 我们要构建的复杂对象
// ---------------------------------------------------------------------------
struct HtmlElement {
  string name, text;                     // 标签名和文本内容
  vector<HtmlElement> elements;          // 子元素（支持嵌套）
  const size_t indent_size = 2;          // 缩进空格数

  HtmlElement() {}

  HtmlElement(const string &name, const string &text)
      : name(name), text(text) {}

  // 递归地将 HTML 元素转为格式化的字符串（含缩进）
  string str(int indent = 0) const {
    ostringstream oss;
    string i(indent_size * indent, ' ');    // 当前层级的缩进
    oss << i << "<" << name << ">" << endl;

    // 输出文本内容（如果有），比标签多一级缩进
    if (text.size() > 0)
      oss << string(indent_size * (indent + 1), ' ') << text << endl;

    // 递归处理子元素
    for (const auto &e : elements)
      oss << e.str(indent + 1);

    oss << i << "</" << name << ">" << endl;
    return oss.str();
  }
};

// ---------------------------------------------------------------------------
// ConcreteBuilder：HTML 建造者
// 封装了 HTML 元素的构建逻辑，提供逐步添加子元素的 API
// ---------------------------------------------------------------------------
struct HtmlBuilder {
  HtmlElement root;   // 待构建的根元素

  HtmlBuilder(string root_name) { root.name = root_name; }

  // 添加一个子元素到根元素中
  void add_child(string child_name, string child_text) {
    HtmlElement e{child_name, child_text};
    root.elements.emplace_back(e);    // emplace_back：原地构造，避免拷贝
  }

  // 获取构建结果的字符串表示
  string str() const { return root.str(); }
};

// ---------------------------------------------------------------------------
// 主函数
// ---------------------------------------------------------------------------
int main() {
  cout << "Naive Builder（朴素拼接方式）:" << endl;
  naive_html_builder();

  cout << "Builder Pattern（建造者模式）: " << endl;
  HtmlBuilder builder{"ul"};                 // 创建以 <ul> 为根的建造者
  builder.add_child("li", "hello");          // 逐步添加子元素
  builder.add_child("li", "world");
  cout << builder.str() << endl;             // 输出构建结果

  return 0;
}
