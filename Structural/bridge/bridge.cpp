/*
 * =============================================================================
 * 设计模式：桥接模式（Bridge Pattern）
 * =============================================================================
 *
 * 【一句话概括】
 * 将抽象部分与它的实现部分分离，使它们可以独立变化。
 *
 * 【适用场景 —— 通用】
 * - 需要避免抽象和实现的永久绑定，或需要在多个维度上独立扩展时
 * - 跨平台开发：GUI 控件（Button/Window）和平台渲染（Windows/macOS/Linux）独立扩展
 * - 设备驱动：设备抽象（Printer/Scanner）和厂商实现（HP/Canon/Epson）解耦
 *
 * 【金融工程应用】
 * - 策略与执行分离：策略（抽象）和订单执行器（实现）独立变化
 *   策略维度可扩展（趋势/套利/做市），执行维度可扩展（CTP/XTP/模拟撮合）
 *   N×M 的组合避免 N×M 的类爆炸，新增策略或执行器互不影响
 * - 定价模型与数值方法解耦：金融产品（期权/互换/结构化）和定价引擎（解析解/蒙特卡洛/PDE）
 *   独立扩展，新增产品类型不需要修改定价引擎，新增数值方法也不需要修改产品类
 * - 数据存储与数据格式解耦：TimeSeries（抽象）和 StorageBackend（实现）分离
 *   数据可以是 K 线/Tick/OrderBook，存储可以是 CSV/HDF5/数据库/内存
 * - 回测引擎与撮合模型解耦：BacktestEngine（抽象）和 MatchingEngine（实现）独立
 *   撮合模型可以是简单撮合/订单簿撮合/对手价撮合，引擎逻辑不变
 * - 报表生成与输出格式解耦：Report（抽象）和 Renderer（实现）分离
 *   报表类型（日报/周报/风控报告）和输出格式（PDF/HTML/Excel）独立扩展
 *
 * 【本示例说明】
 * Shape（形状）和 Renderer（渲染器）是两个独立变化的维度。
 * 如果不使用桥接，每种形状+渲染器组合都需要一个类，产生 2×2 的类爆炸。
 * 桥接模式通过将 Renderer 引用注入 Shape，避免了这种组合爆炸。
 */

#include <iostream>
#include <string>

using namespace std;

// ============================================================================
// 实现部分（Implementor）：渲染器
// 定义渲染的基本操作接口，与形状抽象完全解耦
// ============================================================================

// 渲染器抽象接口
struct Renderer {
  virtual void render_circle(float x, float y, float radius) = 0;
  virtual void render_square(float x, float y, float side) = 0;
};

// 具体实现A：光栅化渲染器（以像素方式绘制）
struct RasterRenderer : Renderer {
  void render_circle(float x, float y, float radius) override {
    cout << "Rasterizing a circle of radius " << radius << endl;
  }

  void render_square(float x, float y, float side) override {
    cout << "Rasterizing a square of side " << side << endl;
  }
};

// 具体实现B：矢量渲染器（以数学曲线方式绘制）
struct VectorRenderer : Renderer {
  void render_circle(float x, float y, float radius) override {
    cout << "Drawing a vector circle of radius " << radius << endl;
  }

  void render_square(float x, float y, float side) override {
    cout << "Drawing a vector square of side " << side << endl;
  }
};

// ============================================================================
// 抽象部分（Abstraction）：形状
// 持有对 Renderer（实现者）的引用，这是桥接的关键
// ============================================================================

struct Shape {
protected:
  // 桥接到渲染器 —— 形状与渲染方式解耦
  Renderer &renderer;
  Shape(Renderer &renderer) : renderer(renderer) {}

public:
  virtual void draw() = 0;        // 绘制形状
  virtual void resize(float factor) = 0; // 缩放形状
};

// 精化抽象A：圆形
struct Circle : Shape {
  Circle(Renderer &renderer, float x, float y, float radius)
      : Shape(renderer), x{x}, y{y}, radius{radius} {}

  float x, y, radius;

  void draw() override { renderer.render_circle(x, y, radius); }

  void resize(float factor) override { radius *= factor; }
};

// 精化抽象B：正方形
struct Square : Shape {
  Square(Renderer &renderer, float x, float y, float side)
      : Shape(renderer), x{x}, y{y}, side{side} {}

  float x, y, side;

  void draw() override { renderer.render_square(x, y, side); }

  void resize(float factor) override { side *= factor; }
};

// ============================================================================
// 客户端代码
// ============================================================================
int main() {
  // 光栅化渲染器 + 圆形
  RasterRenderer rr;
  Circle raster_circle{rr, 5, 5, 5};
  raster_circle.draw();
  raster_circle.resize(2);
  raster_circle.draw();

  // 矢量渲染器 + 正方形
  VectorRenderer vr;
  Square vector_square{vr, 0, 0, 4};
  vector_square.draw();
  vector_square.resize(3);
  vector_square.draw();
  return 0;
}
