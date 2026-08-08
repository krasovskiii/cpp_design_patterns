/*
 * ===========================================================================
 * 设计模式：Builder Exercise（建造者模式练习）
 * ===========================================================================
 *
 * 【核心思想】
 * 使用 Builder 模式构建编程语言的类定义代码。这是一个贴近实际开发的练习：
 * CodeBuilder 将 C++ 类的代码生成过程封装为流式 API，使得构建"类的代码字符串"
 * 像搭积木一样直观。
 *
 * 【适用场景】
 * - 代码生成器（如 ORM 生成 Model 类、Protobuf 编译器）
 * - 需要程序化构建结构化文本（代码、配置文件、文档）的场景
 * - 将构建逻辑与表示分离的通用场景
 * - SQL 查询构建器：QueryBuilder.select().from().where().orderBy() 链式构建 SQL
 * - 测试数据构造器：TestDataBuilder 逐步填充测试对象，确保测试数据的完整性和可读性
 *
 * 【金融工程应用】
 * - CTP/FIX 消息构建器：自动生成交易 API 的消息类代码，确保 tag 编号和字段类型正确
 *   避免手动编写大量 boilerplate 代码，减少 tag 编号错误导致的成交异常
 * - 策略代码生成器：根据策略配置自动生成策略框架代码（初始化/onBar/onOrder/风控检查）
 *   策略研究员只需填充核心信号逻辑，框架代码由 Builder 自动生成
 * - 因子计算表达式构建：将因子公式（如 RSI(14) * MACD(12,26,9)）解析为可执行代码
 *   FactorCodeBuilder 生成带缓存的因子计算代码，提升回测效率
 *
 * 【UML 关键参与者】
 * - Product（产品）：Class —— 表示一个类的完整定义（名称 + 字段列表）
 * - Part（部件）：Field —— 表示类的一个成员字段（类型 + 名称）
 * - Builder（建造者）：CodeBuilder —— 流式添加字段，最终输出类定义
 *
 * 【本例要点】
 * - Field 的构造函数私有，只有 CodeBuilder 可以创建字段
 * - Class 的构造函数私有，只有 CodeBuilder 可以创建类
 * - CodeBuilder.add_field() 返回 *this 实现流式调用
 * - operator<< 重载使 CodeBuilder 可直接输出到 ostream
 */

#include <iostream>
#include <string>
#include <vector>
using namespace std;

/*
 * Field: 类成员字段
 *
 * 表示一个类成员变量的声明，例如 "string name;" 或 "int age;"。
 * 构造函数私有，只有 friend 类 CodeBuilder 可以创建 Field 实例。
 */
class Field {
  string name;  // 字段名称
  string type;  // 字段类型

  // 构造函数私有 —— 只有 CodeBuilder 可以创建字段
  friend class CodeBuilder;
  Field(const string &name, const string &type) : name{name}, type{type} {}

public:
  // 输出字段声明：如 "string name;"
  friend ostream &operator<<(ostream &os, const Field &obj) {
    return os << obj.type << " " << obj.name << ";";
  }
};

/*
 * Class: 类定义（Product 角色）
 *
 * 表示一个完整的类定义，包含类名和字段列表。
 * 构造函数私有，通过 CodeBuilder 创建。
 */
class Class {
  string name;              // 类名
  vector<Field> fields;     // 字段列表

  // 构造函数私有 —— 只有 CodeBuilder 可以创建类
  friend class CodeBuilder;
  Class() {}
  Class(string name) : name(name) {}

public:
  // 添加字段（由 CodeBuilder 调用）
  void add(Field field) { fields.push_back(field); }

  // 输出完整的类定义代码
  friend ostream &operator<<(ostream &os, const Class &obj) {
    os << "class " << obj.name << endl << "{" << endl;

    // 输出每个字段声明，带缩进
    for (auto &&field : obj.fields)
      os << "  " << field << endl;

    os << "};";
    return os;
  }
};

/*
 * CodeBuilder: 代码建造者（Builder 角色）
 *
 * 封装了 Class 对象的构建过程，提供流式 API：
 * CodeBuilder{"Person"}.add_field("name","string").add_field("age","int")
 */
class CodeBuilder {
  Class code;  // 正在构建的类对象

public:
  // 构造 Builder 时指定类名
  CodeBuilder(const string &class_name) { code.name = class_name; }

  // 添加字段，返回 *this 支持链式调用
  CodeBuilder &add_field(const string &name, const string &type) {
    code.add(Field(name, type));
    return *this;
  }

  // 输出 Builder（即输出内部的 Class 对象）
  friend ostream &operator<<(ostream &os, const CodeBuilder &obj) {
    return os << obj.code;
  }
};

int main() {
  // 使用 Builder 构建一个 Person 类定义：
  // class Person {
  //   string name;
  //   int age;
  // };
  auto cb =
      CodeBuilder{"Person"}.add_field("name", "string").add_field("age", "int");
  cout << cb;

  return 0;
}
