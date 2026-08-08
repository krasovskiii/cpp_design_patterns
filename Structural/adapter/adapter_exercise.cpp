/*
 * =============================================================================
 * 设计模式：适配器模式（Adapter Pattern）
 * =============================================================================
 *
 * 【一句话概括】
 * 将一个类的接口转换成客户端期望的另一个接口，使接口不兼容的类能够协同工作。
 *
 * 【适用场景 —— 通用】
 * - 当需要将一个类的功能集成到现有系统中，但该类的接口与系统期望的接口不一致时
 * - 复用遗留代码：旧系统 API 与新系统接口不兼容
 * - 第三方库封装：将第三方库的接口适配为项目内部统一接口
 *
 * 【金融工程应用】
 * - 行情数据格式适配：不同数据源（CTP/万得/彭博/交易所直连）的行情数据结构各不相同，
 *   通过适配器统一转为系统内部的 Quote 格式，策略层只需处理统一接口
 *   WindQuoteAdapter、CtpQuoteAdapter、BloombergQuoteAdapter → 统一 IQuote
 * - 交易接口适配：不同券商 API（CTP/飞马/易盛/XTP）下单接口参数格式各异，
 *   通过适配器统一转为内部 Order 格式，策略代码与具体券商 API 解耦
 * - 回测与实盘适配：实盘用 CTP 的 Bar 结构，回测用 CSV 读取的 Bar 结构，
 *   适配器统一转为策略使用的 InternalBar，实现回测到实盘的无缝切换
 * - 风控规则适配：不同交易所的风控参数格式不同（上期所用百分比、中金所用绝对值），
 *   适配器统一转为系统内部风控参数格式
 * - 遗留系统整合：将旧的 COM/DLL 交易接口适配为新的 gRPC 微服务接口，
 *   保护已有投资的同时逐步迁移到新架构
 *
 * 【本示例说明】
 * Square（正方形）和 Rectangle（矩形）接口不兼容 —— Square 只有 side（边长），
 * 而 Rectangle 接口要求 width() 和 height()。SquareToRectangleAdapter 将 Square
 * 适配为 Rectangle 接口，使得正方形可以像矩形一样使用，并计算出面积。
 */

#include <iostream>
using namespace std;

// 被适配者（Adaptee）：正方形
// 只存储边长 side，无法直接满足 Rectangle 的 width/height 接口
struct Square {
  int side{0};
  explicit Square(const int side) : side(side) {}
};

// 目标接口（Adapter Interface / Target）：矩形
// 客户端期望的接口，需要 width() 和 height() 两个维度
struct Rectangle {
  virtual int width() const = 0;
  virtual int height() const = 0;

  // 接口中可直接提供基于 width() 和 height() 的计算方法
  int area() const { return width() * height(); }
};

// 具体适配器（ConcreteAdapter）
// 通过组合（持有 Square 引用）将 Square 适配为 Rectangle 接口
// 正方形的 width 和 height 都映射到 side
struct SquareToRectangleAdapter : Rectangle {
  const Square &square;
  SquareToRectangleAdapter(const Square &square) : square(square) {}

  int width() const override { return square.side; }
  int height() const override { return square.side; }
};

int main() {
  // 客户端通过 Rectangle 接口使用 Square 对象，无需感知 Square 的存在
  Rectangle *r = new SquareToRectangleAdapter(Square(10));

  cout << "Rectangle w=" << r->width() << ", h=" << r->height() << "." << endl;
  return 0;
}
