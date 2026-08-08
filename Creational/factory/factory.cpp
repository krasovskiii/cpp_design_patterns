/*
 * ===========================================================================
 * 设计模式：Factory（工厂模式 / 简单工厂）
 * ===========================================================================
 *
 * 【核心思想】
 * 使用一个独立的工厂类（或函数）来创建对象，将对象的创建逻辑与使用逻辑分离。
 * 当构造函数无法通过名称区分不同的创建方式时（如直角坐标 vs 极坐标），
 * 工厂方法可以用有意义的名称来区分不同的创建语义。
 *
 * 【适用场景 —— 通用】
 * - 同一个类有多种不同的构造方式，但构造函数签名相同（无法重载区分）
 * - 对象的创建逻辑比较复杂，不适合放在构造函数中
 * - 希望给不同的创建方式赋予有意义的名称（如 NewCartesian / NewPolar）
 * - 对象创建需要根据配置或上下文选择不同的初始化参数
 * - 第三方库封装：隐藏复杂库对象的创建细节，提供简洁的工厂 API
 *
 * 【金融工程应用】
 * - 金融产品创建工厂：同一个 Product 类，根据产品类型不同需要不同的构造逻辑
 *   ProductFactory::createSwap(fixed, floating, notional, tenor)
 *   ProductFactory::createOption(underlying, strike, expiry, type)
 *   区分不同产品的创建语义，避免巨型构造函数
 * - K 线数据构造：BarFactory::fromTick(tickData, interval) vs BarFactory::fromMinute(minData, interval)
 *   不同数据源（Tick/分钟线）合成 K 线的方式不同，工厂方法用名称表达语义差异
 * - 订单创建工厂：OrderFactory::createLimit(sym, qty, price) vs OrderFactory::createMarket(sym, qty)
 *   vs OrderFactory::createStop(sym, qty, triggerPrice)，参数类型相似但语义完全不同
 * - 信号创建工厂：SignalFactory::createMACD(fast, slow, signal) vs SignalFactory::createRSI(period)
 *   不同技术指标参数不同，工厂统一管理创建，避免客户端记忆参数顺序
 *
 * 【UML 关键参与者】
 * - Product（产品）：Point —— 被创建的对象
 * - Factory（工厂）：PointFactory —— 包含静态工厂方法
 * - 注意：这是"简单工厂"模式，工厂不是抽象类，而是具体类
 *
 * 【本例要点】
 * - Point 的构造函数仍然是 public 的 —— 用户可以选择直接构造或使用工厂
 * - 工厂是可选的（optional），没有强制用户使用
 * - PointFactory 提供两种语义清晰的创建方式：
 *   - NewCartesian(x, y)：通过直角坐标创建点
 *   - NewPolar(rho, theta)：通过极坐标创建点
 * - 两种创建方式的参数类型相同（都是两个 float），因此无法通过构造函数重载区分
 */

#include <cmath>
#include <iostream>
#include <string>

using namespace std;

/*
 * Point: Product（产品）角色
 *
 * 一个二维点，使用直角坐标 (x, y) 存储。
 * 构造函数是 public 的 —— 工厂是可选的，不强制使用。
 */
class Point {
  float x, y;

public:
  Point(float x, float y) : x(x), y(y) {}

  friend ostream &operator<<(ostream &os, const Point &point) {
    return os << "(x, y) = (" << point.x << ", " << point.y << ")";
  }
};

/*
 * PointFactory: 简单工厂
 *
 * 这个工厂实现对用户来说是可选的 —— 用户可以使用公开的构造函数，
 * 也可以使用工厂方法。
 * 没有强制的机制来迫使使用者必须使用工厂。
 *
 * 静态工厂方法的优势：
 * 1. 方法名具有语义：NewCartesian / NewPolar 比 Point(x,y) 更清晰
 * 2. 解决了"两个构造函数参数类型相同但语义不同"的问题
 * 3. NewPolar 内部完成极坐标到直角坐标的转换
 */
struct PointFactory {
  // 通过直角坐标创建点
  static Point NewCartesian(float x, float y) { return {x, y}; }

  // 通过极坐标创建点（内部转换为直角坐标）
  static Point NewPolar(float rho, float theta) {
    return {rho * cosf(theta), rho * sinf(theta)};
  }
};

int main() {
  // 使用工厂方法创建直角坐标点
  auto p1 = PointFactory::NewCartesian(0, 1);
  // 使用工厂方法创建极坐标点（PI/4 = 45度，半径=1）
  auto p2 = PointFactory::NewPolar(1, M_PI_4);

  cout << p1 << endl;
  cout << p2 << endl;
  return 0;
}
