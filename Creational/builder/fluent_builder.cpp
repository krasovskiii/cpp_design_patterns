/*
 * =============================================================================
 * 设计模式：流式建造者模式 (Fluent Builder Pattern)
 * =============================================================================
 *
 * 【一句话概括】
 * 通过让建造者的每个方法返回 *this 引用，实现链式调用（方法级联），
 * 使对象构建代码更加流畅、可读。
 *
 * 【适用场景 —— 通用】
 * - 需要连续设置多个属性或添加多个子组件
 * - 希望构建代码具有 DSL（领域特定语言）风格
 * - 配合静态工厂方法 create() 和终结方法 build() 提供清晰的构建生命周期
 * - API 请求构建：http::Request::create().url("/api/v1/orders").header("Auth", token).body(json).build()
 * - 断言库：EXPECT_THAT(value, IsGreaterThan(10).And(IsLessThan(20))) 风格
 *
 * 【金融工程应用】
 * - 期权策略构建：OptionStrategy::create().buyCall("510050", 2.50).sellCall("510050", 2.60)
 *     .expiry("2024-06").build()  —— 构建牛市看涨价差，链式表达清晰直观
 * - 投资组合构建：Portfolio::Builder().addAsset("000300", 0.3).addAsset("002415", 0.2)
 *     .constraint(MaxWeight(0.35)).objective(MinVariance()).build()
 * - 风控规则链：RiskRules::create().addRule(MaxPositionRule(100)).addRule(StopLossRule(0.05))
 *     .addRule(CircuitBreakerRule(0.10)).build()
 *
 * 【本例说明】
 * HtmlBuilder 的 add_child() 返回自身引用，使得可以连续调用。
 * 构造函数被隐藏，强制用户使用建造者。
 * 提供 create() 静态工厂方法和 build() 终结方法。
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// ---------------------------------------------------------------------------
// Product：HTML 元素
// 构造函数为 private，通过 friend 授权给建造者
// ---------------------------------------------------------------------------
class HtmlElement {
  friend class HtmlBuilder;   // 允许建造者访问私有成员

  std::string name, text;
  std::vector<HtmlElement> elements;
  const size_t indent_size = 2;

  // 隐藏构造函数 —— 强制用户通过建造者创建对象
  HtmlElement() {}

  HtmlElement(const std::string &name, const std::string &text)
      : name(name), text(text) {}

public:
  // 递归生成格式化的 HTML 字符串
  std::string str(int indent = 0) const {
    ostringstream oss;
    string i(indent_size * indent, ' ');
    oss << i << "<" << name << ">" << endl;
    if (text.size() > 0)
      oss << string(indent_size * (indent + 1), ' ') << text << endl;

    for (const auto &e : elements)
      oss << e.str(indent + 1);

    oss << i << "</" << name << ">" << endl;
    return oss.str();
  }
};

// ---------------------------------------------------------------------------
// ConcreteBuilder：流式 HTML 建造者
// 每个构建方法返回 *this 实现链式调用
// ---------------------------------------------------------------------------
class HtmlBuilder {
  HtmlElement root;   // 内部持有的待构建元素

public:
  HtmlBuilder(std::string root_name) { root.name = root_name; }

  // 流式 API：返回自身引用，支持链式调用
  // 用法：builder.add_child("li","a").add_child("li","b")
  HtmlBuilder &add_child(std::string child_name, std::string child_text) {
    HtmlElement e{child_name, child_text};
    root.elements.emplace_back(e);
    return *this;   // 关键：返回自身引用
  }

  std::string str() const { return root.str(); }

  // 隐式类型转换：允许将 Builder 直接赋值给 HtmlElement
  operator HtmlElement() const { return root; }

  // build() 终结方法：明确表示构建完成，返回最终产品
  // 使用 std::move 避免不必要的拷贝
  HtmlElement build() {
    return std::move(root);    // root 被移走后不应再使用此建造者
  }

  // 静态工厂方法：提供更清晰的创建入口
  static HtmlBuilder create(std::string root_name) { return {root_name}; }
};

// ---------------------------------------------------------------------------
// 主函数：展示三种使用方式
// ---------------------------------------------------------------------------
int main() {
  // 用户无法直接创建 HtmlElement（构造函数是私有的）
  // HtmlElement e;  // 编译错误！

  // ---- 方式一：传统流式调用 ----
  cout << "Fluent Builder（流式调用）:" << endl;
  HtmlBuilder builder{"ul"};
  builder.add_child("li", "hello").add_child("li", "world");  // 链式调用
  cout << builder.str() << endl;

  // ---- 方式二：Create 语法（隐式类型转换） ----
  // 利用 operator HtmlElement() 将建造者转换为元素
  // 缺点：用户可能不知道可以这样用
  cout << "Fluent Builder - Create 语法:" << endl;
  HtmlElement elem = HtmlBuilder::create("ul")
                         .add_child("li", "hello")
                         .add_child("li", "world");
  cout << elem.str() << endl;

  // ---- 方式三：Create + Build 语法（推荐） ----
  // build() 明确表示构建完成，语义清晰
  cout << "Fluent Builder - Create + Build 语法:" << endl;
  auto elem2 = HtmlBuilder::create("ul")
                   .add_child("li", "using create")
                   .add_child("li", "and build syntax")
                   .build();          // 显式终结构建过程
  cout << elem2.str() << endl;

  return 0;
}
