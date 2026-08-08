/*
 * =============================================================================
 * 设计模式：代理模式（Proxy Pattern）—— 通信代理
 * =============================================================================
 *
 * 【一句话概括】
 * 为远程对象提供本地代表，隐藏网络通信细节，使客户端可以像调用本地对象一样调用远程服务。
 *
 * 【适用场景 —— 通用】
 * - 需要访问位于不同进程或不同机器上的服务时（RPC、REST API、数据库代理等）
 *
 * 【金融工程应用】
 * - 交易网关代理：策略进程在本地调用 IExecutionProxy.submitOrder()，
 *   代理通过 TCP/共享内存将订单转发到交易网关进程，策略层完全不需要关心通信协议
 * - 行情订阅代理：策略通过 IQuoteProxy 订阅行情，代理封装了行情服务器的连接管理、
 *   断线重连、心跳维护，策略只看到"本地调用"
 * - 风控服务代理：本地策略通过 IRiskProxy.check(order) 调用远程风控微服务，
 *   代理隐藏了 gRPC/HTTP 通信细节，支持服务发现和负载均衡
 * - 回测集群代理：主节点通过 IWorkerProxy 将子任务分发到集群节点，
 *   代理封装了任务序列化和网络传输
 *
 * 【本示例说明】
 * RemotePong 通信代理将 ping() 调用转发给远程 Server，
 * tryit() 函数完全不感知底层是本地调用还是远程调用。
 */

#include <iostream>
#include <string>

using namespace std;

// 抽象主题（Subject）：定义统一的 ping 接口
struct Pingable {
  virtual string ping(const string &message) = 0;
};

// ============================================================================
// 真实主题（Real Subject）：本地 Pong 服务
// 直接在本地处理 ping 请求
// ============================================================================
struct Pong : Pingable {
  string ping(const string &message) override { return message + " pong"; }
};

// ============================================================================
// 模拟远程服务
// 在实际场景中，该服务运行在另一个进程或另一台机器上
// ============================================================================
struct Server {
  // 模拟远程过程调用：接收请求字符串，通过引用参数返回响应
  static void call(const string &request, string &response) {
    response = request + " pong";
  }
};

// ============================================================================
// 通信代理（Communication Proxy）：RemotePong
// 将本地 ping() 调用转换为对远程 Server 的调用
// 客户端无需知道底层通信细节
// ============================================================================
struct RemotePong : Pingable {
  string ping(const string &message) override {
    string response;
    // 通过 Server::call 模拟远程调用
    // 实际项目中这里可能是 HTTP 请求、gRPC 调用等
    Server::call(message, response);
    return response;
  }
};

// 客户端函数：通过 Pingable 接口工作
// 无论传入的是本地 Pong 还是 RemotePong，行为完全一致
void tryit(Pingable &pp) { cout << pp.ping("ping") << endl; }

int main() {
  // 本地服务：直接调用
  Pong pp;
  for (size_t i = 0; i < 3; ++i) {
    tryit(pp);
  }

  // 远程服务：通过通信代理透明访问
  RemotePong rp;
  for (size_t i = 0; i < 3; ++i) {
    tryit(rp);
  }
  return 0;
}
