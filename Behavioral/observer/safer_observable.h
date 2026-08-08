/*
 * 设计模式：观察者模式（Observer）—— 线程安全的被观察者
 * 核心思想：在 Observable 的基础上增加互斥锁（mutex）保护，确保多线程环境下的安全操作。
 *           但需要注意：如果在 notify 期间调用 unsubscribe，可能导致死锁！
 * 文件说明：safer_observable.h - 带互斥锁保护的被观察者模板
 */

#include <algorithm>
#include <mutex>
#include <string>
#include <vector>

using namespace std;

template <typename T> struct Observer;

// 线程安全版被观察者（SaferObservable）：使用互斥锁保护观察者列表
template <typename T> struct SaferObservable {
  std::vector<Observer<T> *> observers;
  typedef std::mutex mutex_t;
  mutex_t mtx; // 互斥锁，保护 observers 列表

public:
  // 线程安全地通知所有观察者
  void notify(T &source, const std::string &field_name) {
    std::scoped_lock<mutex_t> lock{mtx}; // RAII 锁，作用域结束自动释放
    for (auto observer : observers)
      observer->field_changed(source, field_name);
  }

  // 线程安全地添加观察者
  void subscribe(Observer<T> &observer) {
    std::scoped_lock<mutex_t> lock{mtx};
    observers.push_back(&observer);
  }

  // 线程安全地移除观察者
  void unsubscribe(Observer<T> &observer) {
    // 注意：如果在 notify() 执行期间调用此方法，可能导致死锁！
    // 因为 notify() 已经持有锁，此方法尝试再次获取同一把锁
    // 解决方法：使用递归锁（recursive_mutex）或在 notify 中先拷贝列表再遍历
    std::scoped_lock<mutex_t> lock{mtx};
    observers.erase(remove(observers.begin(), observers.end(), &observer),
                    observers.end());
  }
};
