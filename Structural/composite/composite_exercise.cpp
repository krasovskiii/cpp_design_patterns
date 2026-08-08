/*
 * =============================================================================
 * 设计模式：组合模式（Composite Pattern）—— 练习
 * =============================================================================
 *
 * 【一句话概括】
 * 将单个值和值的集合统一到同一接口下，使客户端可以用相同方式处理它们。
 *
 * 【适用场景】
 * - 需要将标量值（单值）和容器值（多值）统一求和或聚合时
 *
 * 【金融工程应用】
 * - 持仓聚合：SinglePosition（单品种持仓）和 PortfolioPosition（多品种持仓集合）
 *   统一计算总市值/总风险敞口/总保证金，sum() 对单个和集合一视同仁
 * - 盈亏归因：SinglePnL（单策略盈亏）和 CompositePnL（多策略盈亏组合）
 *   统一汇总，各层级的 sum() 递归聚合
 *
 * 【本示例说明】
 * ContainsIntegers 定义了 sum() 统一接口，SingleValue 是叶子节点，
 * ManyValues 是组合节点，全局 sum() 函数统一遍历求和。
 */

#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using namespace std;

// 抽象组件（Component）：包含整数的对象
// 定义统一的求和接口
class ContainsIntegers {
public:
  virtual int sum() const = 0;
};

// 叶子节点（Leaf）：单个整数值
// 包装单个 int，sum() 直接返回该值
class SingleValue : public ContainsIntegers {
  int value{0};

public:
  SingleValue() = default;

  explicit SingleValue(const int value) : value(value) {
    cout << "Created SingleValue(" << value << ")" << endl;
  }

  int sum() const override { return value; }
};

// 组合节点（Composite）：多个整数值的集合
// 继承 vector<int> 存储多个值，sum() 返回所有值的累加和
class ManyValues : public vector<int>, public ContainsIntegers {
public:
  ManyValues() { cout << "Created ManyValues" << endl; }

  void add(int value) {
    cout << "Adding (" << value << ") to others." << endl;
    push_back(value);
  }

  int sum() const override { return accumulate(begin(), end(), 0); }
};

// 客户端函数：统一处理单个值和多个值
// 通过 ContainsIntegers 接口，无需区分 SingleValue 和 ManyValues
int sum(const vector<ContainsIntegers *> items) {
  int total{0};
  for (auto item : items) {
    total += item->sum();
  }
  return total;
}

int main() {
  SingleValue single{1};
  ManyValues many;
  many.add(2);
  many.add(3);

  int total = sum({&single, &many});
  cout << "The sum is (" << total << ") and should be (6)" << endl;
  return 0;
}
