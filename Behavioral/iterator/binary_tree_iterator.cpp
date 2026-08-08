/*
 * =============================================================================
 * 设计模式：迭代器模式（Iterator）—— 二叉树自定义迭代器
 * =============================================================================
 *
 * 【一句话概括】
 * 为二叉树数据结构实现自定义迭代器，支持前序遍历，用统一 for-range 语法遍历。
 *
 * 【适用场景 —— 通用】
 * - 需要为自定义数据结构提供 STL 兼容的迭代器
 * - 需要支持多种遍历策略（前序、中序、后序等）
 *
 * 【金融工程应用】
 * - 时间序列遍历：为自定义 K 线序列实现迭代器，支持按时间正序/倒序遍历，
 *   策略代码用 for (auto &bar : bars) 风格读取，无需关心底层存储结构
 * - 订单簿遍历：为订单簿（OrderBook）实现迭代器，支持从买一/卖一开始逐档遍历，
 *   统一接口简化策略的订单簿分析代码
 * - 因子树遍历：自定义因子计算树（DAG）实现迭代器，按拓扑序或依赖序自动遍历计算
 *
 * 【关键参与者】
 *   - Iterator（迭代器）：PreOrderIterator，实现 ++、*、!= 操作
 *   - Aggregate（聚合）：BinaryTree，提供 begin() 和 end()
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
using namespace std;

template <typename T> struct BinaryTree;

// 二叉树节点
template <typename T> struct Node {
  T value = T();
  Node<T> *left = nullptr;   // 左子节点
  Node<T> *right = nullptr;  // 右子节点
  Node<T> *parent = nullptr; // 父节点（用于迭代器回溯）
  BinaryTree<T> *tree = nullptr; // 指向所属树的指针

  explicit Node(const T &value) : value(value) {}

  Node(const T &value, Node<T> *const left, Node<T> *const right)
      : value(value), left(left), right(right) {
    this->left->tree = this->right->tree = tree;
    this->left->parent = this->right->parent = this; // 设置父节点引用
  }

  // 递归设置子树中所有节点的 tree 指针
  void set_tree(BinaryTree<T> *t) {
    tree = t;
    if (left)
      left->set_tree(t);
    if (right)
      right->set_tree(t);
  }

  // 递归删除所有子节点
  ~Node() {
    if (left)
      delete left;
    if (right)
      delete right;
  }
};

// 二叉树容器
template <typename T> struct BinaryTree {
  Node<T> *root = nullptr; // 仅保存根节点引用

  explicit BinaryTree(Node<T> *const root) : root{root}, pre_order{*this} {
    root->set_tree(this); // 初始化所有节点的 tree 指针
  }

  ~BinaryTree() {
    if (root)
      delete root;
  }

  // V1: 实现一个前序遍历迭代器（默认遍历策略）
  template <typename U> struct PreOrderIterator {
    Node<U> *current;

    explicit PreOrderIterator(Node<U> *current) : current(current) {}

    // 比较两个迭代器是否指向不同节点
    bool operator!=(const PreOrderIterator<U> &other) {
      return current != other.current;
    }

    // 前序遍历的递增操作（非递归实现，复杂度较高但避免了递归栈开销）
    PreOrderIterator<U> &operator++() {
      if (current->right) {
        // 有右子节点：进入右子树，然后一路向左走到底
        current = current->right;
        while (current->left)
          current = current->left;
      } else {
        // 无右子节点：向上回溯，找到第一个当前节点在其左子树中的祖先
        Node<T> *p = current->parent;
        while (p && current == p->right) {
          current = p;
          p = p->parent;
        }
        current = p;
      }
      return *this;
    }

    // 解引用：返回当前节点引用
    Node<U> &operator*() { return *current; }
  }; // struct PreOrderIterator

  using iterator = PreOrderIterator<T>;  // 用 using 而非 typedef

  // 结束迭代器：nullptr 表示遍历结束
  iterator end() { return iterator{nullptr}; }

  // 起始迭代器：从根节点开始，一路向左走到底（前序遍历的第一个节点）
  iterator begin() {
    Node<T> *n = root;

    if (n)
      while (n->left)
        n = n->left; // 找到最左节点
    return iterator{n};
  }

  // V2: 通过遍历对象暴露多种遍历策略
  // 用户可以显式选择使用哪个遍历对象（如 family.pre_order）
  class PreOrderTraversal {
    BinaryTree<T> &tree;

  public:
    explicit PreOrderTraversal(BinaryTree<T> &tree) : tree{tree} {}
    iterator begin() { return tree.begin(); }
    iterator end() { return tree.end(); }
  } pre_order; // 前序遍历对象
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

  // V1: 使用默认迭代器进行前序遍历
  // 用户无法选择遍历策略（始终为前序遍历）
  for (auto it = family.begin(); it != family.end(); ++it) {
    cout << (*it).value << "\n";
  }

  // 等价写法：使用 range-for 语法
  for (const auto &it : family) {
    cout << it.value << "\n";
  }

  cout << "=== and now, through a dedicated object:\n";

  // V2: 通过遍历对象显式选择遍历策略
  for (const auto &it : family.pre_order) {
    cout << it.value << "\n";
  }

  return 0;
}
