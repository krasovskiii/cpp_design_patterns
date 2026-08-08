/*
 * =============================================================================
 * 设计模式：访问者模式（Visitor）—— 非循环访问者
 * =============================================================================
 *
 * 【一句话概括】
 * 基于 RTTI 实现访问者，访问者只需实现其关心的类型，层次结构可逐步扩展。
 *
 * 【适用场景】
 * - 类层次结构经常变化，需要逐步添加新类型
 * - 访问者不需要处理所有元素类型
 *
 * 【金融工程应用】
 * - 渐进式产品系统：新金融产品（如雪球/凤凰/鲨鱼鳍）逐步加入系统，
 *   已有访问者（估值/风控）不需要立即支持新产品，逐步添加支持
 * - 模块化风控：不同风控模块（保证金/持仓限制/流动性）只关注自己关心的产品类型，
 *   不需要为所有产品类型实现 visit()
 *
 * 【关键参与者】
 *   - VisitorBase（标记接口）：所有访问者的基类
 *   - Visitor<T>（类型化访问者接口）：为特定元素类型 T 定义 visit() 方法
 */

#include <iostream>
#include <sstream>
#include <string>
using namespace std;

// -----------------------------------------------------------------------------
// 访问者基类
// -----------------------------------------------------------------------------

// 类型化访问者接口模板：为 Visitable 类型定义 visit() 方法
template <typename Visitable> struct Visitor {
  virtual void visit(Visitable &obj) = 0;
};

// VisitorBase 作为标记接口（Marker Interface），供层次结构中的 accept() 方法使用
// accept() 方法会通过 dynamic_cast 将 VisitorBase 转换为 Visitor<T> 类型
struct VisitorBase {
  virtual ~VisitorBase() = default;
};

// -----------------------------------------------------------------------------
// 元素层次结构
// -----------------------------------------------------------------------------

// 抽象表达式（Element）
struct Expression {
  virtual ~Expression() = default;

  // accept() 接收 VisitorBase 对象，期望它实际是 Visitor<Expression> 类型
  virtual void accept(VisitorBase &obj) {
    using EV = Visitor<Expression>; // <--- 期望访问者实现了 Visitor<Expression>
    if (auto ev = dynamic_cast<EV *>(&obj)) // 运行时类型检查
      ev->visit(*this);
  }
};

// 具体元素：双精度浮点数（DoubleExpression）
struct DoubleExpression : Expression {
  double value;

  explicit DoubleExpression(double value) : value(value) {}

  virtual void accept(VisitorBase &obj) override {
    using DEV = Visitor<DoubleExpression>; // <--- 期望访问者实现了 Visitor<DoubleExpression>
    if (auto ev = dynamic_cast<DEV *>(&obj)) // 运行时类型检查
      ev->visit(*this);
  }
};

// 具体元素：加法表达式（AdditionExpression）
struct AdditionExpression : Expression {
  Expression *left, *right;

  AdditionExpression(Expression *left, Expression *right)
      : left(left), right(right) {}

  ~AdditionExpression() {
    delete left;
    delete right;
  }

  virtual void accept(VisitorBase &obj) override {
    using AEV = Visitor<AdditionExpression>; // <--- 期望访问者实现了 Visitor<AdditionExpression>
    if (auto ev = dynamic_cast<AEV *>(&obj)) // 运行时类型检查
      ev->visit(*this);
  }
};

// -----------------------------------------------------------------------------
// 具体访问者：ExpressionPrinter
// -----------------------------------------------------------------------------

// 这种方法的额外灵活性：
// 访问者不必继承所有 Visitable 类型的 Visitor 接口
// 层次结构不需要是刚性的：可以逐步添加新类型，
// 而不必在所有地方创建 accept() 方法

struct ExpressionPrinter
    : VisitorBase,               // <--- 继承标记接口
      Visitor<Expression>,       // <-- 可以作为回退处理器（fallback）
      // Visitor<DoubleExpression>, // <-- 可以选择性移除不需要的类型！
      Visitor<AdditionExpression> {

  // 回退处理：如果访问者没有为 Expression 实现特定逻辑
  void visit(Expression &) override {
    // 不是真正的回退！仅用于演示
    oss << "<this will not be displayed>";
  }

  // 可以移除的：如果访问者不需要处理 DoubleExpression
  // void visit(DoubleExpression &obj) override { oss << obj.value; }

  // 处理 AdditionExpression
  void visit(AdditionExpression &obj) override {
    oss << "(";
    obj.left->accept(*this);  // 递归访问左子表达式
    oss << "+";
    obj.right->accept(*this); // 递归访问右子表达式
    oss << ")";
  }

  string str() const { return oss.str(); }

private:
  ostringstream oss;
};

// 客户端：演示非循环访问者模式
int main() {
  // 构建表达式：1 + 2 + 3
  auto e = new AdditionExpression{
      new DoubleExpression{1},
      new AdditionExpression{new DoubleExpression{2}, new DoubleExpression{3}}};

  ExpressionPrinter ep;
  ep.visit(*e); // 直接调用访问者的 visit 方法
  cout << ep.str() << "\n"; // 输出: (1+(2+3))

  return 0;
}
