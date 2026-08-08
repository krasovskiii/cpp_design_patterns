/*
 * =============================================================================
 * 设计模式：解释器模式（Interpreter）
 * =============================================================================
 *
 * 【一句话概括】
 * 为一种语言定义其文法表示，并定义一个解释器来解释该语言中的句子。
 *
 * 【适用场景 —— 通用】
 * - 需要解释执行一种简单语言或表达式
 * - 语法相对简单，不需要复杂的解析器生成器
 * - 需要频繁修改或扩展语法规则
 * - SQL 解析器、正则表达式引擎、配置文件 DSL 解析
 *
 * 【金融工程应用】
 * - 因子表达式解析器：解析 "RSI(14) > 70 AND MACD(12,26) > 0" 等因子公式，
 *   动态构建因子计算 AST，支持用户自定义因子表达式
 * - 交易规则 DSL：解析 "BUY WHEN MA(5) CROSS ABOVE MA(20) AND VOLUME > 1000000"
 *   将自然规则语言转为可执行的信号生成代码
 * - 期权组合表达式：解析 "BUY CALL@3500 + SELL CALL@3600" 组合策略表达式，
 *   自动构建策略盈亏图
 * - FIX 消息解析：解释器解析 FIX 协议的 tag-value 消息，
 *   转为内部 Order 对象，处理不同版本的 FIX 协议差异
 * - 风控规则表达式：解析 "position_value > 0.2 * nav OR margin > available * 0.8"
 *   动态加载和执行风控规则
 *
 * 【关键参与者】
 *   - AbstractExpression（抽象表达式）：Element 接口，定义 eval() 求值方法
 *   - TerminalExpression（终结符表达式）：Integer，代表具体数值
 *   - NonterminalExpression（非终结符表达式）：BinaryOperation，代表二元运算
 *   - Context（上下文）：Token 序列（词法分析结果）
 */

#include <cctype>
#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// Token（词法单元）：表示表达式中的一个最小语法单元
struct Token {
  enum Type { integer, plus, minus, lparen, rparen } type; // Token 类型
  string text;                                              // Token 的文本内容

  Token(Type type, const string &text) : type(type), text(text) {}

  friend ostream &operator<<(ostream &os, const Token &token) {
    os << "`" << token.text << "`";
    return os;
  }
};

// 词法分析器（Lexer）：将输入字符串转换为 Token 序列
vector<Token> lex(const string &input) {
  vector<Token> result;

  for (size_t i = 0; i < input.size(); ++i) {
    switch (input[i]) {
    case '+':
      result.push_back({Token::plus, "+"});    // 识别加号
      break;
    case '-':
      result.push_back({Token::minus, "-"});   // 识别减号
      break;
    case '(':
      result.push_back({Token::lparen, "("});  // 识别左括号
      break;
    case ')':
      result.push_back({Token::rparen, ")"});  // 识别右括号
      break;
    default:
      // 读取连续的数字字符，构成一个整数 Token
      ostringstream buffer;
      buffer << input[i];
      for (size_t j = i + 1; j < input.size(); ++j) {
        if (isdigit(input[j])) {
          buffer << input[j];
          ++i; // 跳过已处理的数字字符
        } else {
          result.push_back({Token::integer, buffer.str()});
          break;
        }
      }
    }
  }
  return result;
}

// 抽象表达式（AbstractExpression）：定义求值接口
struct Element {
  virtual int eval() const = 0; // 纯虚函数：计算表达式的值
};

// 终结符表达式（TerminalExpression）：整数字面量
struct Integer : Element {
  int value;

  explicit Integer(int value) : value{value} {}

  int eval() const override { return value; } // 直接返回数值
};

// 非终结符表达式（NonterminalExpression）：二元运算（加/减）
struct BinaryOperation : Element {
  enum Type { addition, substraction } type;   // 运算类型
  shared_ptr<Element> lhs, rhs;                // 左右子表达式

  int eval() const override {
    auto left = lhs->eval();   // 递归求左值
    auto right = rhs->eval();  // 递归求右值

    if (type == Type::addition) {
      return left + right;     // 加法运算
    }
    return left - right;       // 减法运算
  }
};

// 语法解析器（Parser）：将 Token 序列转换为抽象语法树（AST）
shared_ptr<Element> parse(const vector<Token> &tokens) {
  auto result = make_unique<BinaryOperation>(); // 根节点为二元运算
  bool have_lhs{false}; // 是否已设置左操作数

  for (size_t i = 0; i < tokens.size(); ++i) {
    auto &token = tokens[i];

    switch (token.type) {
    case Token::integer: {
      int value = stoi(token.text);
      auto integer = make_shared<Integer>(value);
      if (!have_lhs) {
        result->lhs = integer; // 设置为左操作数
        have_lhs = true;
      } else {
        result->rhs = integer; // 设置为右操作数
      }
      break;
    }
    case Token::plus:
      result->type = BinaryOperation::addition; // 设置加法运算
      break;
    case Token::minus:
      result->type = BinaryOperation::substraction; // 设置减法运算
      break;
    case Token::lparen: {
      // 遇到左括号：找到匹配的右括号，递归解析子表达式
      size_t j = i;
      for (; j < tokens.size(); ++j) {
        if (tokens[j].type == Token::rparen) {
          break; // 找到匹配的右括号
        }
      }
      // 提取子表达式的 Token 序列并递归解析
      vector<Token> subexpression(&tokens[i + 1], &tokens[j]);
      auto element = parse(subexpression);
      if (!have_lhs) {
        result->lhs = element;
        have_lhs = true;
      } else {
        result->rhs = element;
      }
      i = j; // 跳过子表达式
      break;
    }
    case Token::rparen:
      break; // 忽略右括号（已在 lparen 中处理）
    }
  }
  return result; // 返回构建的抽象语法树
}

// 客户端：驱动词法分析和语法解析，最终求值
int main() {
  string input{"(13-4)-(12+1)"}; // 待解释的表达式：(13-4)-(12+1) = 9-13 = -4
  auto tokens = lex(input);       // 第一步：词法分析
  for (auto &t : tokens) {
    cout << t << " ";
  }
  cout << endl;

  try {
    auto parsed = parse(tokens);             // 第二步：语法解析，构建 AST
    cout << input << " = " << parsed->eval() << endl; // 第三步：解释执行（求值）
  } catch (const exception &e) {
    cout << e.what() << endl;
  }

  return 0;
}
