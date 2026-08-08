/*
 * =============================================================================
 * 设计模式：访问者模式（Visitor）—— 多重分派实现
 * =============================================================================
 *
 * 【一句话概括】
 * 通过 type_index + map 实现多重分派，根据多个对象的运行时类型决定行为。
 *
 * 【适用场景】
 * - 需要根据多个对象的运行时类型决定行为
 * - 希望避免复杂的访问者模式代码
 *
 * 【金融工程应用】
 * - 交易撮合分派：根据订单类型（限价/市价）× 对手方类型（零售/机构/做市商）
 *   分派到不同的撮合算法，多重分派表集中管理所有组合
 * - 多资产风控：根据资产类型（股票/期货/期权）× 风控类型（保证金/持仓/流动性）
 *   分派到不同的风控规则，避免复杂的 if-else 嵌套
 *
 * 【关键参与者】
 *   - GameObject（基类）：定义 type() 和 collide() 接口
 *   - GameObjectImpl<T>（CRTP 中间类）：使用 CRTP 返回具体类型的 type_index
 *   - outcomes map（分派表）：type_index 对 -> 处理函数
 */

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <typeindex>
using namespace std;

struct GameObject;
void collide(GameObject &first, GameObject &second);

// 游戏对象基类
struct GameObject {
  virtual ~GameObject() = default;
  virtual type_index type() const = 0; // 返回运行时类型信息

  // 多重分派（Multiple Dispatch）是必需的
  // 因为碰撞行为取决于两个对象的类型
  virtual void collide(GameObject &other) {
    // 委托给全局 collide 函数
    ::collide(*this, other);
  }
};

// CRTP 中间类：自动实现 type() 方法，返回具体的 type_index
template <typename T> struct GameObjectImpl : GameObject {
  type_index type() const override { return typeid(T); }
};

// 具体游戏对象层次结构（使用 CRTP）
struct Planet : GameObjectImpl<Planet> {};           // 行星
struct Asteroid : GameObjectImpl<Asteroid> {};       // 小行星
struct Spaceship : GameObjectImpl<Spaceship> {};     // 飞船

// 武装飞船（Spaceship 的子类）
struct ArmedSpaceship : Spaceship {
  type_index type() const override {
    return typeid(ArmedSpaceship); // 必须重写 type() 以返回正确的类型（模型限制）
  }
};

// 碰撞处理函数
void spaceship_planet() { cout << "spaceship lands on planet\n"; }
void asteroid_planet() { cout << "asteroid burns up in atmosphere\n"; }
void asteroid_spaceship() { cout << "asteroid hits and destroys spaceship\n"; }
void asteroid_armed_spaceship() { cout << "spaceship shoots asteroid\n"; }

// 类型对到碰撞函数的映射表
// 随着类型数量增加，映射表会迅速变得复杂
// 但这是实现多重分派的一种可行方式
map<pair<type_index, type_index>, void (*)(void)> outcomes{
    {{typeid(Spaceship), typeid(Planet)}, spaceship_planet},
    {{typeid(Asteroid), typeid(Planet)}, asteroid_planet},
    {{typeid(Asteroid), typeid(Spaceship)}, asteroid_spaceship},
    {{typeid(Asteroid), typeid(ArmedSpaceship)}, asteroid_armed_spaceship}};

// 全局碰撞函数：根据两个对象的类型查找碰撞处理函数
void collide(GameObject &first, GameObject &second) {
  // 在映射表中查找碰撞函数
  // 检查两种顺序：A-B 和 B-A
  auto it = outcomes.find({first.type(), second.type()});
  if (it == outcomes.end()) {
    it = outcomes.find({second.type(), first.type()}); // 尝试反向查找
    if (it == outcomes.end()) {
      cout << "objects pass each other harmlessly\n"; // 无定义的碰撞
      return;
    }
  }
  it->second(); // 调用碰撞处理函数
}

// 客户端：演示多重分派
int main() {
  ArmedSpaceship spaceship;
  Asteroid asteroid;
  Planet planet;

  collide(planet, spaceship);    // 飞船降落在行星上
  collide(planet, asteroid);     // 小行星在大气层中燃烧
  collide(spaceship, asteroid);  // 飞船射击小行星
  collide(planet, planet);       // 无定义的碰撞

  cout << "Member collision:\n";
  planet.collide(asteroid); // 通过成员函数调用碰撞（行星 + 小行星）

  // 但这不会正常工作：
  // 映射表中没有 (Spaceship, Planet) 的反向条目，
  // 只有 (Spaceship, Planet) 的正向条目，所以反向查找可能找不到
  spaceship.collide(planet); // 这在映射表中找不到 :/

  return 0;
}
