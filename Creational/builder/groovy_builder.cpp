/*
 * ===========================================================================
 * 设计模式：Groovy-style Builder（Groovy 风格建造者模式）
 * ===========================================================================
 *
 * 【核心思想】
 * 利用 C++ 的初始化列表（initializer_list）和继承机制，实现类似 Groovy/Kotlin
 * 等语言的 DSL（领域特定语言）风格的构建语法。通过为每种 HTML 标签定义专门的类，
 * 让代码看起来就像在写 HTML 本身。
 *
 * 【适用场景】
 * - 需要构建嵌套结构的数据（如 HTML、XML、JSON、UI 组件树）
 * - 希望构建代码具有极高的可读性，接近声明式语法
 * - 目标语言是 DSL 友好的场景（配置文件生成、报告生成等）
 *
 * 【金融工程应用】
 * - 期权组合 DSL：构建复杂的期权组合策略（蝶式/铁鹰/跨式），使用嵌套声明式语法
 *   Butterfly{Call{strike:3500}, Put{strike:3400}, Call{strike:3300}} 直观表达策略结构
 * - 因子表达式 DSL：定义多层嵌套的因子计算表达式树
 *   CompositeFactor{WeightFactor{RSI(14), 0.5}, WeightFactor{MACD(), 0.3}, WeightFactor{VOL(20), 0.2}}
 * - 报表模板定义：声明式定义风险报表的结构和内容层级
 *   Report{Section{"VaR Analysis", Chart{...}, Table{...}}, Section{"Greeks", ...}}
 *
 * 【UML 关键参与者】
 * - Product（产品）：Tag 及其子类（P, Image） —— 具体标签元素
 * - Builder（建造者）：无显式 Builder —— 构建逻辑嵌入在 Tag 继承体系中
 * - 关键机制：通过构造函数接收 initializer_list<Tag> 实现嵌套子元素
 *
 * 【本例要点】
 * - Tag 是基类，封装了 HTML 标签的通用行为（名称、文本、子元素、属性）
 * - P（段落）和 Image（图片）继承 Tag，提供类型安全的标签构造
 * - 通过 C++ 的初始化列表语法实现嵌套：P{Image{"url"}}
 * - 这种风格在 Groovy/Kotlin 等语言中非常流行，C++ 中通过继承模拟
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

/*
 * Tag: 通用 HTML 标签基类
 *
 * 封装了 HTML 标签的所有通用属性：
 * - name:       标签名称（如 "p", "img"）
 * - text:       标签内的文本内容
 * - children:   子标签列表（递归嵌套）
 * - attributes: 属性键值对列表（如 src="url"）
 *
 * 构造函数为 protected，意味着只有子类可以构造 Tag 实例。
 * 这强制用户使用具体的标签子类（如 P, Image）来创建元素。
 *
 * operator<< 重载使得 Tag 对象可以直接输出到 ostream，
 * 递归地生成完整的 HTML 标记。
 */
struct Tag {
  string name, text;
  vector<Tag> children;
  vector<pair<string, string>> attributes;

  // 重载输出运算符，递归生成 HTML 字符串
  friend ostream &operator<<(ostream &os, const Tag &tag) {
    os << "<" << tag.name;

    // 输出所有属性（如 src="http://..."）
    for (const auto &att : tag.attributes)
      os << " " << att.first << "=\"" << att.second << "\"";

    // 如果是自闭合标签（无子元素、无文本），直接闭合
    if (tag.children.size() == 0 && tag.text.length() == 0)
      os << "/>" << endl;
    else {
      // 开始标签 > 结束
      os << ">" << endl;

      // 输出文本内容
      if (tag.text.length())
        os << tag.text << endl;

      // 递归输出所有子标签
      for (const auto &child : tag.children)
        os << child;

      // 闭合标签
      os << "</" << tag.name << ">" << endl;
    }
    return os;
  }

protected:
  // 构造函数：名称 + 文本（用于叶子标签如 <p>text</p>）
  Tag(const string &name, const string &text) : name(name), text(text) {}

  // 构造函数：名称 + 子标签列表（用于容器标签如 <ul><li>...</li></ul>）
  Tag(const string &name, const vector<Tag> &children)
      : name(name), children(children) {}
};

/*
 * P: 段落标签（<p>）
 *
 * 继承自 Tag，通过构造函数自动设定标签名为 "p"。
 * 提供两种构造方式：
 * 1. 文本构造：P("some text") -> <p>some text</p>
 * 2. 子元素构造：P{Image{"url"}} -> <p><img src="url"/></p>
 *
 * 使用 initializer_list<Tag> 参数，支持 P{child1, child2, ...} 语法。
 */
struct P : Tag {
  P(const string &text) : Tag("p", text) {}
  P(initializer_list<Tag> children) : Tag("p", children) {}
};

/*
 * Image: 图片标签（<img>）
 *
 * 继承自 Tag，自动设定标签名为 "img"。
 * 构造时接受图片 URL，并将其作为 src 属性存储。
 * 由于没有文本和子元素，输出为自闭合标签：<img src="url"/>
 */
struct Image : Tag {
  explicit Image(const string &url) : Tag{"img", ""} {
    attributes.emplace_back(make_pair("src", url));
  }
};

int main() {
  // Groovy 风格的嵌套语法：
  // P{Image{"url"}} 表示：一个段落中包含一个图片
  //
  // 展开后的 HTML：
  // <p>
  //   <img src="http://google.com"/>
  // </p>
  //
  // 这种写法的可读性极高，接近声明式 DSL
  cout <<

      P{Image{"http://google.com"}}

       << endl;
  return 0;
}
