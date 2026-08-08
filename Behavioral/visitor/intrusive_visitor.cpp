/*
 * =============================================================================
 * 设计模式：访问者模式（Visitor）—— 侵入式访问者
 * =============================================================================
 *
 * 【一句话概括】
 * 将操作直接添加到被访问对象的类层次结构中，每个类自己实现操作方法。
 *
 * 【适用场景】
 * - 类层次结构稳定，操作变化较少
 * - 简单场景下快速实现
 *
 * 【金融工程应用】
 * - 金融产品自描述：每种产品（期货/期权/互换）内置自己的风险描述方法，
 *   适合产品类型固定的简单系统
 *
 * 【缺点】添加新操作（如求值）需要修改所有子类，违反开闭原则。
 *
 * 【关键参与者】
 *   - Element（元素接口）：Expression，定义 accept/print 方法
 *   - ConcreteElement（具体元素）：DoubleExpression、AdditionExpression
 */

#include <iostream>
#include <sstream>
#include <string>
using namespace std;

// 数学表达式层次结构
// 我们想为这些表达式添加打印功能

// 抽象表达式（Element）：定义打印接口
struct Expression {
  virtual ~Expression() = default;

  // 侵入式实现：打印操作直接定义在类中
  virtual void print(ostringstream &oss) = 0;
};

// 具体表达式：双精度浮点数（终结符）
struct DoubleExpression : Expression {
  double value;
  explicit DoubleExpression(const double value) : value{value} {}

  // 侵入式实现：打印数值
  void print(ostringstream &oss) override { oss << value; }
};

// 具体表达式：加法表达式（非终结符）
struct AdditionExpression : Expression {
  Expression *left, *right; // 左右子表达式

  AdditionExpression(Expression *const left, Expression *const right)
      : left{left}, right{right} {}

  ~AdditionExpression() {
    delete left;
    delete right;
  }

  // 侵入式实现：递归打印 "(left+right)"
  void print(ostringstream &oss) override {
    oss << "(";
    left->print(oss);  // 递归打印左子表达式
    oss << "+";
    right->print(oss); // 递归打印右子表达式
    oss << ")";
  }
};

// 客户端：演示侵入式访问者
int main() {
  // 构建表达式：1 + 2 + 3
  //    __+__
  //   1    _+_
  //       2   3

  auto e = new AdditionExpression{
      new DoubleExpression{1},
      new AdditionExpression{new DoubleExpression{2}, new DoubleExpression{3}}};
  ostringstream oss;
  e->print(oss); // 调用侵入式打印方法
  cout << oss.str() << endl; // 输出: (1+(2+3))

  return 0;
}
