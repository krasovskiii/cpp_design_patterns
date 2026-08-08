/*
 * =============================================================================
 * 设计模式：访问者模式（Visitor）—— 基于 std::variant 的现代 C++ 实现
 * =============================================================================
 *
 * 【一句话概括】
 * 使用 C++17 std::variant 和 std::visit 实现访问者模式，编译期保证类型安全。
 *
 * 【适用场景 —— 通用】
 * - 已知所有可能的类型变体（封闭集合）
 * - 希望在编译期保证所有类型都被处理
 * - 需要类型安全且高性能的访问者实现
 *
 * 【金融工程应用】
 * - 订单类型联合体：std::variant<LimitOrder, MarketOrder, StopOrder> 替代继承层次，
 *   std::visit 实现 Validate/Route/Log 操作，编译期保证所有订单类型都被处理
 * - 行情类型联合体：std::variant<Tick, Bar, OrderBook> 统一处理不同粒度行情，
 *   新增行情类型时编译器强制所有 visit 处更新
 * - 事件类型联合体：std::variant<BarEvent, OrderEvent, FillEvent, SignalEvent>
 *   在回测事件循环中使用 std::visit 分发处理，编译期检查完整性
 *
 * 【关键参与者】
 *   - std::variant（可辨识联合体）：替代传统 Element 层次结构
 *   - Visitor（访问者）：AddressPrinter，实现多个 operator() 重载
 *   - std::visit：自动根据 variant 中的类型调用对应的 operator()
 */

#include <iostream>
#include <string>
#include <variant>

using namespace std;

// 访问者（Visitor）：必须为 variant 中每种类型重载 operator()
struct AddressPrinter {
  // 处理 string 类型（房屋名称）
  void operator()(const string &house_name) const {
    cout << "A house called " << house_name << "\n";
  }

  // 处理 int 类型（门牌号）
  void operator()(const int house_number) const {
    cout << "House number " << house_number << "\n";
  }
};

int main() {

  // 创建访问者对象
  AddressPrinter ap;

  // 可辨识联合体（Visitable）：variant 表示房屋，可以通过名称或号码标识
  std::variant<string, int> house;

  // 设置房屋为名称类型
  house = "Montefiore Castle";
  std::visit(ap, house); // std::visit(访问者, 被访问对象)：自动分派到 string 版本的 operator()

  // 设置房屋为号码类型
  house = 221;
  std::visit(ap, house); // 自动分派到 int 版本的 operator()

  // 使用 lambda 作为内联访问者
  std::visit(
      [](auto &arg) {
        using T = decay_t<decltype(arg)>; // 获取 arg 的实际类型

        // 由于类型在编译期未知，需要使用 if constexpr 来分派
        if constexpr (is_same_v<T, string>) {
          cout << "A house called " << arg.c_str() << "\n";
        } else {
          cout << "House number " << arg << "\n";
        }
      },
      house);

  return 0;
}
