/*
 * =============================================================================
 * 设计模式：组合模式（Composite Pattern）—— 神经网络连接
 * =============================================================================
 *
 * 【一句话概括】
 * 通过统一接口处理单个对象和对象集合，使它们可以互换使用。
 *
 * 【适用场景 —— 通用】
 * - 需要对单个元素和元素集合执行相同操作时
 *
 * 【金融工程应用】
 * - 订单簿聚合：单个订单和订单集合（OrderBook）统一处理，计算总成交量/加权均价
 *   无论是修改单个订单还是批量操作订单簿，接口一致
 * - 行情快照集合：单个 Tick 和 Tick 序列统一处理，计算 VWAP/TWAP
 *   begin()/end() 让单个元素和集合都能在 range-for 中统一遍历
 * - 因子计算管道：单个因子和因子组合（因子树）统一处理数据流
 *   connect_to 模式用于因子间数据传递
 *
 * 【本示例说明】
 * Neuron（神经元）和 Layer（层）都需要支持 connect_to 操作，
 * CRTP 提供了统一的 connect_to 方法处理四种连接方式。
 */

#include <iostream>
#include <string>
#include <vector>

using namespace std;

// ============================================================================
// CRTP 基类：提供统一的连接操作
// Self 是派生类类型，通过 static_cast 将 this 转换为派生类指针
// ============================================================================
template <typename Self> struct Connection {
  // 将当前对象与另一个对象（可以是 Neuron 或 Layer）进行全连接
  // T 可以是 Neuron 或 Layer，只要它们支持 begin()/end() 迭代
  template <typename T> void connect_to(T &other) {
    for (Neuron &from : *static_cast<Self *>(this))
      for (Neuron &to : other) {
        from.out.push_back(&other);
        to.in.push_back(&from);
      }
  }
};

// ============================================================================
// 叶子节点：神经元
// 继承自 Connection<Neuron>，获得 connect_to 方法
// ============================================================================
struct Neuron : Connection<Neuron> {
  vector<Neuron *> in, out; // 输入和输出连接
  unsigned int id;

  Neuron() {
    static int id{1};
    this->id = id++;
  }

  // 使 Neuron 可迭代 —— 返回自身一次，使其在组合模式中表现为"单元素集合"
  Neuron *begin() { return this; }
  Neuron *end() { return this + 1; }

  // 打印神经元的所有连接信息
  friend ostream &operator<<(ostream &os, const Neuron &neuron) {
    for (auto n : neuron.in)
      os << n->id << "\t-->\t[" << neuron.id << "]" << endl;

    for (auto n : neuron.out)
      os << "[" << neuron.id << "]\t-->\t" << n->id << endl;

    return os;
  }
};

// ============================================================================
// 组合节点：层（Layer）
// 继承自 vector<Neuron>（天然是集合）和 Connection<Layer>（获得 connect_to 方法）
// ============================================================================
struct Layer : vector<Neuron>, Connection<Layer> {
  Layer(int count) {
    while (count-- > 0)
      emplace_back(Neuron{});
  }

  // 打印层中所有神经元的连接信息
  friend ostream &operator<<(ostream &os, const Layer &layer) {
    for (auto &n : layer)
      os << n;
    return os;
  }
};

int main() {
  // 创建神经元和层
  Neuron n1, n2;
  Layer l1{2}, l2{3};

  // 四种连接方式全部使用同一个 connect_to 接口
  n1.connect_to(n2); // 1. 神经元到神经元
  n1.connect_to(l1); // 2. 神经元到层
  l1.connect_to(l2); // 3. 层到层
  l2.connect_to(n2); // 4. 层到神经元

  cout << n1 << n2 << endl;
  cout << l1 << endl;
  return 0;
}
