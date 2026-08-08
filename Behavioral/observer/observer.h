/*
 * 设计模式：观察者模式（Observer）—— 观察者接口
 * 核心思想：定义观察者（Observer）需要实现的回调接口。
 *           观察者订阅被观察者（Observable）的状态变化通知。
 * 文件说明：observer.h - 观察者接口的模板定义
 */

#include <string>

// 观察者接口模板
// T：被观察对象的类型
template <typename T> struct Observer {
  // 字段变化回调：当被观察对象的某个字段发生变化时被调用
  // source: 发出通知的被观察对象引用
  // field_name: 发生变化的字段名称
  virtual void field_changed(T &source, const std::string &field_name) = 0;
};
