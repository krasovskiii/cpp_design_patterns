/*
 * =============================================================================
 * SOLID 设计原则 - 开闭原则 (Open-Closed Principle, OCP)
 * =============================================================================
 *
 * 【一句话概括】
 * 软件实体（类、模块、函数等）应该对扩展开放，对修改关闭。
 * 即：应该通过添加新代码来扩展功能，而不是修改已有的代码。
 *
 * 【适用场景 —— 通用】
 * - 当需要为已有系统添加新功能，但不想触动经过测试的稳定代码
 * - 当系统中的过滤/排序/条件判断逻辑经常变化
 * - 构建可扩展的插件式架构时
 * - 规则引擎：核心引擎稳定不变，新业务规则通过添加规则类扩展
 * - 报表系统：基础报表框架不动，新报表类型通过新增报表类实现
 * - 日志处理管道：日志解析器不动，新增过滤器/转换器通过继承实现
 * - 编译器/解释器：词法分析器不动，新增语法节点类型通过 Visitor 模式扩展
 *
 * 【金融工程应用】
 * - 技术指标库扩展：指标计算引擎（IndicatorEngine）对修改关闭，通过继承 Indicator 基类
 *   新增任何技术指标（MACD、RSI、布林带、自定义指标）只需添加新的子类，引擎自动发现并计算
 *   新增"自适应均线"指标时，无需修改引擎代码，指标注册机制自动加载
 * 
 * - 策略信号生成：信号引擎依赖 ISignalGenerator 接口
 *   新增信号类型（突破信号/均值回归信号/波动率信号/机器学习信号）只需新增实现类
 *   核心信号分发和优先级逻辑无需变动，通过配置注入即可组合多种信号
 * 
 * - 风控规则系统：风控框架核心不动，具体规则通过 IRiskRule 接口扩展
 *   新增"科创板涨跌幅限制""ETF 申赎额度""期权希腊字母限制"只需新增规则类
 *   风控检查管道（Chain of Responsibility）自动纳入新规则
 * 
 * - 回测事件引擎：BarEvent、OrderEvent、FillEvent 等事件类型通过继承 Event 基类扩展
 *   新增 PortfolioEvent（组合更新事件）无需修改事件引擎的核心循环逻辑
 * 
 * - 定价模型：衍生品定价框架通过 IPricingModel 接口扩展
 *   新增"亚式期权定价""障碍期权定价""可转债定价"只需实现接口，框架通过配置加载
 *   估值系统（风险报表/P&L 归因）对新增定价模型完全透明
 * - 订单类型扩展：OMS 系统通过 IOrderType 接口扩展
 *   新增"冰山订单""条件订单""TWAP/VWAP 算法单"只需实现接口
 *   订单路由和撮合引擎对订单类型透明
 *
 * 【反例 / 不遵守的后果】
 * - 每次添加新功能都需要修改已有类，容易引入 bug
 * - 类的代码不断膨胀，变得难以维护
 * - 违反单一职责原则：一个类承担了所有可能的过滤逻辑
 * - 量化系统中：SignalGenerator 包含 if-else 判断所有信号类型，新增信号要修改核心类
 *   经过长期测试的核心逻辑被频繁修改，回测结果的可信度逐渐降低
 * - 测试成本指数级上升，每次改代码全量回归测试
 *
 * 【本例说明】
 * ProductFilter（坏方案）：每次新增过滤条件都要修改类本身
 * BetterFilter + Specification 模式（好方案）：
 *   新增过滤条件只需添加新的 Specification 子类，无需修改 BetterFilter
 */

#include <iostream>
#include <string>
#include <vector>

using namespace std;

// 产品属性枚举
enum class Color { red, green, blue };
enum class Size { small, medium, large };

// 产品结构体
struct Product {
  string name;
  Color color;
  Size size;
};

// ===========================================================================
// 方案一：不好的实现 —— 违反开闭原则
// ===========================================================================
// 问题1：每次新增过滤条件（如按名称、按价格），都必须修改 ProductFilter 类
// 问题2：组合过滤条件（如按颜色+尺寸）会形成笛卡尔积式的爆炸增长
struct ProductFilter {
  // 按颜色过滤
  // 参数用 const 引用避免按值拷贝整个 vector（Effective C++ 条款 20）
  vector<Product *> by_color(const vector<Product *> &items, Color color) {
    vector<Product *> result;
    for (auto &i : items)
      if (i->color == color)
        result.push_back(i);
    return result;
  }

  // 按颜色和尺寸同时过滤 —— 这只是一个组合，N个属性会产生2^N-1个函数！
  vector<Product *> by_size_and_color(const vector<Product *> &items,
                                      Size size, Color color) {
    vector<Product *> result;
    for (auto &i : items)
      if (i->size == size && i->color == color)
        result.push_back(i);
    return result;
  }
};

// ===========================================================================
// 方案二：好的实现 —— 使用 Specification 模式，遵守开闭原则
// ===========================================================================
// 核心思想：将过滤条件抽象为"规格（Specification）"接口，
// 每种具体条件作为独立的类实现，可以随意组合。

// 前向声明：用于 operator&& 的实现
template <typename T> struct AndSpecification;

// Specification 接口：判断某个元素是否满足条件
// is_satisfied 是只读判断，参数用 const T*（Effective C++ 条款 3：const 正确性）
template <typename T> struct Specification {
  virtual ~Specification() = default;
  virtual bool is_satisfied(const T *item) const = 0;
};

// Filter 接口：对集合应用某个规格进行过滤
template <typename T> struct Filter {
  // items 用 const 引用，避免按值拷贝整个 vector（Effective C++ 条款 20）
  virtual vector<const T *> filter(const vector<T *> &items,
                                   const Specification<T> &spec) = 0;
};

// BetterFilter 实现：不再关心具体的过滤条件，只负责遍历和判断
// 这是"对修改关闭"的关键 —— 无论新增多少过滤条件，这个类都不需要改变
struct BetterFilter : Filter<Product> {
  vector<const Product *> filter(
      const vector<Product *> &items,
      const Specification<Product> &spec) override {
    vector<const Product *> result;
    for (auto &item : items)
      if (spec.is_satisfied(item))   // 委托给 Specification 判断
        result.push_back(item);
    return result;
  }
};

// operator&& 重载：使两个 Specification 可以用 && 运算符组合
template <typename T>
AndSpecification<T> operator&&(const Specification<T> &first,
                               const Specification<T> &second) {
  return {first, second};
}

// ---------------------------------------------------------------------------
// 具体的 Specification 实现：每个过滤条件都是独立的类
// 扩展新条件时，只需新增一个类，无需修改任何已有代码
// ---------------------------------------------------------------------------

// 按颜色过滤的规格
struct ColorSpecification : Specification<Product> {
  Color color;
  ColorSpecification(Color color) : color(color) {}
  bool is_satisfied(const Product *item) const override {
    return item->color == color;
  }
};

// 按尺寸过滤的规格
struct SizeSpecification : Specification<Product> {
  Size size;
  SizeSpecification(Size size) : size(size) {}
  bool is_satisfied(const Product *item) const override {
    return item->size == size;
  }
};

// 组合规格：AND 逻辑（同时满足两个条件）
template <typename T> struct AndSpecification : Specification<T> {
  const Specification<T> &first;
  const Specification<T> &second;

  AndSpecification(const Specification<T> &first,
                   const Specification<T> &second)
      : first(first), second(second) {}

  bool is_satisfied(const T *item) const override {
    return first.is_satisfied(item) && second.is_satisfied(item);
  }
};

// ---------------------------------------------------------------------------
// 主函数：对比两种方案
// ---------------------------------------------------------------------------
int main() {
  Product apple{"Apple", Color::green, Size::small};
  Product tree{"Tree", Color::green, Size::large};
  Product house{"House", Color::blue, Size::large};

  const vector<Product *> items{&apple, &tree, &house};

  // ---- 坏方案：ProductFilter ----
  cout << endl << "ProductFilter (违反开闭原则):" << endl;
  ProductFilter pf;
  auto green_things = pf.by_color(items, Color::green);
  for (auto &item : green_things)
    cout << item->name << " is green\n";

  // ---- 好方案：BetterFilter + Specification ----
  cout << endl << "BetterFilter (遵守开闭原则):" << endl;
  BetterFilter bf;
  ColorSpecification green(Color::green);
  for (auto &item : bf.filter(items, green))
    cout << item->name << " is green\n";

  // 组合条件：绿色且大尺寸
  cout << endl << "And Specification (组合条件):" << endl;
  SizeSpecification large(Size::large);
  AndSpecification<Product> green_and_large(green, large);
  for (auto &item : bf.filter(items, green_and_large))
    cout << item->name << " is green and large\n";

  // 使用 && 运算符简化组合语法
  cout << endl << "使用 operator&& 简化组合:" << endl;
  auto spec = green && large;
  for (auto &item : bf.filter(items, spec))
    cout << item->name << " is green and large\n";

  return 0;
}
