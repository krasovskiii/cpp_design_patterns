/*
 * =============================================================================
 * 设计模式：装饰器模式（Decorator Pattern）—— 静态装饰器（Mixin）
 * =============================================================================
 *
 * 【一句话概括】
 * 通过模板和 Mixin 继承在编译期组合功能，保留被装饰对象的具体类型接口。
 *
 * 【适用场景 —— 通用】
 * - 需要在编译期确定装饰组合，且需要保留被装饰对象所有接口时
 *
 * 【金融工程应用】
 * - 编译期策略组合：策略模板在编译期组合信号+风控+执行装饰层，
 *   零运行时开销（虚函数开销对高频交易不可接受），且保留具体策略类的特有方法
 * - 因子编译期组合：因子模板在编译期嵌套，RSI<Period<14>, PriceType<Close>>
 *   编译期展开所有计算逻辑，适合延迟敏感的实时因子计算
 *
 * 【本示例说明】
 * 使用模板参数 T 继承具体类型，编译期将装饰层"编织"到类型中，
 * 可调用 T 的所有方法（如 Circle::resize()），克服了动态装饰器的局限。
 */

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

// 抽象组件（Component）：形状
struct Shape {
  virtual string str() const = 0;
};

// 具体组件A：圆形
struct Circle : Shape {
  float radius;

  Circle(){};
  Circle(float radius) : radius(radius) {}

  void resize(float factor) { radius *= factor; }

  string str() const override {
    ostringstream oss;
    oss << "A circle of radius " << radius;
    return oss.str();
  }
};

// 具体组件B：正方形
struct Square : Shape {
  float side;

  Square(){};
  Square(float side) : side(side) {}

  void resize(float factor) { side *= factor; }

  string str() const override {
    ostringstream oss;
    oss << "A square of side " << side;
    return oss.str();
  }
};

///////////////////////////////
// 静态装饰器
// 使用 Mixin 继承（继承自模板参数 T）
///////////////////////////////

// 静态装饰器A：为形状添加颜色
// 继承自模板参数 T，保留 T 的所有接口
template <typename T> struct ColoredShape : T {
  // 编译期检查：确保 T 是 Shape 的子类
  static_assert(is_base_of<Shape, T>::value,
                "Template argument must be a Shape");

  string color;

  // 默认构造函数
  ColoredShape() {}

  // 可变参数模板构造函数：支持嵌套装饰器的参数转发
  // 例如 ColoredShape<Circle>("red", 10.0f)
  //      TransparentShape<ColoredShape<Square>>(51, "blue", 10)
  template <typename... Args>
  ColoredShape(const string &color, Args... args)
      : T(std::forward<Args>(args)...), color{color} {}

  // 重写 str()，附加颜色信息
  string str() const override {
    ostringstream oss;
    oss << T::str() << " has the color " << color;
    return oss.str();
  }
};

// 静态装饰器B：为形状添加透明度
template <typename T> struct TransparentShape : T {
  static_assert(is_base_of<Shape, T>::value,
                "Template argument must be a Shape");

  uint8_t transparency;

  TransparentShape() {}

  // 可变参数模板构造函数：支持嵌套装饰器的参数转发
  template <typename... Args>
  TransparentShape(const uint8_t &transparency, Args... args)
      : T(std::forward<Args>(args)...), transparency{transparency} {}

  // 重写 str()，附加透明度信息
  string str() const override {
    ostringstream oss;
    oss << T::str() << " has "
        << static_cast<float>(transparency) / 255.f * 100.f << "% transparency";
    return oss.str();
  }
};

int main() {
  Square square{8};

  // 静态装饰：编译期确定类型组合
  ColoredShape<Square> red_square{"red", 4};
  // 嵌套装饰：透明度装饰器包裹颜色装饰器，再包裹正方形
  TransparentShape<ColoredShape<Square>> transparent_and_red{51, "blue", 10};
  cout << square.str() << endl;
  cout << red_square.str() << endl;
  cout << transparent_and_red.str() << endl;

  Circle circle{10};
  ColoredShape<Circle> green_circle{"green", 7};
  cout << circle.str() << endl;
  cout << green_circle.str() << endl;
  // 静态装饰器的优势：可以调用子类特有方法 resize()
  green_circle.resize(10);
  cout << green_circle.str() << endl;

  return 0;
}
