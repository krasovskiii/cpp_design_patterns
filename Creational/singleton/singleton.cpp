/*
 * ===========================================================================
 * 设计模式：Singleton（单例模式）
 * ===========================================================================
 *
 * 【核心思想】
 * 确保一个类只有一个实例，并提供一个全局访问点。单例模式是创建型模式中
 * 最简单也是最常用的模式之一。
 *
 * 【适用场景 —— 通用】
 * - 系统中只需要一个实例来协调行为（如配置管理器、日志记录器、数据库连接池）
 * - 该实例需要被多个客户端共享访问
 * - 需要对该实例进行严格控制（如访问计数、懒加载）
 * - 硬件接口访问：打印机管理器、串口管理器等物理资源唯一访问点
 *
 * 【金融工程应用】
 * - 交易会话管理器：一个进程中只需一个 TradingSession 实例，管理当日的交易状态、
 *   连接状态、会话时间窗口，避免多个实例导致状态不一致
 * - 全局配置管理器：量化系统的大量参数（合约乘数、保证金率、费率表）需要全局一致，
 *   ConfigManager 单例确保所有模块读取相同的配置，避免配置不一致导致的交易错误
 * - 行情数据总线：QuoteBus 单例作为全局行情分发中心，所有策略订阅行情、
 *   所有模块发布行情都通过同一个总线，保证行情的全局一致性
 * - 订单路由管理器：OrderRouter 单例作为全局订单入口，统一管理订单的创建、
 *   路由和状态跟踪，确保订单不会重复提交或丢失
 * - 日志与审计记录器：AuditLogger 单例记录所有交易操作，确保审计日志的完整性和时序一致性，
 *   多个策略/模块的日志通过同一通道输出，保持时序正确
 * - 数据库连接池：DBConnectionPool 单例管理 MySQL/Redis 连接的复用，
 *   避免每个策略独立创建连接导致连接数爆炸
 *
 * 【解决的问题】
 * - 保证全局只有一个实例，避免多个实例导致的状态不一致
 * - 提供全局访问点，避免到处传递对象引用
 * - 支持懒加载（lazy initialization），只在第一次使用时创建
 *
 * 【UML 关键参与者】
 * - Singleton（单例）：SingletonDatabase —— 拥有私有构造函数和静态访问方法
 *   - 私有构造函数阻止外部直接创建实例
 *   - 删除拷贝构造和赋值操作，防止复制
 *   - 静态 get() 方法提供唯一的全局访问点
 *
 * 【本例要点】
 * - SingletonDatabase 从文件 "capitals.txt" 加载首都人口数据
 * - 使用 Meyers' Singleton（C++11 保证的线程安全静态局部变量初始化）
 * - static 局部变量在第一次调用 get() 时初始化，后续调用返回同一实例
 * - 删除拷贝构造和赋值操作，防止创建多个实例
 */

#include <fstream>
#include <iostream>
#include <map>
#include <string>

using namespace std;

/*
 * SingletonDatabase: 单例数据库
 *
 * 关键设计要素：
 * 1. 私有构造函数 —— 外部无法直接创建实例
 * 2. 删除拷贝构造和赋值 —— 防止通过复制产生多个实例
 * 3. 静态 get() 方法 —— 全局唯一访问点
 * 4. 静态局部变量 —— C++11 保证线程安全的懒加载
 */
class SingletonDatabase {
private:
  // 私有构造函数 —— 只能通过 get() 方法创建实例
  SingletonDatabase() {
    cout << "Initializing the database" << endl;
    // 从文件加载首都人口数据
    ifstream ifs("capitals.txt");

    string s, s2;
    while (getline(ifs, s)) {
      getline(ifs, s2);
      int pop = stoi(s2);
      capitals[s] = pop;
    }
  }
  map<string, int> capitals;  // 首都名称 -> 人口数量

public:
  // 禁止拷贝和赋值 —— 确保单例的唯一性
  SingletonDatabase(const SingletonDatabase &) = delete;
  void operator=(const SingletonDatabase &) = delete;

  /*
   * get(): 获取单例实例的静态方法
   *
   * 使用 Meyers' Singleton 模式：
   * - static 局部变量 db 在第一次调用 get() 时初始化
   * - C++11 保证 static 局部变量初始化是线程安全的
   * - 后续调用直接返回已初始化的实例引用
   *
   * 注意：返回的是引用（&），不是指针，调用者不能 delete 它
   */
  static SingletonDatabase &get() {
    static SingletonDatabase db;
    return db;
  }

  // 查询某城市的人口数量
  int get_population(const string &name) { return capitals[name]; }
};

int main() {
  // 不能这样做！拷贝构造被删除了
  // auto db = SingletonDatabase::get();

  string city = "Tokyo";
  // 直接通过 get() 访问单例，查询人口
  int pop = SingletonDatabase::get().get_population(city);
  cout << city << " has a population of " << pop << endl;
  return 0;
}
