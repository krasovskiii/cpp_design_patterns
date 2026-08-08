/*
 * =============================================================================
 * 设计模式：迭代器模式（Iterator）—— C++20 协程实现
 * =============================================================================
 *
 * 【一句话概括】
 * 利用 C++20 协程的 co_yield 机制，以更自然的方式表达递归遍历逻辑。
 *
 * 【适用场景】
 * - 遍历逻辑天然递归，但希望以迭代方式暴露
 * - 需要惰性生成（lazy evaluation）遍历序列
 *
 * 【金融工程应用】
 * - 时间序列惰性遍历：回测中需要按时间顺序遍历海量 K 线，
 *   协程生成器惰性产出每根 K 线，避免一次性加载全部数据到内存
 * - 行情数据流处理：逐 Tick 惰性产出处理结果，实现流式处理管道
 *
 * 【关键参与者】
 *   - generator（协程生成器）：co_yield 逐个产出遍历节点
 *   - BinaryTree（聚合）：提供 post_order() 协程入口
 */

#include <coroutine>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

template <typename T> struct BinaryTree;

// 二叉树节点（与 binary_tree_iterator.cpp 中结构相同）
template <typename T> struct Node {
  T value = T();
  Node<T> *left = nullptr;
  Node<T> *right = nullptr;
  Node<T> *parent = nullptr;
  BinaryTree<T> *tree = nullptr;

  explicit Node(const T &value) : value(value) {}

  Node(const T &value, Node<T> *const left, Node<T> *const right)
      : value(value), left(left), right(right) {
    this->left->tree = this->right->tree = tree;
    this->left->parent = this->right->parent = this;
  }

  void set_tree(BinaryTree<T> *t) {
    tree = t;
    if (left)
      left->set_tree(t);
    if (right)
      right->set_tree(t);
  }

  ~Node() {
    if (left)
      delete left;
    if (right)
      delete right;
  }
};

// 二叉树容器
template <typename T> struct BinaryTree {
  Node<T> *root = nullptr;

  explicit BinaryTree(Node<T> *const root) : root{root} {
    root->set_tree(this);
  }

  ~BinaryTree() {
    if (root)
      delete root;
  }

  // 后序遍历入口：返回一个协程生成器
  // todo: 使用递归协程实现后序遍历迭代器
  generator<Node<T> *> post_order() { return post_order_impl(root); }

private:
  // 后序遍历的递归实现（使用递归生成器）
  // 遍历顺序：左子树 -> 右子树 -> 根节点
  // generator<Node<T> *> post_order_impl(Node<T> *node) {
  //   if (node) {
  //     for (auto x : post_order_impl(node->left))
  //       co_yield x;     // 先产出左子树的所有节点
  //     for (auto y : post_order_impl(node->right))
  //       co_yield y;     // 再产出右子树的所有节点
  //     co_yield node;    // 最后产出根节点
  //   }
  // }
};

int main() {
  // 构建二叉树（中序遍历视角）：
  //         me
  //       |     |
  //   mother   father
  //    |  |
  //  m'm  m'f

  BinaryTree<string> family{new Node<string>{
      "me",
      new Node<string>{"mother", new Node<string>{"mother's mother"},
                       new Node<string>{"mother's father"}},
      new Node<string>{"father"}}};

  cout << "=== postorder travesal with coroutines:\n";

  // 使用协程进行后序遍历（产出的是节点指针）
  // 后序遍历预期输出：m'm, m'f, mother, father, me
  // for (auto it : family.post_order()) {
  //   cout << it->value << endl;
  // }

  return 0;
}
