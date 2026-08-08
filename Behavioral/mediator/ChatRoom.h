/*
 * 设计模式：中介者模式（Mediator）—— 聊天室示例
 * 核心思想：定义一个中介对象来封装一组对象之间的交互，使各对象之间松耦合。
 *           ChatRoom 作为中介者，管理 Person 之间的消息传递。
 * 适用场景：
 *   - 一组对象以定义良好但复杂的方式通信，导致相互依赖关系混乱
 *   - 需要集中控制一组对象的通信逻辑
 *   - 希望复用某个对象，但它与其他对象的通信过于复杂
 * 关键参与者：
 *   - Mediator（中介者接口）：ChatRoom，定义通信接口
 *   - ConcreteMediator（具体中介者）：ChatRoom，实现广播和私信
 *   - Colleague（同事类）：Person，通过中介者进行通信
 * 文件说明：ChatRoom.h - 聊天室中介者的声明和部分实现
 */

#pragma once

#include "Person.h"
#include <algorithm>

// 聊天室中介者：管理参与者并协调消息传递
struct ChatRoom {
  // 参与者列表
  vector<Person *> people;

  // 广播消息给所有参与者（使用访问者模式）
  void broadcast(const string &origin, const string &message);

  // 参与者加入聊天室
  void join(Person *p);

  // 发送私信给指定参与者
  void message(const string &origin, const string &who, const string &message) {
    // 在参与者列表中查找目标
    auto target = std::find_if(begin(people), end(people),
                               [&](const Person *p) { return p->name == who; });
    if (target != end(people)) {
      (*target)->receive(origin, message); // 调用目标参与者的 receive() 方法
    }
  }
};
