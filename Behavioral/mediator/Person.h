/*
 * 设计模式：中介者模式（Mediator）—— 参与者（Colleague）
 * 核心思想：Person 作为同事类（Colleague），不直接与其他 Person 通信，
 *           而是通过 ChatRoom 中介者来发送和接收消息。
 * 文件说明：Person.h - 参与者类的声明
 */

#pragma once

#include <string>
#include <vector>
using namespace std;

struct ChatRoom; // 前置声明中介者

// 参与者（Colleague 同事类）：聊天室中的用户
struct Person {
  string name;
  ChatRoom *room{nullptr};    // 指向全局中介者（聊天室）的指针
  vector<string> chat_log;    // 聊天记录

  explicit Person(const string &name);

  // 发送全局消息（通过中介者广播）
  void say(const string &message) const;

  // 发送私信（通过中介者转发）
  void pm(const string &who, const string &message) const;

  // 接收消息（由中介者调用）
  void receive(const string &origin, const string &message);

  bool operator==(const Person &rhs) const;

  bool operator!=(const Person &rhs) const;
};
