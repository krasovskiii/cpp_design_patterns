/*
 * =============================================================================
 * 设计模式：责任链模式（Chain of Responsibility）—— 指针链实现
 * =============================================================================
 *
 * 【一句话概括】
 * 将请求沿着一条处理链传递，每个处理者决定是自己处理还是传递给下一个处理者。
 *
 * 【适用场景 —— 通用】
 * - 多个对象可以处理同一请求，但处理者在运行时才确定
 * - 需要向多个对象之一提交请求而不明确指定接收者
 * - 处理请求的对象集合应动态指定
 * - Web 中间件管道：认证→授权→日志→限流→路由，每层可决定是否继续传递
 *
 * 【金融工程应用】
 * - 订单风控管道：订单进入系统后依次经过 资金检查→持仓限制→涨跌停限制→熔断检查→下单执行，
 *   每层检查失败则拒绝订单并终止传递，所有检查通过才执行
 * - 行情数据清洗管道：原始行情→异常值过滤→复权处理→缺失值填充→标准化输出，
 *   每层可独立开关（如回测时跳过异常值过滤）
 * - 信号确认管道：初始信号→趋势确认→成交量确认→波动率确认→最终信号，
 *   任一环节不满足条件则丢弃信号，避免单一指标误判
 * - 交易审批链：交易员→风控员→合规→主管，小额订单可能只需交易员确认，
 *   大额订单需要整条链审批，每层权限不同
 *
 * 【关键参与者】
 *   - Handler（处理者基类）：CreatureModifier，定义处理接口和链维护逻辑
 *   - ConcreteHandler（具体处理者）：DoubleAttackModifier、IncreaseDefenseModifier、NoBonusesCurseModifier
 *   - Client（客户端）：main() 中组装责任链
 */

#include <iostream>
#include <string>

using namespace std;

// 生物体：包含名称、攻击力、防御力三个属性
struct Creature {
  string name;
  int attack;
  int defense;

  Creature(const string &name, int attack, int defense)
      : name(name), attack(attack), defense(defense) {}

  // 输出生物体状态
  friend ostream &operator<<(ostream &os, const Creature &creature) {
    os << "name: " << creature.name << ", attack: " << creature.attack
       << ", defense: " << creature.defense;
    return os;
  }
};

// 生物修饰器基类（Handler）：责任链中的节点
class CreatureModifier {
  CreatureModifier *next{nullptr}; // 指向链中下一个修饰器的指针

protected:
  Creature &creature; // 被修饰的生物体引用

public:
  CreatureModifier(Creature &creature) : creature(creature) {}

  // 将修饰器添加到链表末尾（递归查找尾部）
  void add(CreatureModifier *cm) {
    if (next)
      next->add(cm); // 递归传递给下一个节点
    else
      next = cm; // 到达尾部，追加新节点
  }

  // 处理函数（虚函数，子类可重写）：默认行为是直接传递给下一个节点
  virtual void handle() {
    cout << creature << endl;
    if (next)
      next->handle(); // 调用链中下一个节点的 handle()
  }
};

// 具体处理者：双倍攻击力修饰器
class DoubleAttackModifier : public CreatureModifier {
public:
  DoubleAttackModifier(Creature &creature) : CreatureModifier(creature) {}

  void handle() override {
    creature.attack *= 2;        // 将攻击力翻倍
    CreatureModifier::handle();  // 调用基类 handle()，继续传递链
  }
};

// 具体处理者：增加防御力修饰器
class IncreaseDefenseModifier : public CreatureModifier {
public:
  IncreaseDefenseModifier(Creature &creature) : CreatureModifier(creature) {}

  void handle() override {
    if (creature.attack <= 2)    // 仅在攻击力较低时增加防御
      creature.defense++;
    CreatureModifier::handle();  // 继续传递链
  }
};

// 具体处理者：诅咒修饰器——阻断责任链
class NoBonusesCurseModifier : public CreatureModifier {
public:
  NoBonusesCurseModifier(Creature &creature) : CreatureModifier(creature) {}

  void handle() override {
    // 故意不调用 CreatureModifier::handle()
    // 从而中断责任链，后续所有修饰器都不会被执行
  }
};

// 客户端：组装并执行责任链
int main() {
  Creature goblin{"Goblin", 1, 1};
  CreatureModifier root{goblin};          // 根节点（不做任何修改）
  DoubleAttackModifier d1{goblin};        // 双倍攻击修饰器
  DoubleAttackModifier d2{goblin};        // 第二个双倍攻击修饰器
  DoubleAttackModifier d3{goblin};        // 第三个双倍攻击修饰器
  IncreaseDefenseModifier i1{goblin};     // 增加防御修饰器
  IncreaseDefenseModifier i2{goblin};     // 第二个增加防御修饰器
  NoBonusesCurseModifier n1{goblin};      // 诅咒修饰器（阻断链）

  // 构建责任链：root -> d1 -> i1 -> d2 -> i2 -> n1 -> d3
  // 注：因为 n1 在 d3 之前且不调用下一个，所以 d3 永远不会执行
  root.add(&d1); // a=2, d=1
  root.add(&i1); // a=2, d=2
  root.add(&d2); // a=4, d=2
  root.add(&i2); // a=4, d=2
  root.add(&n1); // a=4, d=2 （n1 阻断链，后续不会执行）
  root.add(&d3); // a=4, d=2 （永远不会被执行）
  root.handle(); // 从头开始执行整个链
  return 0;
}
