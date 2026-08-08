/*
 * =============================================================================
 * 设计模式：中介者模式（Mediator）—— 聊天室实现
 * =============================================================================
 *
 * 【一句话概括】
 * ChatRoom 作为中介者，集中管理消息的路由和分发，使 Person 之间无需直接引用即可通信。
 *
 * 【适用场景 —— 通用】
 * - 多个对象之间需要通信，但希望避免直接耦合
 * - GUI 对话框中的控件交互：按钮、文本框、下拉框通过 Dialog 中介者协调
 *
 * 【金融工程应用】
 * - 交易系统中央路由：OrderRouter 作为中介者，统一路由订单到各交易所，
 *   策略和交易所不直接耦合，新增交易所只需在中介者注册
 * - 风控中心：RiskMediator 作为风控中介者，接收各模块（策略/执行/行情）的状态信息，
 *   统一评估全局风险并通知相关模块
 * - 事件总线：EventBus 作为中介者，各模块发布和订阅事件，
 *   行情变化→策略收到行情→产生信号→执行模块收到信号，全通过事件总线协调
 *
 * 【文件说明】ChatRoom.cpp - 聊天室中介者的实现
 */

#include "ChatRoom.h"
#include "Person.h"

// 广播消息：将消息发送给除发送者外的所有参与者
void ChatRoom::broadcast(const string &origin, const string &message) {
  for (auto p : people)
    if (p->name != origin)          // 不将消息发送给发送者自己
      // 此处运用了访问者模式（Visitor Pattern）的思想
      p->receive(origin, message);  // 通知每个参与者接收消息
}

// 参与者加入聊天室
void ChatRoom::join(Person *p) {
  string join_msg = p->name + " joins the chat";
  broadcast("room", join_msg); // 广播加入通知
  p->room = this;              // 将参与者的中介者指针指向自己
  people.push_back(p);         // 添加到参与者列表
}
