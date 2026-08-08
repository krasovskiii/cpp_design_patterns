/*
 * ===========================================================================
 * 设计模式：Factory Method（工厂方法模式）
 * ===========================================================================
 *
 * 【核心思想】
 * 将工厂方法定义在类自身内部（静态方法），而不是使用独立的工厂类。
 * 构造函数被设为 private，强制用户必须通过工厂方法来创建对象。
 * 这比独立的工厂类（factory.cpp）提供了更强的约束。
 *
 * 【适用场景 —— 通用】
 * - 需要强制用户通过工厂方法创建对象（隐藏构造函数）
 * - 同一类有不同语义的创建方式，且参数类型相同
 * - 希望将创建逻辑内聚在类内部，而不是分散到外部工厂类
 * - 需要验证/日志记录等创建前后的附加逻辑
 *
 * 【金融工程应用】
 * - 金融合约对象创建：Contract::fromISIN("CNE100003662") 从 ISIN 码创建，
 *   Contract::fromSymbol("510050.SH") 从交易代码创建，
 *   构造函数私有，强制使用语义化的工厂方法，确保合约数据来源清晰
 * - 时间序列创建：TimeSeries::fromCSV(path) vs TimeSeries::fromDatabase(conn, query)
 *   vs TimeSeries::fromAPI(url, token)，不同数据源构造方式不同，工厂方法名即文档
 * - 定价模型参数：PricingModel::BlackScholes(s, k, r, t, v) vs PricingModel::Heston(params)
 *   参数个数和类型差异大，构造函数私有化强制语义清晰
 * - 回测结果对象：BacktestResult::fromJSON(json) vs BacktestResult::fromProtobuf(data)
 *   不同序列化格式的反序列化逻辑不同，工厂方法封装转换细节
 *
 * 【UML 关键参与者】
 * - Product（产品）同时也是 Factory（工厂）：Point 类内部包含静态工厂方法
 * - 这是工厂方法的一种简化变体 —— Creator 和 Product 合二为一
 *
 * 【与 factory.cpp 的对比】
 * - factory.cpp：构造函数 public + 独立工厂类 → 工厂可选
 * - factory_method.cpp：构造函数 private + 内部工厂方法 → 工厂强制
 *
 * 【本例要点】
 * - 构造函数私有化：Point(float, float) 是 private
 * - 静态工厂方法作为公开 API：NewCartesian / NewPolar
 * - 这解决了三个问题：
 *   1. 不能有两个参数类型相同的构造函数（C++ 限制）
 *   2. 避免使用可选参数或类型标记（不优雅）
 *   3. 避免无意义的参数名称
 */

#include <cmath>
#include <iostream>
#include <string>

using namespace std;

/*
 * Point: 既是 Product 也是 Factory
 *
 * 我们想要创建直角坐标点和极坐标点，但是：
 * - 不能有两个参数类型相同的构造函数
 *     Point(float x, float y);       // 直角坐标
 *     Point(float rho, float theta); // 极坐标 —— 编译错误！签名相同
 * - 想避免使用可选参数（如 PointType 标记）
 *     Point(float a, float b, PointType type = PointType::cartesian);
 * - 想避免无意义的参数名
 *
 * 解决方案：隐藏构造函数，使用静态工厂方法
 */
class Point {
  // 构造函数私有化 —— 只有静态工厂方法可以创建 Point
  Point(float x, float y) : x(x), y(y) {}

public:
  float x, y;

  /*
   * 静态工厂方法：通过不同的方法名区分创建方式
   *
   * 这允许我们使用不同的逻辑生成 Point，而无需依赖构造函数重载。
   */
  // 通过直角坐标创建点
  static Point NewCartesian(float x, float y) { return {x, y}; }

  // 通过极坐标创建点（内部转换为直角坐标）
  static Point NewPolar(float rho, float theta) {
    return {rho * cosf(theta), rho * sinf(theta)};
  }

  friend ostream &operator<<(ostream &os, const Point &point) {
    return os << "(x, y) = (" << point.x << ", " << point.y << ")";
  }
};

int main() {
  // 必须通过工厂方法创建，不能直接使用构造函数
  // Point p(0, 1);  // 编译错误 —— 构造函数是 private

  auto p1 = Point::NewCartesian(0, 1);
  auto p2 = Point::NewPolar(1, M_PI_4);

  cout << p1 << endl;
  cout << p2 << endl;
  return 0;
}
