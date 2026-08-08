/*
 * ===========================================================================
 * 设计模式：Prototype Exercise（原型模式练习）
 * ===========================================================================
 *
 * 【核心思想】
 * 使用序列化方式实现 Line 对象的深拷贝。Line 包含两个 Point 指针，
 * 通过 serialize/deserialize 机制实现通用的深拷贝，
 * 避免手动编写复杂的拷贝构造函数。
 *
 * 【适用场景】
 * - 对象包含多个指针成员，需要深拷贝
 * - 希望用序列化方式统一处理深拷贝逻辑
 * - 对象结构可能在未来扩展（加更多 Point 成员），序列化方式更容易维护
 *
 * 【金融工程应用】
 * - 投资组合深拷贝：组合包含多个资产权重、约束条件等复杂对象，
 *   序列化深拷贝比手动拷贝构造函数更可靠，避免遗漏嵌套对象的拷贝
 * - 回测状态序列化：回测过程中的策略状态（信号历史、持仓状态、缓存指标值）
 *   通过序列化保存到磁盘，中断后可恢复，也用于分布式回测的状态传递
 *
 * 【UML 关键参与者】
 * - Prototype（原型）：Line —— 通过 deep_copy() 提供克隆能力
 * - 组成对象：Point —— 实现序列化接口
 *
 * 【本例要点】
 * - Point 和 Line 都实现了 serialize/deserialize 接口
 * - Line::deep_copy() 将所有 Point 序列化到一个流，再反序列化创建副本
 * - 演示了深拷贝的正确行为：修改 l2 的 end 点不影响 l1
 */

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

/*
 * Point: 二维点
 *
 * 实现了手动文本序列化，支持深拷贝。
 * 格式：x y（空格分隔）
 */
struct Point {
  int x{0}, y{0};

  Point() {}

  Point(const int x, const int y) : x{x}, y{y} {}

  // 拷贝构造函数
  Point(const Point &p) : x(p.x), y(p.y) {}

  friend ostream &operator<<(ostream &os, const Point &point) {
    return os << "(" << point.x << ", " << point.y << ")";
  }

  // 序列化：将 x, y 写入输出流（空格分隔）
  void serialize(ostream &os) const { os << x << ' ' << y << '\n'; }

  // 反序列化：从输入流读取 x, y
  void deserialize(istream &is) {
    is >> x >> y;
    is.ignore(); // 消耗末尾的换行符
  }
};

/*
 * Line: 线段（原型模式练习）
 *
 * 包含两个 Point 指针：start 和 end。
 * 通过序列化实现深拷贝，避免手动管理拷贝构造函数的复杂性。
 */
struct Line {
  Point *start, *end;

  Line() : start(nullptr), end(nullptr) {}

  Line(Point *const start, Point *const end) : start(start), end(end) {}

  // 拷贝构造函数 —— 手动深拷贝方式（作为对比）
  Line(const Line &l) : start{new Point{*l.start}}, end{new Point{*l.end}} {}

  ~Line() {
    delete start;
    delete end;
  }

  /*
   * deep_copy(): 通过序列化实现深拷贝
   *
   * 工作流程：
   * 1. 将 start 和 end 两个 Point 序列化到 ostringstream
   * 2. 从 istringstream 反序列化创建两个新的 Point
   * 3. 用新的 Point 构造新的 Line
   *
   * 优势：即使未来 Line 增加了更多 Point 成员（如 control_point），
   * 只需在序列化和反序列化中添加对应代码，逻辑结构清晰。
   */
  Line deep_copy() const {
    // 序列化阶段：将 start 和 end 写入字符串流
    ostringstream oss;
    start->serialize(oss);
    end->serialize(oss);
    string s = oss.str();

    // 反序列化阶段：从字符串流恢复 Point 对象
    istringstream iss(s);
    auto p1 = new Point();
    auto p2 = new Point();
    p1->deserialize(iss);
    p2->deserialize(iss);
    return Line(p1, p2);
  }

  friend ostream &operator<<(ostream &os, const Line &line) {
    return os << "p1: " << *line.start << ", p2: " << *line.end;
  }
};

int main() {
  // 创建线段 l1: (0,0) -> (1,1)
  auto l1 = Line(new Point(0, 0), new Point(1, 1));

  // 深拷贝 l1 得到 l2
  auto l2 = l1.deep_copy();

  // 修改 l2 的终点
  l2.end->x = 90;
  l2.end->y = 100;

  // 验证：l1 不受 l2 修改的影响（深拷贝正确）
  cout << l1 << endl;  // (0,0) -> (1,1)  保持不变
  cout << l2 << endl;  // (0,0) -> (90,100)  已修改

  return 0;
}
