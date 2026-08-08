/*
 * =============================================================================
 * 设计模式：代理模式（Proxy Pattern）—— 属性代理
 * =============================================================================
 *
 * 【一句话概括】
 * 通过代理对象控制对原始属性的访问，在读写操作中插入额外行为。
 *
 * 【适用场景 —— 通用】
 * - 需要在属性访问时自动执行额外操作（日志记录、权限检查、值验证、变更通知等）
 *
 * 【金融工程应用】
 * - 风控参数变更通知：RiskLimit 的属性变更时（如止损比例从 5% 改为 3%），
 *   属性代理自动触发通知到风控引擎和审计日志
 * - 策略参数校验：Strategy 的参数（period、threshold）通过属性代理自动校验范围，
 *   如 period 必须在 [2, 200] 内，非法赋值自动拒绝并记录
 * - 实时持仓监控：Position 的 quantity 变更时，属性代理自动触发风险敞口重算
 *  和超限检查，确保持仓变更实时反映在风控系统中
 * - 行情数据校验：Quote 的 price/volume 通过属性代理自动校验合法性，
 *   过滤负价格或异常成交量
 *
 * 【本示例说明】
 * Property<T> 代理了类型 T 的值访问，通过 operator T() 拦截读操作，
 * operator= 拦截写操作。
 */

#include <iostream>
#include <string>
using namespace std;

// 属性代理模板类
// 包装一个值，拦截所有读写操作
template <typename T> struct Property {
  T value;

  // 构造函数：通过赋值操作符初始化，触发日志输出
  Property(T value) {
    // 使用 *this = value 调用赋值操作符，而非直接赋值
    *this = value;
  }

  // 隐式类型转换操作符：拦截读操作
  // 当将 Property<T> 隐式转换为 T 时调用（如 int x = creature.agility;）
  operator T() {
    cout << "Reading!" << endl;
    return value;
  }

  // 赋值操作符重载：拦截写操作
  // 当给 Property<T> 赋值时调用（如 creature.strength = 15;）
  T operator=(T new_value) {
    cout << "Assignment!" << endl;
    return value = new_value;
  }
};

// Creature 类使用 Property<int> 替代原始 int
// 外部接口完全不变，但自动获得了访问日志功能
struct Creature {
  Property<int> strength{10};
  Property<int> agility{10};
};

int main() {
  Creature c;
  c.strength = 15;     // 写操作 → 输出 "Assignment!"
  int x = c.agility;   // 读操作 → 输出 "Reading!"
  return x;
}
