/*
 * 设计模式：中介者模式（Mediator）—— 聊天室客户端
 * 核心思想：演示 Person 通过 ChatRoom 中介者进行全局消息广播和私信。
 * 文件说明：main.cpp - 客户端入口，组装中介者模式示例
 */

#include "ChatRoom.h"
#include "Person.h"

int main() {
  ChatRoom room; // 创建中介者（聊天室）

  Person john{"John"};
  Person jane{"Jane"};
  room.join(&john); // John 加入聊天室
  room.join(&jane); // Jane 加入聊天室
  john.say("hi room");        // John 发送全局消息
  jane.say("oh, hey john");   // Jane 回复全局消息

  Person simon{"Simon"};
  room.join(&simon);          // Simon 加入聊天室
  simon.say("hi everyone!");  // Simon 发送全局消息

  jane.pm("Simon", "glad you found us, simon!"); // Jane 给 Simon 发私信

  return 0;
}
