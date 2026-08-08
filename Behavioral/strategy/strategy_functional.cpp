/*
 * =============================================================================
 * 设计模式：策略模式（Strategy）—— 现代 C++ 的 std::function 版（零抽象）
 * =============================================================================
 *
 * 【一句话概括】
 * 用 std::function 替代「策略接口 + 多个具体策略类」，实现策略切换而无需任何类层级。
 *
 * 【为什么要这个版本？——呼应 README 的「过度设计」章节】
 * 经典 Strategy 模式在策略数量少、仅需简单回调时，可能过度工程化：
 *   - 需要定义抽象接口 + N 个派生类 + 工厂来创建策略，样板代码多
 *   - 运行时多态引入虚函数调用开销，低延迟路径上不划算
 * 现代 C++ 中，策略往往只是一个「算法签名」：
 *   - 用 std::function 存下任意可调用对象（lambda、函数指针、函数对象）
 *   - 无需继承体系，组合性与可读性更强
 * 这正是「用 SOLID 而非堆 GoF」「性能路径保持简单」的体现。
 *
 * 【金融工程应用 —— 定价模型切换】
 * 传统做法：PricingStrategy 接口 + BSPricing、MonteCarloPricing、PDE 三个类。
 * 现代做法：直接用一个 `double(double spot)` 的可调用对象，用 std::function 装不同实现。
 *   - BSPricing / MonteCarloPricing 不需要有共同的基类，只需满足相同签名
 *   - 场景：欧式期权在 BS 解析解快但假设强，Monte Carlo 慢但灵活，运行时按品种/精度切换
 *
 * 【本例演示】
 * - Pricer：持有 std::function，可被任意可调用对象注入
 * - BSPricing / MonteCarloPricing：普通函数（甚至无需同类型）
 * - 任意切换策略，无需继承、无需虚函数
 */

#include <cmath>
#include <functional>
#include <iostream>
#include <string>

// 策略「签名」：给定标的价格与参数，返回期权价格
// 这就是 Strategy 模式的抽象核心，但它只是一个类型别名，而非接口类
using PricingStrategy = std::function<double(double spot, double strike, double t)>;

// 具体策略 1：Black-Scholes 解析解（普通函数，无需继承任何接口）
double bs_pricing(double spot, double strike, double t) {
  const double sigma = 0.2, r = 0.05;
  const double d1 =
      (std::log(spot / strike) + (r + 0.5 * sigma * sigma) * t) / (sigma * std::sqrt(t));
  const double d2 = d1 - sigma * std::sqrt(t);
  // 简化正态分布 CDF 近似（仅示例）
  const auto cdf = [](double x) { return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0))); };
  return spot * cdf(d1) - strike * std::exp(-r * t) * cdf(d2);
}

// 具体策略 2：Monte Carlo 价格（普通函数，与 bs_pricing 类型不同但签名兼容）
double mc_pricing(double spot, double strike, double t) {
  const double sigma = 0.2, r = 0.05;
  const int N = 100000;
  double sum = 0.0;
  for (int i = 0; i < N; ++i) {
    // Box-Muller 生成标准正态
    const double u1 = (i % 2 == 0) ? 0.1234 : 0.5678; // 演示用伪随机
    const double z = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u1);
    const double st = spot * std::exp((r - 0.5 * sigma * sigma) * t + sigma * std::sqrt(t) * z);
    sum += std::max(st - strike, 0.0);
  }
  return std::exp(-r * t) * (sum / N);
}

// 上下文（Context）：期权估值器，持有策略，运行时可切换
struct OptionPricer {
  // 注入任意策略（lambda、函数、函数对象皆可）
  void set_strategy(PricingStrategy s) { strategy_ = std::move(s); }

  // 执行当前策略定价
  double price(double spot, double strike, double t) const {
    return strategy_(spot, strike, t);
  }

private:
  PricingStrategy strategy_;
};

int main() {
  OptionPricer pricer;
  const double spot = 100.0, strike = 105.0, t = 1.0;

  // 运行时切换到 BS 解析解
  pricer.set_strategy(bs_pricing);
  std::cout << "BS price          : " << pricer.price(spot, strike, t) << std::endl;

  // 运行时切换到 Monte Carlo
  pricer.set_strategy(mc_pricing);
  std::cout << "Monte Carlo price : " << pricer.price(spot, strike, t) << std::endl;

  // 甚至可以直接用 lambda 作为一次性策略，无需任何命名类型
  pricer.set_strategy([](double s, double k, double) { return s * 0.9 - k; });
  std::cout << "Custom strategy   : " << pricer.price(spot, strike, t) << std::endl;

  return 0;
}
