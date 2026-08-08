/*
 * =============================================================================
 * 设计模式：中介者模式（Mediator）+ 观察者模式（Observer）—— 事件代理
 * =============================================================================
 *
 * 【一句话概括】
 * Game 作为中介者，通过 Signal/Slot 机制在 Player 和 Coach 之间传递事件。
 *
 * 【适用场景】
 * - 多个组件需要通过事件进行解耦通信
 * - 需要在不修改发送者或接收者代码的情况下添加新的事件处理者
 *
 * 【金融工程应用】
 * - 交易事件总线：TradeEventBus 作为中介者+观察者，策略发布信号事件，
 *   风控/执行/日志模块订阅各自关心的事件类型，完全解耦
 * - 行情事件分发：QuoteEventBus 接收交易所行情，分发给所有订阅者（策略/风控/显示/存储），
 *   新增订阅者无需修改行情接收模块
 *
 * 【关键参与者】
 *   - Mediator（中介者）：Game，持有 Signal<EventData*> 事件信号
 *   - Colleague（同事类/参与者）：Player（发送事件）、Coach（接收事件）
 *   - Signal/Slot：连接事件发送者和接收者的机制
 */

#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
struct Game;
using namespace std;

// 简单的信号/槽实现（替代 boost::signals2）
// 支持连接多个槽函数，调用时依次执行所有槽
template <typename... Args> class Signal {
public:
  using Slot = function<void(Args...)>;

  struct Connection {
    int id;
    Signal *signal;

    void disconnect() const {
      if (signal)
        signal->disconnect(id);
    }
  };

  Connection connect(Slot slot) {
    int id = next_id_++;
    slots_[id] = move(slot);
    return Connection{id, this};
  }

  void operator()(Args... args) const {
    for (const auto &[id, slot] : slots_) {
      slot(args...);
    }
  }

private:
  void disconnect(int id) { slots_.erase(id); }

  int next_id_ = 0;
  unordered_map<int, Slot> slots_;
};

// 事件数据基类：所有具体事件数据的抽象接口
struct EventData {
  virtual ~EventData() = default;
  virtual void print() const = 0; // 打印事件信息
};

struct Player;

// 具体事件数据：球员得分事件
struct PlayerScoredData : EventData {
  string player_name;
  int goals_scored_so_far; // 当前总进球数

  PlayerScoredData(const string &player_name, const int goals_scored_so_far)
      : player_name(player_name), goals_scored_so_far(goals_scored_so_far) {}

  void print() const override {
    cout << player_name << " has scored! (their " << goals_scored_so_far
         << " goal)"
         << "\n";
  }
};

// 中介者（MEDIATOR）：游戏对象
// 持有事件信号，协调 Player 和 Coach 之间的通信
struct Game {
  Signal<EventData *> events; // 事件信号（同时扮演 Observer 的角色）
};

// 参与者（PARTICIPANT）：发送信号的球员
struct Player {
  string name;
  int goals_scored = 0;
  Game &game; // 引用中介者

  Player(const string &name, Game &game) : name(name), game(game) {}

  // 进球操作：增加进球数并通过中介者发送事件信号
  void score() {
    goals_scored++;
    PlayerScoredData ps{name, goals_scored};
    game.events(&ps); // 通过中介者发送得分事件信号
  }
};

// 参与者（PARTICIPANT）：订阅信号的教练
struct Coach {
  Game &game;

  explicit Coach(Game &game) : game(game) {
    // 教练订阅进球事件：如果球员进球数少于 3，则庆祝
    game.events.connect([](EventData *e) {
      // 动态类型转换，判断事件类型是否为 PlayerScoredData
      PlayerScoredData *ps = dynamic_cast<PlayerScoredData *>(e);
      if (ps && ps->goals_scored_so_far < 3) {
        cout << "coach says: well done, " << ps->player_name << "\n";
      }
    });
  }
};

// 客户端：演示中介者+观察者模式
int main() {
  Game game;                         // 创建中介者
  Player player{"Sam", game};        // 创建球员
  Coach coach{game};                 // 创建教练（自动订阅事件）

  player.score(); // 第 1 个进球：教练庆祝
  player.score(); // 第 2 个进球：教练庆祝
  player.score(); // 第 3 个进球：教练不再庆祝（进球数 >= 3）

  return 0;
}
