/*
 * =============================================================================
 * 设计模式：适配器模式（Adapter Pattern）—— 类适配器（继承方式）
 * =============================================================================
 *
 * 【一句话概括】
 * 通过继承将一个类的接口转换成客户端期望的另一个接口，使原本不兼容的类能够协同工作。
 *
 * 【适用场景 —— 通用】
 * - 需要复用现有的遗留类，但其接口与目标接口不兼容时
 * - 需要通过继承访问被适配类的 protected 成员
 *
 * 【金融工程应用】
 * - 遗留计算库适配：将旧的 C 风格期权定价库（greeks_calc(opt, result)）通过类适配器
 *   适配为 C++ 接口（opt.calcGreeks()），无需修改经过多年验证的定价算法
 * - 老旧风控模块封装：将 Fortran 编写的 VaR 计算模块通过类适配器包装为现代 C++ 接口，
 *   避免重写高复杂度的数学算法，同时融入新系统架构
 *
 * 【本示例说明】
 * RectangleAdapter 公开继承目标接口 Rectangle，同时私有继承被适配的 LegacyRectangle，
 * 将旧接口 oldDraw() 转换为新接口 draw()。
 */

#include <iostream>
#include <memory>

using namespace std;

// 坐标类型定义：用 using 而非 typedef（C++11 推荐，可读性更好）
using Coordinate = int;
using Dimension = int;

// 目标接口（Target / Adapter Interface）
// 客户端期望使用的接口：所有矩形都应支持 draw() 操作
class Rectangle {
public:
  // 多态基类需要虚析构（Effective C++ 条款 7）
  virtual ~Rectangle() = default;
  virtual void draw() = 0;
};

// 被适配者（Adaptee）
// 需要被复用的遗留组件，使用左上角和右下角坐标表示矩形
class LegacyRectangle {
public:
  LegacyRectangle(Coordinate x1, Coordinate y1, Coordinate x2, Coordinate y2)
      : x1_(x1), y1_(y1), x2_(x2), y2_(y2) {  // 用初始化列表替代赋值
    cout << "LegacyRectangle:  create.  (" << x1_ << "," << y1_ << ") => ("
         << x2_ << "," << y2_ << ")" << endl;
  }
  void oldDraw() {
    cout << "LegacyRectangle:  oldDraw.  (" << x1_ << "," << y1_ << ") => ("
         << x2_ << "," << y2_ << ")" << endl;
  }

private:
  Coordinate x1_;
  Coordinate y1_;
  Coordinate x2_;
  Coordinate y2_;
};

// 适配器（Adapter）
// - 公开继承目标接口 Rectangle，满足客户端对接口的期望
// - 私有继承被适配者 LegacyRectangle，复用其实现
// 将客户端传入的 (x, y, width, height) 参数转换为 LegacyRectangle 的 (x1,y1, x2,y2) 坐标
class RectangleAdapter : public Rectangle, private LegacyRectangle {
public:
  RectangleAdapter(Coordinate x, Coordinate y, Dimension w, Dimension h)
      : LegacyRectangle(x, y, x + w, y + h) {
    cout << "RectangleAdapter: create.  (" << x << "," << y
         << "), width = " << w << ", height = " << h << endl;
  }
  // 加 override（Effective C++ 条款 5）
  void draw() override {
    cout << "RectangleAdapter: draw." << endl;
    // 调用被适配者的旧方法完成实际绘制
    oldDraw();
  }
};

// 客户端代码
int main() {
  // 客户端只看到 Rectangle 接口，无需知道底层是 LegacyRectangle
  // 用 unique_ptr 管理，自动释放内存（Effective C++ 条款 13/21）
  unique_ptr<Rectangle> r = make_unique<RectangleAdapter>(120, 200, 60, 40);
  r->draw();
}
