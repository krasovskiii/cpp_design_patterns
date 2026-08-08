/*
 * 设计模式：中介者模式（Mediator）—— 参与者实现
 * 核心思想：Person 的所有通信操作都委托给 ChatRoom 中介者，
 *           自身不直接持有其他参与者的引用。
 * 文件说明：Person.cpp - 参与者类的实现
 */

#include "Person.h"
#include "ChatRoom.h"
#include <iostream>

Person::Person(const string &name) : name(name) {}

// 发送全局消息：委托中介者进行广播
void Person::say(const string &message) const {
  room->broadcast(name, message);
}

// 发送私信：委托中介者转发给目标用户
void Person::pm(const string &who, const string &message) const {
  room->message(name, who, message);
}

// 接收消息：由中介者调用，打印消息并记录到聊天日志
void Person::receive(const string &origin, const string &message) {
  string s{origin + ": \"" + message + "\""};
  std::cout << "[" << name << "'s chat session]" << s << "\n";
  chat_log.emplace_back(s); // 将消息添加到聊天记录
}

bool Person::operator==(const Person &rhs) const { return name == rhs.name; }

bool Person::operator!=(const Person &rhs) const { return !(rhs == *this); }
