/*
 * =============================================================================
 * 设计模式：模板方法模式（Template Method）
 * =============================================================================
 *
 * 【一句话概括】
 * 在一个方法中定义算法的骨架，将某些步骤延迟到子类中实现。
 *
 * 【适用场景 —— 通用】
 * - 一次性实现算法的不变部分，将可变行为留给子类实现
 * - 子类之间的共同行为应被提取到基类中以避免代码重复
 * - 需要控制子类的扩展（只允许重写特定步骤）
 *
 * 【金融工程应用】
 * - 回测框架骨架：BacktestEngine 定义回测算法骨架（init→nextDay→onBar→onOrder→finish），
 *   具体策略只需重写 onBar() 和 onOrder()，框架管理数据加载/撮合/绩效计算
 * - 策略模板基类：Strategy 基类定义策略生命周期骨架（init→onStart→onBar→onStop→onFinish），
 *   子类只需实现信号生成逻辑，生命周期管理由基类统一处理
 * - 风险报告生成：ReportGenerator 定义报告生成流程（loadData→calcMetrics→formatReport→export），
 *   不同类型报告（日报/周报/风控报告）只需重写具体计算和格式化步骤
 * - 订单处理管道：OrderProcessor 定义处理流程（validate→enrich→route→confirm→log），
 *   不同交易所的处理器只需重写路由和确认步骤
 * - 数据导入管道：DataImporter 定义导入流程（connect→download→parse→validate→store），
 *   不同数据源（万得/彭博/交易所）只需重写下载和解析步骤
 *
 * 【关键参与者】
 *   - AbstractClass（抽象类）：Game，定义模板方法 run() 和纯虚函数步骤
 *   - ConcreteClass（具体类）：Chess，实现具体步骤
 */

#include <iostream>
#include <string>
using namespace std;

// 抽象类（AbstractClass）：定义游戏算法的通用骨架
class Game {
public:
  explicit Game(int number_of_players) : number_of_players(number_of_players) {}

  // 多态基类需要虚析构（Effective C++ 条款 7）
  virtual ~Game() = default;

  // 模板方法（Template Method）：定义游戏运行的算法骨架
  // run() 是普通成员函数（非虚），天然禁止子类重写整体流程；
  // 用 final 仅适用于虚函数，这里保持非虚即为最终算法（Effective C++ 条款 35）
  void run() {
    start();                         // 步骤1: 开始游戏
    while (!have_winner())           // 步骤2: 判断是否有赢家
      take_turn();                   // 步骤3: 执行回合
    cout << "Player " << get_winner() << " wins.\n"; // 步骤4: 宣布赢家
  }

protected:
  // 以下为模板方法（钩子方法/Hook Methods）——子类必须实现
  virtual void start() = 0;         // 开始游戏
  // 只读判断可以声明为 const（Effective C++ 条款 3：const 正确性）
  virtual bool have_winner() const = 0;   // 判断游戏是否结束
  virtual void take_turn() = 0;     // 执行一个回合
  virtual int get_winner() const = 0;     // 获取赢家编号

  int current_player{0};            // 当前玩家编号
  int number_of_players;            // 玩家总数
};

// 具体类（ConcreteClass）：国际象棋的具体实现
class Chess : public Game {
public:
  explicit Chess() : Game{2} {} // 国际象棋固定为 2 人游戏

protected:
  // 实现开始游戏：打印开始信息
  void start() override {
    cout << "Starting a game of chess with " << number_of_players
         << " players\n";
  }

  // 实现胜负判断：达到最大回合数则游戏结束
  bool have_winner() const override { return turns == max_turns; }

  // 实现回合逻辑：轮流下棋
  void take_turn() override {
    cout << "Turn " << turns << " taken by player " << current_player << "\n";
    turns++;
    current_player = (current_player + 1) % number_of_players; // 切换玩家
  }

  // 实现获取赢家：返回当前玩家
  int get_winner() const override { return current_player; }

private:
  int turns{0}, max_turns{10}; // 当前回合数和最大回合数
};

// 客户端：演示模板方法模式
int main() {
  Chess chess;
  chess.run(); // 调用模板方法，执行完整的游戏流程
  return 0;
}
