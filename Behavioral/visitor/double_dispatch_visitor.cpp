/*
 * =============================================================================
 * 设计模式：访问者模式（Visitor）—— 双分派访问者（经典 GoF 实现）
 * =============================================================================
 *
 * 【一句话概括】
 * 通过 accept(visitor) + visitor.visit(element) 实现双分派，根据运行时类型决定调用。
 *
 * 【适用场景 —— 通用】
 * - 需要对一个稳定的类层次结构添加多种操作
 * - 类层次结构中的类很少变化，但操作经常增加
 * - 需要在编译期保证所有类型都被处理
 *
 * 【金融工程应用】
 * - 金融产品多操作：金融产品层次（期货/期权/互换/结构化）稳定，但操作频繁增加
 *   （估值/风控/结算/报表/监管报送），新增操作只需添加新 Visitor，产品类无需修改
 *   - PricerVisitor：统一估值接口，调用各产品的定价模型
 *   - RiskVisitor：计算各产品的希腊字母和 VaR 贡献
 *   - SettlementVisitor：处理各产品的结算逻辑
 *   - RegulatoryVisitor：生成监管报表数据
 * - 订单类型多操作：订单层次（限价/市价/止损/冰山/TWAP）稳定，操作频繁增加
 *   - ValidateVisitor：验证订单参数合法性
 *   - RouteVisitor：路由订单到交易所
 *   - FeeVisitor：计算手续费
 *   - AuditVisitor：生成审计日志
 * - 行情数据多操作：行情类型（Tick/Bar/OrderBook）稳定，操作频繁增加
 *   - NormalizeVisitor：标准化处理
 *   - AdjustVisitor：复权处理
 *   - FilterVisitor：异常值过滤
 *
 * 【关键参与者】
 *   - Visitor（访问者接口）：ExpressionVisitor，为每种元素类型定义 visit() 方法
 *   - ConcreteVisitor（具体访问者）：ExpressionPrinter、ExpressionEvaluator
 *   - Element（元素接口）：Expression，定义 accept(visitor) 方法
 */

#include <iostream>
#include <sstream>
#include <string>
using namespace std;

// 数学表达式层次结构
// 我们想为这些表达式添加打印和求值功能

// =============================================================================
// 双分派处理器
// - 需要额外的模板代码，但在合理范围内
// - 可以轻松处理多种操作（打印、求值等）

struct SubtractionExpression;
struct DoubleExpression;
struct AdditionExpression;

// 访问者接口（Visitor）：为每种元素类型定义 visit() 方法
struct ExpressionVisitor {
  virtual void visit(DoubleExpression *de) = 0;
  virtual void visit(AdditionExpression *ae) = 0;
  virtual void visit(SubtractionExpression *se) = 0;
};

// 具体访问者1：打印（ExpressionPrinter）
struct ExpressionPrinter : ExpressionVisitor {
  ostringstream oss;
  string str() const { return oss.str(); }
  void visit(DoubleExpression *de) override;
  void visit(AdditionExpression *ae) override;
  void visit(SubtractionExpression *se) override;
};

// 具体访问者2：求值（ExpressionEvaluator）
struct ExpressionEvaluator : ExpressionVisitor {
  double result;
  void visit(DoubleExpression *de) override;
  void visit(AdditionExpression *ae) override;
  void visit(SubtractionExpression *se) override;
};

// =============================================================================
// 元素层次结构：Expression、DoubleExpression、AdditionExpression、SubtractionExpression

// 抽象元素（Element）
struct Expression {
  virtual ~Expression() = default;

  // 每个元素必须实现 accept() 方法
  // 所有子类的 accept() 实现都一样！
  virtual void accept(ExpressionVisitor *visitor) = 0;
};

// 具体元素：双精度浮点数（DoubleExpression）
struct DoubleExpression : Expression {
  double value;
  explicit DoubleExpression(const double value) : value{value} {}

  // accept() 的实现在所有元素中都是相同的，但无法提取到基类！
  // 关键点在于 `this` 指针的类型：
  // 它强制编译器选择正确的 visit() 重载（DoubleExpression* 版本）
  void accept(ExpressionVisitor *visitor) override { visitor->visit(this); }
};

// 具体元素：加法表达式（AdditionExpression）
struct AdditionExpression : Expression {
  Expression *left, *right;

  AdditionExpression(Expression *const left, Expression *const right)
      : left{left}, right{right} {}

  ~AdditionExpression() {
    delete left;
    delete right;
  }

  void accept(ExpressionVisitor *visitor) override { visitor->visit(this); }
};

// 具体元素：减法表达式（SubtractionExpression）
struct SubtractionExpression : Expression {
  Expression *left, *right;

  SubtractionExpression(Expression *const left, Expression *const right)
      : left{left}, right{right} {}

  ~SubtractionExpression() {
    delete left;
    delete right;
  }

  void accept(ExpressionVisitor *visitor) override { visitor->visit(this); }
};

// =============================================================================
// 访问者必须为每种元素类型实现特定的处理逻辑

// ExpressionPrinter: 打印 DoubleExpression
void ExpressionPrinter::visit(DoubleExpression *de) { oss << de->value; }

// ExpressionPrinter: 打印 AdditionExpression
void ExpressionPrinter::visit(AdditionExpression *e) {
  // 仅当右子表达式是 SubtractionExpression 时才打印括号
  bool need_braces = dynamic_cast<SubtractionExpression *>(e->right);
  e->left->accept(this); // 双分派：通过 accept 调用正确的 visit
  oss << "-";
  if (need_braces)
    oss << "(";
  e->right->accept(this);
  if (need_braces)
    oss << ")";
}

// ExpressionPrinter: 打印 SubtractionExpression
void ExpressionPrinter::visit(SubtractionExpression *se) {
  // 仅当右子表达式是 SubtractionExpression 时才打印括号
  bool need_braces = dynamic_cast<SubtractionExpression *>(se->right);
  if (need_braces)
    oss << "(";
  se->left->accept(this);
  oss << "-";
  se->right->accept(this);
  if (need_braces)
    oss << ")";
}

// ExpressionEvaluator: 求值 DoubleExpression
void ExpressionEvaluator::visit(DoubleExpression *de) { result = de->value; }

// ExpressionEvaluator: 求值 AdditionExpression
void ExpressionEvaluator::visit(AdditionExpression *ae) {
  ae->left->accept(this);  // 递归求左值
  auto temp = result;       // 保存左值
  ae->right->accept(this); // 递归求右值
  result += temp;           // 累加
}

// ExpressionEvaluator: 求值 SubtractionExpression
void ExpressionEvaluator::visit(SubtractionExpression *se) {
  se->left->accept(this);  // 递归求左值
  auto temp = result;       // 保存左值
  se->right->accept(this); // 递归求右值
  result -= temp;           // 相减
}

// 客户端：演示双分派访问者模式
int main() {
  // 构建表达式：1 + (2 - 3)
  auto e = new AdditionExpression{
      new DoubleExpression{1},
      new SubtractionExpression{new DoubleExpression{2},
                                new DoubleExpression{3}}};

  ExpressionPrinter printer;
  ExpressionEvaluator evaluator;
  printer.visit(e);   // 打印表达式
  evaluator.visit(e); // 求值表达式
  cout << printer.str() << " = " << evaluator.result << endl; // 输出: 1+(2-3) = 0

  return 0;
}
