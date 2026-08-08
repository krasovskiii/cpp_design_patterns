/*
 * =============================================================================
 * SOLID 设计原则 - 接口隔离原则 (Interface Segregation Principle, ISP)
 * =============================================================================
 *
 * 【一句话概括】
 * 客户端不应该被迫依赖它们不使用的方法。
 * 即：应该将臃肿的接口拆分为更小、更具体的接口。
 *
 * 【适用场景 —— 通用】
 * - 当一个接口包含太多方法，而不同实现者只需要其中一部分时
 * - 当发现实现类中有大量"空实现"或"抛出异常"的方法时
 * - 设计多功能设备（打印/扫描/传真）的抽象时
 * - 微服务架构：将庞大的 UserService 拆分为 IUserQuery、IUserAuth、IUserProfile
 * - GUI 控件：将"可点击""可拖拽""可缩放"拆分为独立接口，控件按需实现
 * - REST API 设计：不同客户端（Web/App/第三方）按需暴露不同 API 端点
 * - 权限系统：将大权限接口拆分为 IReadable、IWritable、IExecutable、IDeletable
 *
 * 【金融工程应用】
 * - 金融产品接口拆分：不要设计一个大而全的 IFinancialProduct 接口包含定价/风控/结算/行权
 *   应拆分为 IPricable（可定价）、IRiskMeasurable（可度量风险）、ISettleable（可结算）
 *   债券只需实现定价+结算，期权需要全部实现，外汇即期只需实现定价+结算，不关心行权
 * - 行情处理分离：行情模块拆分为 IQuoteReceiver（接收行情）、IQuoteFilter（过滤行情）、IQuoteStore（存储行情）
 *   实盘系统三个接口全实现，回放系统只需 IQuoteStore + IQuoteReceiver（从文件读取），无需实现过滤逻辑
 * - 策略信号接口：IEntrySignal（入场信号）、IExitSignal（出场信号）、IPositionSizer（仓位管理）、IRiskController（风控）
 *   趋势策略可能只需要入场+出场，网格策略需要全部四个，均值回归策略只需要入场+出场+风控
 * - 交易所接口分离：拆分 IOrderEntry（下单）、IOrderCancel（撤单）、IOrderQuery（查询）、IMarketData（行情）
 *   模拟交易可能只支持查询和行情，实盘全部支持，不同券商支持能力不同，按需实现
 * - 报表系统：IReportGenerator 拆分为 IBalanceSheet（资产负债表）、IIncomeStatement（利润表）、ICashFlow（现金流量表）
 *   不同报告类型独立演进，修改现金流量表格式不影响资产负债表生成
 *
 * 【反例 / 不遵守的后果】
 * - 实现者被迫提供无意义的方法实现（要么空着，要么抛异常）
 * - 接口的使用者被误导，以为所有实现都支持所有功能
 * - 修改接口的某个方法会影响所有实现者（即使它们不关心该方法）
 * - 量化平台中：所有策略必须实现 onOptionExercise() 空方法，即使 90% 的策略不涉及期权
 *
 * 【本例说明】
 * BadIMachine 将 print/scan/fax 放在一个接口中，
 * 导致 SimpleScanner 被迫实现它不需要的 print 和 fax 方法。
 * 好的方案：拆分为 IPrinter、IScanner、IFax 三个独立接口，
 * 客户端可以根据需要组合使用。
 */

#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct Document;  // 前向声明

// ===========================================================================
// 方案一：不好的接口设计 —— 违反接口隔离原则
// ===========================================================================
// 将所有功能塞到一个接口中，强制实现者提供所有方法
struct BadIMachine {
  // 多态基类应提供虚析构（Effective C++ 条款 7）
  virtual ~BadIMachine() = default;
  virtual void print(Document &doc) = 0;
  virtual void scan(Document &doc) = 0;
  virtual void fax(Document &doc) = 0;
};

// 问题：Scanner 根本不需要 print 和 fax，但被迫实现
struct BadScanner : BadIMachine {
  void print(Document &doc) override {
    // 注意：抛异常应「抛值」而非「抛指针」。
    // throw new ... 会泄漏内存且无法被 catch(exception&) 捕获（Effective C++ 条款 13）。
    throw logic_error("Not implemented.");  // 丑陋的"空实现"
  }

  void scan(Document &doc) override {
    // OK —— 真正需要的功能
  }

  void fax(Document &doc) override {
    throw logic_error("Not implemented.");  // 又一个不必要的实现
  }
};

// ===========================================================================
// 方案二：好的接口设计 —— 遵守接口隔离原则
// ===========================================================================
// 将大接口拆分为三个独立的、专注的小接口

struct IPrinter {
  virtual void print(Document &doc) = 0;   // 只负责打印
};

struct IScanner {
  virtual void scan(Document &doc) = 0;    // 只负责扫描
};

struct IFax {
  virtual void fax(Document &doc) = 0;     // 只负责传真
};

// ---- 简单设备：只需实现自己需要的接口 ----

struct Printer : IPrinter {
  void print(Document &doc) override {
    // 只需实现打印逻辑，无需关心扫描和传真
  }
};

struct Scanner : IScanner {
  void scan(Document &doc) override {
    // 只需实现扫描逻辑
  }
};

// ---- 复杂设备：通过多重继承组合多个接口 ----

struct ComplexMachine : IPrinter, IScanner {
  void print(Document &doc) override {
    // 打印逻辑
  }

  void scan(Document &doc) override {
    // 扫描逻辑
  }
};

// ---- 装饰器模式：动态组合已有设备的功能 ----
// 这种方式比 ComplexMachine 更灵活：可以组合任意已有设备的实现
struct DecoratedMachine : IPrinter, IScanner {
  IPrinter &printer;    // 引用已有的打印机（依赖注入）
  IScanner &scanner;    // 引用已有的扫描仪（依赖注入）

  DecoratedMachine(IPrinter &printer, IScanner &scanner)
      : printer(printer), scanner(scanner) {}

  void print(Document &doc) override { printer.print(doc); }  // 委托
  void scan(Document &doc) override { scanner.scan(doc); }    // 委托
};

// ---------------------------------------------------------------------------
// 主函数
// ---------------------------------------------------------------------------
int main() {
  // 接口隔离的好处：
  // - 客户端代码只需依赖自己真正使用的接口
  // - 每个实现类只需实现真正需要的方法
  // - 更容易测试和扩展
  return 0;
}
