/*
 * =============================================================================
 * 设计模式：迭代器模式（Iterator）—— 自定义前向迭代器
 * =============================================================================
 *
 * 【一句话概括】
 * 为单链表实现自定义的前向迭代器，使其兼容 STL 算法。
 *
 * 【适用场景】
 * - 需要为自定义容器实现 STL 兼容的迭代器
 * - 需要与标准库算法（如 for_each）配合使用
 *
 * 【金融工程应用】
 * - 自定义队列迭代器：为订单队列实现迭代器，支持 STL 算法（find_if 查找特定订单、
 *   count_if 统计符合条件的订单数）
 * - 持仓列表迭代器：为持仓链表实现迭代器，支持标准库的排序和筛选算法
 *
 * 【关键参与者】
 *   - Iterator（迭代器）：ListIterator，实现前向迭代器语义
 *   - Aggregate（聚合）：Node 构成的单链表
 */

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
using namespace std;

// 单链表节点
struct Node {
  string value;
  Node *next = nullptr; // 指向下一个节点

  explicit Node(const string &value) : value(value) {}

  // 构造时将当前节点链接到父节点后面
  Node(const string &value, Node *const parent) : value(value) {
    parent->next = this;
  }
};

// 自定义前向迭代器（替代 boost::iterator_facade）
// 实现了 increment()、equal() 和 dereference() 的核心语义
struct ListIterator {
  // STL 迭代器必须的类型定义（iterator traits）
  using iterator_category = forward_iterator_tag; // 前向迭代器标签
  using value_type = Node;                         // 元素类型
  using difference_type = ptrdiff_t;               // 迭代器差值类型
  using pointer = Node *;                          // 指针类型
  using reference = Node &;                        // 引用类型

  Node *current = nullptr; // 当前指向的节点

  ListIterator() {}

  explicit ListIterator(Node *const current) : current(current) {}

  // 解引用操作符：返回当前节点引用
  reference operator*() const { return *current; }

  // 成员访问操作符：返回当前节点指针
  pointer operator->() const { return current; }

  // 前置递增（prefix ++）：移动到下一个节点
  ListIterator &operator++() {
    current = current->next;
    return *this;
  }

  // 后置递增（postfix ++）：保存当前位置后移动到下一个节点
  ListIterator operator++(int) {
    ListIterator tmp = *this;
    current = current->next;
    return tmp;
  }

  // 相等比较
  bool operator==(const ListIterator &other) const {
    return current == other.current;
  }

  // 不等比较
  bool operator!=(const ListIterator &other) const {
    return current != other.current;
  }
};

int main() {
  // 构建单链表：alpha -> beta -> gamma
  Node alpha{"alpha"};
  Node beta{"beta", &alpha};   // beta 链接到 alpha 后面
  Node gamma{"gamma", &beta};  // gamma 链接到 beta 后面

  // 使用自定义迭代器与 STL for_each 算法配合
  // 起始迭代器指向 alpha，结束迭代器为默认构造（nullptr）
  for_each(ListIterator{&alpha}, ListIterator{},
           [](const Node &n) { cout << n.value << endl; });

  return 0;
}
