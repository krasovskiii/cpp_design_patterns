/*
 * ===========================================================================
 * 设计模式：Singleton Test Issues and DI（单例的测试问题与依赖注入）
 * ===========================================================================
 *
 * 【核心思想】
 * 单例模式最大的问题是可测试性差。因为单例直接硬编码了依赖关系，
 * 在单元测试中很难用模拟对象（mock）替换它。本例展示了这个问题，
 * 并通过依赖注入（Dependency Injection）来解决。
 *
 * 【适用场景 —— 通用】
 * - 需要对使用单例的代码进行单元测试
 * - 希望代码不依赖于具体的单例实现
 * - 需要能够替换数据源（真实数据库 vs 测试数据库）
 *
 * 【金融工程应用】
 * - 回测数据源替换：策略从 SingletonDatabase 读取行情，单元测试注入 DummyDatabase
 *   提供已知的模拟行情，验证策略逻辑正确性。实盘替换为真实行情源
 * - 交易接口 Mock：策略依赖 ExecutionInterface 单例下单，回测时注入 MockExecution
 *   记录下单信号而不实际发送，验证信号逻辑后，实盘切换为真实 CTP 接口
 * - 风控规则测试：风控模块依赖 RiskConfig 单例读取参数，测试时注入配置模拟对象，
 *   可以测试极端参数组合（如 0 持仓上限）下系统的行为
 *
 * 【单例测试问题】
 * SingletonRecordFinder 直接调用 SingletonDatabase::get() →
 *   测试依赖于真实的数据库文件（capitals.txt），无法独立运行
 *
 * 【DI 解决方案】
 * ConfigurableRecordFinder 通过构造函数接收 Database& →
 *   测试时可以注入 DummyDatabase（模拟数据），测试完全独立
 *
 * 【UML 关键参与者】
 * - Database（抽象接口）：Database —— 定义 get_population() 接口
 * - SingletonDatabase（单例实现）：依赖于外部文件
 * - DummyDatabase（测试替身）：使用硬编码的模拟数据
 * - SingletonRecordFinder（紧耦合）：直接依赖单例 → 难以测试
 * - ConfigurableRecordFinder（松耦合）：依赖注入 → 易于测试
 *
 * 【本例要点】
 * - 展示了单例模式导致测试困难的具体例子
 * - 展示了如何用依赖注入重构来提升可测试性
 * - 证明了"面向接口编程"优于"面向具体实现编程"
 */

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

/*
 * Database: 抽象接口
 *
 * 定义数据库的公共接口。通过面向接口编程，
 * 客户端不依赖具体的数据库实现。
 */
class Database {
public:
  // 多态基类需要虚析构（Effective C++ 条款 7）
  virtual ~Database() = default;
  virtual int get_population(const string &name) = 0;
};

/*
 * SingletonDatabase: 单例数据库实现
 *
 * 从文件中加载数据，实现 Database 接口。
 * 问题：测试时必须依赖真实的 capitals.txt 文件。
 *
 * 注意：继承必须是 public（Effective C++ 条款 32），私有继承无法通过
 * 基类指针使用多态，且与 DummyDatabase 的 : public Database 保持一致。
 */
class SingletonDatabase : public Database {
private:
  // 私有构造函数 —— 加载文件数据
  SingletonDatabase() {
    cout << "Initializing the database" << endl;
    ifstream ifs("capitals.txt");

    string s, s2;
    while (getline(ifs, s)) {
      getline(ifs, s2);
      int pop = stoi(s2);
      capitals[s] = pop;
    }
  }
  map<string, int> capitals;

public:
  // 禁止拷贝和赋值
  SingletonDatabase(const SingletonDatabase &) = delete;
  void operator=(const SingletonDatabase &) = delete;

  // 获取单例实例
  static SingletonDatabase &get() {
    static SingletonDatabase db;
    return db;
  }

  int get_population(const string &name) override { return capitals[name]; }
};

/*
 * DummyDatabase: 测试用模拟数据库
 *
 * 使用硬编码的测试数据，不依赖任何外部文件。
 * 用于单元测试中替换 SingletonDatabase。
 */
class DummyDatabase : public Database {
  map<string, int> capitals;

public:
  DummyDatabase() {
    capitals["alpha"] = 1;
    capitals["beta"] = 2;
    capitals["gamma"] = 3;
  }

  int get_population(const string &name) override { return capitals[name]; }
};

/*
 * SingletonRecordFinder: 紧耦合的记录查找器（反模式）
 *
 * 问题：直接硬编码调用 SingletonDatabase::get()。
 * 这导致：
 * 1. 无法在测试中替换为模拟数据库
 * 2. 测试结果依赖于 capitals.txt 文件的内容
 * 3. 测试不可重复 —— 如果文件内容变了，测试就失败
 */
struct SingletonRecordFinder {
  int total_population(vector<string> names) {
    int result{0};
    for (auto &name : names)
      result += SingletonDatabase::get().get_population(name);
    return result;
  }
};

/*
 * ConfigurableRecordFinder: 松耦合的记录查找器（推荐方式）
 *
 * 通过依赖注入（构造函数注入 Database&），数据库实现是可配置的。
 * 优势：
 * 1. 测试时可以注入 DummyDatabase
 * 2. 不依赖于全局单例
 * 3. 代码更灵活，可以在不同场景使用不同的数据库
 */
struct ConfigurableRecordFinder {
  Database &db;  // 依赖抽象接口，而非具体实现

  ConfigurableRecordFinder(Database &db) : db(db) {}

  int total_population(vector<string> names) {
    int result{0};
    for (auto &name : names)
      result += db.get_population(name);
    return result;
  }
};

int main() {
  // ===== 单例数据库的基本使用 =====
  string city = "Tokyo";
  int pop = SingletonDatabase::get().get_population(city);
  cout << city << " has a population of " << pop << endl;

  // ===== 问题演示：使用 SingletonRecordFinder 的测试 =====
  // 这个测试依赖于真实的数据库数据！
  // 如果 capitals.txt 文件中的 Seoul=1750000, Mexico City=17400000，
  // 则测试通过；否则失败。测试不可靠。
  SingletonRecordFinder rf;
  vector<string> names{"Seoul", "Mexico City"};
  int tp = rf.total_population(names);
  if (tp == 1750000 + 17400000) {
    cout << "Real test passed" << endl;
  }

  // ===== 解决方案：使用依赖注入的测试 =====
  // 使用 DummyDatabase，测试数据完全可控、可预测
  // 不依赖任何外部文件，测试结果始终一致
  DummyDatabase db;
  ConfigurableRecordFinder cf{db};
  if (4 == cf.total_population(vector<string>{"alpha", "gamma"})) {
    cout << "Mock test passed" << endl;
  }

  return 0;
}
