/*
 * =============================================================================
 * 设计模式：组合模式（Composite Pattern）
 * =============================================================================
 *
 * 【一句话概括】
 * 将对象组织成树形结构，使客户端可以统一处理单个对象和组合对象。
 *
 * 【适用场景 —— 通用】
 * - 需要表示"部分-整体"的层次结构，且希望客户端忽略单个对象与组合对象的差异时
 * - UI 组件树：窗口包含面板，面板包含按钮/文本框，统一渲染
 * - 文件系统：目录包含文件和子目录，统一遍历/计算大小
 *
 * 【金融工程应用】
 * - 投资组合层次结构：Portfolio 包含子组合和单个资产，统一计算收益率/风险/VaR
 *   FundOfFunds → SubPortfolio → Asset，对任意层级调用 getReturn()/getRisk()
 * - 订单批量处理：BasketOrder 包含多个子订单，统一执行/撤销/查询状态
 *   调用 basket.execute() 递归执行所有子订单，失败时可统一回滚
 * - 因子树构建：复合因子（CompositeFactor）由子因子通过运算符（加/减/乘）组合
 *   RSIFactor + MACDFactor 形成信号树，统一 evaluate() 递归计算
 * - 风控规则组合：CompositeRule 包含多个子规则（AND/OR 逻辑）
 *   positionCheck AND stopLossCheck AND circuitBreaker，统一 check() 递归验证
 * - 账户体系：总账户包含子账户，子账户包含持仓，统一计算保证金/盈亏/风险敞口
 *
 * 【本示例说明】
 * GraphicObject 是抽象基类，Circle 是叶子节点，Group 是组合节点。
 * 通过将多个 Circle 和子 Group 添加到 root Group 中，形成树形结构。
 * 客户端调用 root.draw() 即可递归绘制整棵树。
 */

#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

// 抽象组件（Component）：图形对象
// 定义了叶子节点和组合节点的公共接口
struct GraphicObject {
  // 多态基类需要虚析构（Effective C++ 条款 7）
  virtual ~GraphicObject() = default;
  virtual void draw() = 0;
};

// 叶子节点（Leaf）：圆形
// 不包含子对象，实现具体的绘制操作
struct Circle : GraphicObject {
  void draw() override { cout << "Circle" << endl; }
};

// 组合节点（Composite）：组
// 可以包含任意数量的子 GraphicObject（包括其他 Group），形成递归树形结构
struct Group : GraphicObject {
  string name;
  vector<GraphicObject *> objects;

  // 用 std::move 转移所有权，避免多余拷贝（Effective C++ 条款 20）
  Group(string name) : name(std::move(name)) {}

  // 递归绘制：先打印组名，再遍历绘制所有子对象
  void draw() override {
    cout << "Group " << name << " contains { " << endl;  // string 已有 << 重载
    for (auto &&o : objects)
      o->draw();
    cout << "}" << endl;
  }
};

int main() {
  // 构建树形结构：
  // root
  //  |- c1 (Circle)
  //  |- subgroup
  //       |- c2 (Circle)
  Group root("root");
  Circle c1, c2;
  root.objects.push_back(&c1);

  Group subgroup("sub");
  subgroup.objects.push_back(&c2);

  root.objects.push_back(&subgroup);

  // 统一调用 draw()，递归绘制整棵树
  root.draw();
  return 0;
}
