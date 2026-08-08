/*
 * ===========================================================================
 * 设计模式：Inner Factory（内部工厂模式）
 * ===========================================================================
 *
 * 【核心思想】
 * 将工厂类嵌套定义在产品类的内部（inner class / nested class），
 * 同时提供一个静态的工厂单例供外部使用。这种方式将工厂与产品紧密绑定，
 * 同时提供了更简洁的 API。
 *
 * 【适用场景 —— 通用】
 * - 希望强制使用工厂，并且将工厂与产品类紧密关联
 * - 希望提供简洁的 API（如 Point::Factory.NewCartesian()）
 * - 需要控制工厂的实例化（通过静态单例暴露）
 * - 工厂逻辑与产品类高度内聚，不想分散到外部
 *
 * 【金融工程应用】
 * - 订单对象内部工厂：Order::Builder.NewLimit(sym, qty, price) 将订单工厂内嵌在订单类中
 *   Order::Builder.NewMarket(sym, qty)，工厂和产品的内聚性极强
 * - 金融时间序列构造：Series::Factory.FromCSV() / Series::Factory.FromArrow()
 *   工厂作为 Series 的内部类，提供静态单例访问点，API 简洁清晰
 * - 行情快照构造：Quote::Factory.FromTick() / Quote::Factory.FromSnapshot()
 *   不同的行情数据来源通过内部工厂统一创建，内聚性强
 *
 * 【UML 关键参与者】
 * - Product（产品）：Point —— 包含内部工厂类
 * - Factory（工厂）：Point::PointFactory —— 嵌套在 Point 内部的工厂类
 * - 工厂通过静态成员 Point::Factory 以单例形式暴露
 *
 * 【设计要点】
 * - Point 的构造函数是 private
 * - PointFactory 是 Point 的私有内部类
 * - Point::Factory 是一个 public 静态成员，作为工厂的唯一访问入口
 * - 这既强制了工厂的使用，又提供了简洁的 API
 *
 * 【API 对比】
 * - factory.cpp:       PointFactory::NewCartesian(x, y)
 * - factory_method.cpp: Point::NewCartesian(x, y)
 * - inner_factory.cpp:  Point::Factory.NewCartesian(x, y)
 *   最后一种方式最清晰地表达了"这是 Point 的工厂方法"
 */

#include <cmath>
#include <iostream>
#include <string>

using namespace std;

/*
 * 设计目标：强制使用工厂
 * - 隐藏构造函数
 * - 将工厂放在类内部
 */
class Point {
  float x, y;
  // 构造函数私有化
  Point(float x, float y) : x(x), y(y) {}

public:
  friend ostream &operator<<(ostream &os, const Point &point) {
    return os << "(x, y) = (" << point.x << ", " << point.y << ")";
  }

private:
  /*
   * PointFactory: 内部工厂类（嵌套类）
   *
   * 定义在 Point 的 private 区域，外部无法直接访问。
   * 只能通过 Point::Factory 静态成员来使用。
   */
  class PointFactory {
  public:
    static Point NewCartesian(float x, float y) { return {x, y}; }

    static Point NewPolar(float rho, float theta) {
      return {rho * cosf(theta), rho * sinf(theta)};
    }
  };

public:
  /*
   * 可选：将内部类设为 private，并提供这个静态单例。
   * 这使得 API 更加简洁 —— Point::Factory.NewCartesian()
   *
   * 静态成员，防止创建工厂的多个实例。
   */
  static PointFactory Factory;
};

int main() {
  // 方式一：如果内部类是 public 的，可以这样访问
  // auto p1 = Point::PointFactory::NewCartesian(0, 1);
  // auto p2 = Point::PointFactory::NewPolar(1, M_PI_4);

  // 方式二（推荐）：通过静态单例访问
  // 语法更简洁：Point::Factory.NewCartesian() vs Point::PointFactory::NewCartesian()
  auto p1 = Point::Factory.NewCartesian(0, 1);
  auto p2 = Point::Factory.NewPolar(1, M_PI_4);

  cout << p1 << endl;
  cout << p2 << endl;
  return 0;
}
