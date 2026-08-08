/*
 * =============================================================================
 * 设计模式：组合模式（Composite Pattern）—— 数组-backed 属性
 * =============================================================================
 *
 * 【一句话概括】
 * 使用数组统一管理多个同质属性，通过枚举索引简化聚合操作（sum、average、max）。
 *
 * 【适用场景 —— 通用】
 * - 当一个类有多个同类型的属性，需要频繁进行批量聚合操作时
 *
 * 【金融工程应用】
 * - 多周期技术指标：SMA(5/10/20/60) 多个周期值用数组统一管理，
 *   sum/average/max 批量计算，枚举索引避免硬编码
 * - 多维度风险指标：VaR/CVaR/最大回撤/波动率统一存储在数组中，
 *   批量计算综合风险评分，新增指标只需加枚举值
 * - 希腊字母管理：Delta/Gamma/Theta/Vega/Rho 统一数组存储，
 *   sum() 一键计算总风险敞口
 *
 * 【本示例说明】
 * Creature 的 strength/agility/spirit/cunning 用数组存储，
 * sum()/average()/max() 可直接使用 STL 算法。
 */

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <string>

using namespace std;

class Creature {
private:
  // 传统做法：每个属性独立声明
  // int strength;
  // int agility;
  // int spirit;
  // int cunning;
  // 需要大量 getter/setter 和复杂的聚合逻辑

  // 数组-backed 属性方案：
  // 使用枚举定义属性索引，数组存储属性值
  // 'count' 自动反映属性数量，聚合操作无需修改
  //
  // 终端用户无需了解这种内部实现方式
  enum Abilities { strength, agility, spirit, cunning, count };
  array<int, count> abilities;

public:
  // ===== 各属性的 getter/setter（保持与传统接口一致） =====
  int get_strength() const { return abilities[strength]; }
  void set_strength(int value) { abilities[strength] = value; }

  int get_agility() const { return abilities[agility]; }
  void set_agility(int value) { abilities[agility] = value; }

  int get_spirit() const { return abilities[spirit]; }
  void set_spirit(int value) { abilities[spirit] = value; }

  int get_cunning() const { return abilities[cunning]; }
  void set_cunning(int value) { abilities[cunning] = value; }

  // ===== 聚合操作：利用数组结构简化实现 =====

  // 计算所有属性值之和
  int sum() const { return accumulate(abilities.begin(), abilities.end(), 0); }

  // 计算所有属性值的平均值
  double average() const { return sum() / (double)count; }

  // 获取所有属性值中的最大值
  int max() const { return *max_element(abilities.begin(), abilities.end()); }
};

int main() {
  Creature orc;
  orc.set_strength(16);
  orc.set_agility(10);
  orc.set_spirit(5);
  orc.set_cunning(5);

  cout << "Orc has a (" << orc.sum() << ") stat points, with average stat ("
       << orc.average() << ") and max value (" << orc.max() << ")" << endl;
  return 0;
}
