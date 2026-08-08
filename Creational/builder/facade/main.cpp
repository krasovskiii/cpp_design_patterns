/*
 * ===========================================================================
 * 设计模式：Facade Builder（外观建造者模式）
 * 文件：main.cpp —— 演示入口
 * ===========================================================================
 *
 * 【概述】
 * Person 是一个复杂的类，我们不希望有一个庞大的构造函数。
 * 我们需要 2 个 Builder：Address Builder 和 Job Builder。
 *
 * 实现逻辑需要创建 4 个 Builder 类：
 *
 * - PersonBuilderBase: 基类，帮助我们链式切换不同类型的 Builder。
 *   持有 Person& 引用，所有子 Builder 通过它访问同一个 Person 对象。
 *
 * - PersonBuilder: 唯一的目的就是持有实际的 Person 对象。
 *   其他 Builder 通过引用操作它。这样，我们避免了在 Builder 链中
 *   对对象进行多次复制和析构。
 *
 * - PersonJobBuilder: 创建工作相关的字段。
 *
 * - PersonAddressBuilder: 创建地址相关的字段。
 *
 * 【适用场景】
 * - 复杂对象有多个维度的属性（如 Person 有地址、工作、联系方式），需要分面构建
 * - 希望在不同子 Builder 之间无缝切换（.lives() → .works() → .lives()）
 * - 需要分步骤、分模块地填充一个大型配置对象
 *
 * 【金融工程应用】
 * - 金融产品多维度构建：结构化产品包含产品属性（票息、期限）、风控属性（保证金率、杠杆）、
 *   结算属性（交割方式、结算货币），使用 Facet Builder 分离不同维度的构建逻辑
 *   Product::create().terms().coupon(0.05).tenor(12).risk().margin(0.10).leverage(2).settlement().currency(USD).mode(Delivery)
 * - 交易策略多面配置：策略包含信号参数（周期、阈值）、执行参数（算法类型、激进程度）、
 *   风控参数（止损比例、最大持仓），各维度独立 Builder 避免参数混淆
 * - 回测引擎多模块配置：数据配置（数据源、频率）、交易配置（手续费、滑点）、
 *   绩效分析配置（基准、无风险利率），Facet Builder 让每个模块的配置各司其职
 *
 * 【构建流程】
 * Person::create()          -> 创建 PersonBuilder（持有 Person 实例）
 *   .lives()                -> 切换到 PersonAddressBuilder
 *     .at("street")         -> 设置街道
 *     .with_postcode("123") -> 设置邮编
 *     .in("city")           -> 设置城市
 *   .works()                -> 切换回 PersonJobBuilder
 *     .at("company")        -> 设置公司
 *     .as_a("engineer")     -> 设置职位
 *     .earning(100000)      -> 设置收入
 * 隐式转换为 Person         -> 完成构建，获取最终对象
 */

#include "Person.hpp"
#include "PersonAddressBuilder.hpp"
#include "PersonBuilder.hpp"
#include "PersonJobBuilder.hpp"
#include <iostream>

using namespace std;

int main() {
  // 使用多个 Builder 并在它们之间自由切换
  //
  // 构建过程：
  // 1. Person::create() 创建 PersonBuilder
  // 2. .lives() 切换到地址 Builder
  // 3. .at(...).with_postcode(...).in(...) 设置地址
  // 4. .works() 切换到工作 Builder
  // 5. .at(...).as_a(...).earning(...) 设置工作信息
  // 6. 赋值给 Person 时触发隐式类型转换（移动语义）
  Person p = Person::create()
                 .lives()
                 .at("Gudrunstrasse")
                 .with_postcode("80364")
                 .in("Munich")
                 .works()
                 .at("TTTech Auto")
                 .as_a("Project Engineer")
                 .earning(10e6);

  cout << p << endl;
  return 0;
}
