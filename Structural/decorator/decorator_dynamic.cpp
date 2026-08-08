/*
 * =============================================================================
 * 设计模式：装饰器模式（Decorator Pattern）—— 动态装饰器
 * =============================================================================
 *
 * 【一句话概括】
 * 在不修改原有类的基础上，通过组合方式动态地为对象添加新功能。
 *
 * 【适用场景 —— 通用】
 * - 需要在运行时灵活地为对象添加额外行为，且不希望创建大量子类时
 * - 日志/缓存/权限检查等横切关注点的动态添加
 *
 * 【金融工程应用】
 * - 订单装饰链：基础 Order → LoggingDecorator（记录日志）→ ValidationDecorator（参数校验）
 *   → RiskCheckDecorator（风控检查），运行时动态组合装饰层，各层独立可测
 * - 行情数据装饰：原始 Quote → NormalizedDecorator（标准化）→ FilteredDecorator（去噪）
 *   → AdjustedDecorator（复权），每层装饰独立开发，可自由组合
 * - 策略信号装饰：原始 Signal → ConfidenceDecorator（置信度评分）→ PositionSizeDecorator（仓位计算）
 *   → RiskLimitDecorator（风险限制），运行时按需组合
 * - 回测结果装饰：基础 Result → SharpeDecorator（夏普比率）→ DrawdownDecorator（最大回撤）
 *   → BenchmarkDecorator（基准对比），动态选择需要计算的绩效指标
 *
 * 【本示例说明】
 * ColoredShape 为形状添加颜色，TransparentShape 添加透明度，
 * 可嵌套使用：TransparentShape(ColoredShape(Square))。
 * 局限：只能访问基类 Shape 接口，无法调用子类特有方法。
 */

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

// 抽象组件（Component）：形状
// 定义所有具体组件和装饰器的公共接口
struct Shape {
  // 多态基类需要虚析构（Effective C++ 条款 7）
  virtual ~Shape() = default;
  virtual string str() const = 0;
};

// 具体组件A（Concrete Component）：圆形
struct Circle : Shape {
  float radius;

  // 空构造函数用 = default 替代，语义更清晰（Effective C++ 条款 12）
  Circle() = default;
  Circle(float radius) : radius(radius) {}

  // 子类特有方法 —— 装饰器无法直接调用
  void resize(float factor) { radius *= factor; }

  string str() const override {
    ostringstream oss;
    oss << "A circle of radius " << radius;
    return oss.str();
  }
};

// 具体组件B（Concrete Component）：正方形
struct Square : Shape {
  float side;

  Square() = default;
  Square(float side) : side(side) {}

  // 子类特有方法 —— 装饰器无法直接调用
  void resize(float factor) { side *= factor; }

  string str() const override {
    ostringstream oss;
    oss << "A square of side " << side;
    return oss.str();
  }
};

// 动态装饰器A：为形状添加颜色
// 持有 Shape 引用（组合方式），在 str() 中附加颜色信息
struct ColoredShape : Shape {
  Shape &shape;
  string color;

  ColoredShape(Shape &shape, const string &color)
      : shape(shape), color(color) {}

  string str() const override {
    ostringstream oss;
    oss << shape.str() << " has the color " << color;
    return oss.str();
  }
};

// 动态装饰器B：为形状添加透明度
// 同样持有 Shape 引用，可以嵌套在 ColoredShape 外层
struct TransparentShape : Shape {
  Shape &shape;
  uint8_t transparency;

  TransparentShape(Shape &shape, uint8_t transparency)
      : shape(shape), transparency(transparency) {}

  string str() const override {
    ostringstream oss;
    oss << shape.str() << " has "
        << static_cast<float>(transparency) / 255.f * 100.f << "% transparency";
    return oss.str();
  }
};

int main() {
  Square square{8};
  ColoredShape red_sq{square, "red"};
  // 嵌套装饰：先加颜色，再加透明度
  TransparentShape tr_and_red{red_sq, 51};
  cout << square.str() << endl;
  cout << red_sq.str() << endl;
  cout << tr_and_red.str() << endl;

  // 局限：无法使用被装饰对象的具体类接口
  // 只能访问基类 Shape 的接口
  Circle circle{10};
  ColoredShape green_circle{circle, "green"};
  // 以下代码无法编译 —— ColoredShape 只暴露 Shape 接口
  // green_circle.resize(10);

  return 0;
}
