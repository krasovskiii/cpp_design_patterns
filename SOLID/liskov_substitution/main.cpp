/*
 * =============================================================================
 * SOLID 设计原则 - 里氏替换原则 (Liskov Substitution Principle, LSP)
 * =============================================================================
 *
 * 【一句话概括】
 * 子类对象应该能够替换父类对象，而程序的行为不发生变化。
 * 即：如果 S 是 T 的子类型，那么 T 的对象可以被 S 的对象替换，而程序正确性不变。
 *
 * 【适用场景 —— 通用】
 * - 设计继承层次时，需要确保子类不破坏父类的契约
 * - 当发现子类需要覆盖父类方法并改变其预期行为时，可能继承关系不合适
 * - 单元测试：子类的 mock/stub 对象替换真实对象时，行为必须一致
 * - 容器/集合类继承：自定义 List 继承标准容器时，确保迭代器、增删操作的语义一致
 * - 多态替换：通过基类指针/引用操作派生类对象时，程序行为可预测
 *
 * 【金融工程应用】
 * - 金融产品继承陷阱：看似合理的继承（期货是衍生品→期权是衍生品）可能破坏 LSP
 *   错误设计：class Option : public Future { ... }  —— 期权的行权逻辑破坏了期货的结算契约
 *   正确设计：两者都继承自 abstract Derivative，分别实现各自的 settlement() 契约
 * - 订单类型层次：LimitOrder 和 MarketOrder 继承自 Order 基类
 *   Order.validate() 的契约是"验证订单参数"，MarketOrder 没有 price 字段，
 *   如果基类 validate() 假设 price > 0，则 MarketOrder 破坏了 LSP
 *   修正：基类不包含 price 相关约束，由各自子类定义自己的验证契约
 * - 收益率曲线插值：LinearInterpolation 和 CubicSplineInterpolation 继承自 YieldCurve
 *   YieldCurve.discount(t) 的契约是"返回 [0, 1] 之间的折现因子"
 *   如果 CubicSpline 在尾部出现 overshoot 导致 discount > 1，则违反了 LSP
 *   修正：基类定义的后置条件必须被所有子类满足，或在子类中对异常值做 clamp
 * - 回测引擎中的 SlippageModel：FixedSlippage(0.01) 和 ProportionalSlippage(0.001)
 *   替换后，getSlippage() 的行为语义一致（返回滑点值），但比例模型在不同价格下行为不同
 *   只要契约是"返回一个非负滑点"，两者都满足 LSP；如果契约隐含"与价格无关"，则违反
 * - 数据源替换：LiveDataSource 和 HistoricalDataSource 继承自 IDataSource
 *   IDataSource.getBars(symbol, from, to) 的契约是"返回指定时间区间内的 K 线"
 *   实时源可能因为网络中断返回不完整数据，历史源返回完整数据
 *   必须在契约中明确：允许返回部分数据（用返回值或标志位标明数据完整性）
 *
 * 【反例 / 不遵守的后果】
 * - 看似合理的继承（正方形是矩形）实际上破坏了父类的行为假设
 * - 使用多态时出现意外行为，难以调试
 * - 不得不使用类型检查（if isinstance）来规避问题
 * - 金融系统中：回测策略使用 LiveDataSource 验证通过，切换到 HistoricalDataSource 后因数据完整性问题
 *   策略表现不一致，却难以定位是数据问题还是策略问题
 *
 * 【本例说明】
 * 数学上"正方形是矩形"，但在编程中：
 * - Rectangle.setWidth() 和 setHeight() 是独立操作
 * - Square 的 setWidth() 同时改变了 width 和 height
 * - 使用 Rectangle 引用操作 Square 时，行为不符合预期
 * 这个例子展示了为什么有时"is-a"关系在代码层面不成立
 */

#include <iostream>
#include <string>
#include <vector>

using namespace std;

// ---------------------------------------------------------------------------
// Rectangle 类：宽和高可以独立设置
// ---------------------------------------------------------------------------
class Rectangle {
protected:
  int width, height;    // protected 允许子类直接访问

public:
  Rectangle(int width, int height) : width(width), height(height) {}

  // 多态基类必须有虚析构（Effective C++ 条款 7），否则经基类指针删除
  // 派生类是未定义行为
  virtual ~Rectangle() = default;

  int getWidth() const { return width; }

  // 虚函数：允许子类覆盖设置行为
  virtual void setWidth(int width) { Rectangle::width = width; }

  int getHeight() const { return height; }

  virtual void setHeight(int height) { Rectangle::height = height; }

  // 计算面积 —— 对于 Rectangle，正确行为是 width * height
  int area() const { return width * height; }
};

// ---------------------------------------------------------------------------
// Square 类：继承自 Rectangle，但破坏了父类的行为契约
// ---------------------------------------------------------------------------
// 问题：Square.setWidth(w) 会同时修改 height，这违反了 Rectangle 的预期行为
// 对于 Rectangle：setWidth(5); setHeight(10); → width=5, height=10, area=50
// 对于 Square：  setWidth(5); setHeight(10); → width=10, height=10, area=100
// 两者行为不一致！
class Square : public Rectangle {
public:
  Square(int size) : Rectangle(size, size) {}

  // 设置宽度的同时也改变高度 —— 保持正方形的约束
  void setWidth(int width) override { this->width = this->height = width; }

  // 设置高度的同时也改变宽度 —— 保持正方形的约束
  void setHeight(int height) override { this->width = this->height = height; }
};

// ---------------------------------------------------------------------------
// process 函数：演示 LSP 违反的后果
// 此函数接受 Rectangle 引用，并假设 setHeight 不会改变 width
// 当传入 Square 时，这个假设被破坏，导致意外结果
// ---------------------------------------------------------------------------
void process(Rectangle &r) {
  int w = r.getWidth();       // 记录当前宽度
  r.setHeight(10);             // 设置高度（假设不改变宽度）

  // 预期面积 = 原宽度 * 10，但 Square 的实际结果不同
  cout << "expected area = " << (w * 10)
       << ", got " << r.area() << endl;
}

// ---------------------------------------------------------------------------
// 主函数
// ---------------------------------------------------------------------------
int main() {
  // Rectangle 行为符合预期
  Rectangle r{3, 4};
  process(r);   // expected area = 30, got 30  ✓

  // Square 破坏了预期行为！
  Square sq{5};
  process(sq);  // expected area = 50, got 100  ✗

  return 0;
}
