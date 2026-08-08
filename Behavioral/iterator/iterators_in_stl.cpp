/*
 * =============================================================================
 * 设计模式：迭代器模式（Iterator）—— STL 迭代器基础
 * =============================================================================
 *
 * 【一句话概括】
 * 提供一种方法顺序访问聚合对象中的各个元素，而不暴露其内部表示。
 *
 * 【适用场景】
 * - 需要遍历一个聚合对象而不想暴露其内部结构
 * - 需要为聚合对象提供多种遍历方式（正向/反向/const）
 *
 * 【金融工程应用】
 * - K 线序列遍历：使用迭代器遍历历史 K 线计算技术指标，
 *   前向迭代器用于实时计算，反向迭代器用于历史回测
 * - 行情快照遍历：通过 const_iterator 安全地遍历全市场行情快照，
 *   确保遍历过程中不修改数据
 *
 * 【关键参与者】
 *   - Iterator（迭代器）：vector<string>::iterator、const_reverse_iterator 等
 *   - Aggregate（聚合）：vector<string>
 */

#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main() {
  vector<string> names{"john", "jane", "jill", "jack"};

  // 获取指向第一个元素的迭代器
  vector<string>::iterator it = names.begin(); // 等价于 begin(names)
  cout << "first name is " << *it << "\n";     // 解引用迭代器获取元素值

  ++it; // 迭代器前进到下一个元素
  it->append(string(" goodall")); // 通过迭代器修改元素
  cout << "second name is " << *it << "\n";

  // 使用迭代器遍历剩余元素
  while (++it != names.end()) {
    cout << "another name: " << *it << "\n";
  }

  // 反向遍历整个 vector
  // 注意：使用全局 rbegin/rend，注意递增操作（不是递减）
  for (auto ri = rbegin(names); ri != rend(names); ++ri) {
    cout << *ri;
    if (ri + 1 != rend(names)) // 迭代器算术运算：判断是否还有下一个元素
      cout << ", ";
  }
  cout << endl;

  // 常量反向迭代器：不能通过它修改元素
  vector<string>::const_reverse_iterator jack = crbegin(names);
  cout << "first reverse name is " << *jack << "\n";
  // 以下代码无法编译通过：
  //*jack += "test";

  return 0;
}
