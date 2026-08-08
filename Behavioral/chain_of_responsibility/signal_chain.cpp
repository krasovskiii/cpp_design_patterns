/*
 * =============================================================================
 * 设计模式：责任链模式（Chain of Responsibility）—— 信号/槽变体
 * =============================================================================
 *
 * 【一句话概括】
 * 使用 Signal/Slot 机制实现更灵活的责任链，请求被广播给所有已连接的处理器。
 *
 * 【适用场景】
 * - 需要对一个对象的属性进行链式修改
 * - 需要动态添加/移除处理器，且处理器之间相互独立
 *
 * 【金融工程应用】
 * - 行情修正管道：原始行情信号广播给所有修正器（复权修正→除息修正→异常值修正），
 *   各修正器独立工作，通过 Signal 动态注册/注销
 * - 订单参数修饰链：订单创建后广播给所有修饰器（滑点调整→手续费计算→保证金估算），
 *   各修饰器独立计算自己的部分，互不干扰
 *
 * 【关键参与者】
 *   - Signal（信号）：模板类，管理槽函数列表，负责广播
 *   - Query（查询/请求）：封装被查询的属性信息
 *   - Game（中介者/协调者）：持有 Signal，充当中介角色
 *   - CreatureModifier（具体处理器）：连接信号并修改 Query 结果
 */

#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
using namespace std;

// 简单的信号/槽实现（替代 boost::signals2）
// 信号对象可以连接多个槽函数，调用时所有槽函数依次执行
template <typename... Args> class Signal {
public:
  using Slot = function<void(Args...)>; // 槽函数类型

  // 连接句柄：持有连接 ID 和信号指针，用于断开连接
  struct Connection {
    int id;
    Signal *signal;

    // 断开此连接
    void disconnect() const {
      if (signal)
        signal->disconnect(id);
    }
  };

  // 连接一个槽函数，返回连接句柄
  Connection connect(Slot slot) {
    int id = next_id_++;
    slots_[id] = move(slot);
    return Connection{id, this};
  }

  // 调用操作符：触发信号，依次执行所有已连接的槽函数
  void operator()(Args... args) const {
    for (const auto &[id, slot] : slots_) {
      slot(args...);
    }
  }

private:
  void disconnect(int id) { slots_.erase(id); } // 按 ID 断开连接

  int next_id_ = 0;
  unordered_map<int, Slot> slots_; // 使用 unordered_map 管理槽函数
};

// 查询对象：代表责任链中的一个请求
// 查询影响特定生物体（creature_name）的特定属性（argument）
struct Query {
  string creature_name;
  enum Argument { attack, defense } argument; // 查询的属性类型：攻击力或防御力
  int result; // 查询结果（会被链中的处理器修改）

  Query(const string &creature_name, const Argument argument, const int result)
      : creature_name(creature_name), argument(argument), result(result) {}
};

// 游戏类（同时扮演 Mediator 角色）：持有所有查询信号
struct Game {
  // 查询信号：当需要获取生物体属性时触发
  Signal<Query &> queries;
};

// 生物体类：拥有攻击力和防御力属性
class Creature {
  Game &game;
  int attack, defense;

public:
  string name;
  Creature(Game &game, const string &name, const int attack, const int defense)
      : game(game), attack(attack), defense(defense), name(name) {}

  // 获取攻击力：发起查询，让所有已注册的修饰器有机会修改结果
  int GetAttack() const {
    // 创建查询对象，以基础攻击力为初始值
    Query q{name, Query::Argument::attack, attack};
    game.queries(q); // 通过信号广播查询，所有连接的修饰器都会处理
    return q.result; // 返回被修饰后的最终结果
  }

  friend ostream &operator<<(ostream &os, const Creature &obj) {
    return os << "name: " << obj.name
              << " attack: " << obj.GetAttack() // 注意：这里会触发查询链
              << " defense: " << obj.defense;
  }
};

// 生物修饰器基类：持有游戏和生物体的引用
class CreatureModifier {
  Game &game;
  Creature &creature;

public:
  CreatureModifier(Game &game, Creature &creature)
      : game(game), creature(creature) {}

  virtual ~CreatureModifier() = default;
  // 注意：这里没有 handle() 函数，处理逻辑通过信号连接实现
};

// 具体修饰器：双倍攻击力
class DoubleAttackModifier : public CreatureModifier {
  // 保存信号连接，以便在析构时断开
  Signal<Query &>::Connection conn;

public:
  DoubleAttackModifier(Game &game, Creature &creature)
      : CreatureModifier(game, creature) {
    // 当任何人查询此生物体的攻击力时，返回双倍值
    conn = game.queries.connect([&](Query &q) {
      // 仅当查询匹配此生物体且查询的是攻击力属性时才修改
      if (q.creature_name == creature.name &&
          q.argument == Query::Argument::attack)
        q.result *= 2; // 将查询结果翻倍
    });
  }

  // 析构时自动断开信号连接，效果也随之移除
  ~DoubleAttackModifier() { conn.disconnect(); }
};

// 客户端演示
int main() {
  Game game;
  Creature goblin{game, "Strong Goblin", 2, 2};

  cout << goblin << endl; // 基础属性：攻击力 2

  {
    // 在作用域内注册双倍攻击修饰器
    DoubleAttackModifier dam{game, goblin};

    // 此时查询攻击力会触发修饰器，结果翻倍为 4
    cout << goblin << endl;
  }
  // dam 离开作用域被析构，信号自动断开，修饰效果消失

  cout << goblin << endl; // 恢复为基础攻击力 2

  return 0;
}
