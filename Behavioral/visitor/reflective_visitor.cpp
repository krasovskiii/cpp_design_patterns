/*
 * =============================================================================
 * 设计模式：访问者模式（Visitor）—— 反射式访问者
 * =============================================================================
 *
 * 【一句话概括】
 * 将操作从类层次结构中分离出来，使用 dynamic_cast 进行运行时类型分派。
 *
 * 【适用场景】
 * - 快速原型开发
 * - 类层次结构很少变化
 *
 * 【金融工程应用】
 * - 快速原型：在金融产品类型尚不稳定的开发早期，使用反射式访问者快速实现
 *   估值/风险计算，待类型稳定后迁移到双分派访问者
 *
 * 【缺点】dynamic_cast 效率低，添加新子类时编译器不报错，容易遗漏。
 *
 * 【关键参与者】
 *   - Element（元素接口）：Expression，仅定义虚析构函数
 *   - Visitor（访问者）：ExpressionPrinter，在类外部实现逻辑
 */

#include <iostream>
#include <sstream>
#include <string>
using namespace std;

// 数学表达式层次结构
// 我们想为这些表达式添加打印功能

// 抽象表达式（Element）：不包含任何操作
struct Expression {
  virtual ~Expression() = default;
};

// 具体表达式：双精度浮点数
struct DoubleExpression : Expression {
  double value;
  explicit DoubleExpression(const double value) : value{value} {}
};

// 具体表达式：加法表达式
struct AdditionExpression : Expression {
  Expression *left, *right;

  AdditionExpression(Expression *const left, Expression *const right)
      : left{left}, right{right} {}

  ~AdditionExpression() {
    delete left;
    delete right;
  }
};

// 表达式打印机（Visitor）：在类外部实现打印逻辑！好的设计！
// 但 if-else + dynamic_cast 的链式检查无法避免。
struct ExpressionPrinter {

  // 以下基于重载的实现无法工作，
  // 因为在编译期我们不知道 Expression 指针的具体类型！
  //
  // void print(DoubleExpression *de, ostringstream &oss) const {
  //   oss << de->value;
  // }
  // void print(AdditionExpression *ae, ostringstream &oss) const {
  //   oss << "(";
  //   print(ae->left, oss);
  //   oss << "+";
  //   print(ae->right, oss);
  //   oss << ")";
  // }

  ostringstream oss;

  // 使用 dynamic_cast 在运行时检查类型（反射式分派）
  void print(Expression *e) {
    // 尝试转换为 DoubleExpression
    if (auto de = dynamic_cast<DoubleExpression *>(e)) {
      oss << de->value;
    // 尝试转换为 AdditionExpression
    } else if (auto ae = dynamic_cast<AdditionExpression *>(e)) {
      oss << "(";
      print(ae->left);  // 递归打印左子表达式
      oss << "+";
      print(ae->right); // 递归打印右子表达式
      oss << ")";
    }
    // 如果添加新的 Expression 子类而忘记更新此处，不会报错！
  }

  string str() const { return oss.str(); }
};

// 客户端：演示反射式访问者
int main() {
  // 构建表达式：1 + 2 + 3
  //    __+__
  //   1    _+_
  //       2   3

  auto e = new AdditionExpression{
      new DoubleExpression{1},
      new AdditionExpression{new DoubleExpression{2}, new DoubleExpression{3}}};

  ExpressionPrinter ep;
  ep.print(e); // 外部访问者执行打印操作
  cout << ep.str() << endl; // 输出: (1+(2+3))
}
