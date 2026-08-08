/*
 * 设计模式：观察者模式（Observer）—— 被观察者（Observable）
 * 核心思想：Observable 维护观察者列表，提供 subscribe/unsubscribe/notify 操作。
 *           当状态变化时，通知所有已注册的观察者。
 * 注意：此实现不是线程安全的。
 * 文件说明：observable.h - 基础被观察者模板
 */

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

template <typename T> struct Observer;

// 被观察者（Observable/Subject）：维护观察者列表并负责通知
template <typename T> struct Observable {
  // vector 不是线程安全的容器
  std::vector<Observer<T> *> observers;

public:
  // 通知所有观察者：遍历列表并调用每个观察者的 field_changed() 回调
  void notify(T &source, const std::string &field_name) {
    // 读取观察者列表并逐一通知
    for (auto observer : observers)
      observer->field_changed(source, field_name);
  }

  // 订阅：将观察者添加到通知列表
  void subscribe(Observer<T> &observer) {
    // 向列表中添加观察者指针
    observers.push_back(&observer);
  }

  // 取消订阅：将观察者从通知列表中移除
  void unsubscribe(Observer<T> &observer) {
    // 使用 erase-remove 惯用法移除指定观察者
    observers.erase(remove(observers.begin(), observers.end(), &observer),
                    observers.end());
  }
};
