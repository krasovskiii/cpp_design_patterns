/*
 * =============================================================================
 * 设计模式：装饰器模式（Decorator Pattern）—— 函数式装饰器
 * =============================================================================
 *
 * 【一句话概括】
 * 使用函数对象包装普通函数，在调用前后添加额外行为（如日志）。
 *
 * 【适用场景 —— 通用】
 * - 需要在不修改原函数的情况下添加日志、性能监控、权限检查等横切关注点
 *
 * 【金融工程应用】
 * - 交易函数计时：所有交易 API 调用（下单/撤单/查询）通过函数装饰器自动记录延迟，
 *   make_logger(send_order) 自动在调用前后记录时间戳，用于延迟分析和 SLA 监控
 * - 行情回调监控：onQuote 回调装饰自动记录处理延迟，当延迟超过阈值时告警
 * - 风控检查包装：checkRisk 函数装饰自动记录被拒绝的订单和原因，形成审计日志
 *
 * 【本示例说明】
 * 展示三种函数装饰器：Logger（std::function）、TemplateLogger（模板保留类型）、
 * Logger3（支持有参/有返回值函数）。
 */

#include <functional>
#include <iostream>

using namespace std;

///////////////////////////////////////////////////
// 方式一：简单函数装饰器（基于 std::function）
// 包装无参无返回值的函数，在调用前后打印日志
///////////////////////////////////////////////////
struct Logger {
  function<void()> func;
  string name;

  Logger(const function<void()> &func, const string &name)
      : func(func), name(name) {}

  // 重载 operator()，使装饰器对象可像函数一样调用
  void operator()() const {
    cout << "Entering " << name << endl;
    func();
    cout << "Exiting " << name << endl;
  }
};

///////////////////////////////////////////////////////
// 方式二：模板函数装饰器（保留具体函数类型）
// 相比 std::function，避免了类型擦除的开销
///////////////////////////////////////////////////////
template <typename Func> struct TemplateLogger {
  Func func;
  string name;

  TemplateLogger(const Func &func, const string &name)
      : func(func), name(name) {}

  void operator()() const {
    cout << "Entering " << name << endl;
    func();
    cout << "Exiting " << name << endl;
  }
};

// 辅助函数：利用模板参数推导，无需手动指定 Func 类型
template <typename Func>
auto make_template_logger(Func func, const string &name) {
  return TemplateLogger<Func>{func, name};
}

////////////////////////////////////////////////////////////
// 方式三：支持参数和返回值的模板函数装饰器
// 使用模板特化解析函数签名 R(Args...)
////////////////////////////////////////////////////////////

// 被装饰的原始函数：两个 double 相加
double add(double a, double b) {
  cout << a << "+" << b << " = " << (a + b) << endl;
  return a + b;
}

// 前向声明模板
template <typename> struct Logger3;

// 模板特化：解析 R(Args...) 函数签名
// R 是返回值类型，Args... 是参数类型包
template <typename R, typename... Args> struct Logger3<R(Args...)> {
  function<R(Args...)> func;
  string name;

  Logger3(const function<R(Args...)> &func, const string &name)
      : func(func), name(name) {}

  // 调用时转发所有参数，并返回原函数的返回值
  R operator()(Args... args) {
    cout << "Entering " << name << endl;
    R result = func(args...);
    cout << "Exiting " << name << endl;
    return result;
  }
};

// 辅助函数：利用函数指针推导 R 和 Args...
template <typename R, typename... Args>
auto make_logger3(R (*func)(Args...), const string &name) {
  return Logger3<R(Args...)>(function<R(Args...)>(func), name);
}

int main() {
  // 方式一：直接构造 Logger
  Logger([]() { cout << "Hello" << endl; }, "HelloFunction")();

  // 方式二：通过辅助函数构造模板装饰器
  auto logger =
      make_template_logger([]() { cout << "Hello" << endl; }, "HelloFunction");
  logger();

  // 方式三：装饰有参数和返回值的函数
  auto logged_add = make_logger3(add, "Add");
  auto result = logged_add(2, 3);
  cout << "result = " << result << endl;
}
