/*
 * ===========================================================================
 * 设计模式：Abstract Factory（抽象工厂模式）
 * 文件：main.cpp —— 演示入口
 * ===========================================================================
 *
 * 【核心思想】
 * 演示抽象工厂模式的使用，并对比不使用工厂模式时的代码。
 *
 * 【适用场景 —— 通用】
 * - 系统需要创建一系列相关的产品族，且不依赖于具体类
 * - 产品族中的对象需要一起使用，需要保证一致性
 * - 希望在不同产品族之间切换（如不同的 UI 主题、不同的数据库方言）
 *
 * 【金融工程应用】
 * - 交易所产品族切换：ShanghaiFuturesFactory 创建上期所的产品族（合约/行情/订单），
 *   DalianFuturesFactory 创建大商所的产品族，ShenzhenStockFactory 创建深交所产品族
 *   客户端代码依赖抽象工厂接口，切换交易所时只需替换具体工厂
 * - 回测/实盘环境切换：BacktestFactory 创建模拟组件族（模拟行情/模拟撮合/模拟账户），
 *   LiveFactory 创建实盘组件族（真实行情/CTP下单/真实账户），
 *   策略代码完全不变，只切换工厂即可在回测和实盘间切换
 * - 数据供应商切换：BloombergFactory 创建彭博数据产品族（行情/基本面/新闻），
 *   WindFactory 创建万得数据产品族，ReutersFactory 创建路透数据产品族，
 *   抽象工厂确保同一供应商的数据格式一致
 *
 * 【BAD_make_drink 的问题】
 * BAD_make_drink 函数展示了不使用工厂时的典型反模式：
 * 1. 使用 if-else 链判断类型 —— 每添加新饮品都要修改此函数（违反开闭原则）
 * 2. 创建逻辑和初始化逻辑混杂在一起
 * 3. 代码难以维护和扩展
 *
 * 【DrinkFactory 的优势】
 * 1. 通过注册表自动查找工厂，无需 if-else 链
 * 2. 添加新饮品只需注册新工厂，无需修改 make_drink 逻辑
 * 3. 创建和初始化分离（工厂负责创建，DrinkFactory 统一调用 prepare）
 */

#include "DrinkFactory.hpp"
#include <iostream>

using namespace std;

/*
 * BAD_make_drink: 不使用工厂模式的反面示例
 *
 * 问题分析：
 * 1. 违反开闭原则 —— 每添加一种新饮品（如 HotChocolate），
 *    都需要修改此函数，添加新的 if 分支
 * 2. 创建逻辑与具体的制备参数（200ml vs 50ml）硬编码在一起
 * 3. 函数职责不单一 —— 既要判断类型，又要创建对象，还要初始化
 *
 * 对比：使用 DrinkFactory 后，这些问题都被解决了。
 */
unique_ptr<HotDrink> BAD_make_drink(string type) {
  unique_ptr<HotDrink> drink;
  if (type == "tea") {
    // 茶的特定制备逻辑与创建逻辑耦合
    drink = make_unique<Tea>();
    drink->prepare(200);
  } else {
    drink = make_unique<Coffee>();
    drink->prepare(50);
  }
  return drink;
}

int main() {
  // 使用 DrinkFactory（推荐方式）
  DrinkFactory df;
  // 只需传递饮品名称，工厂自动完成创建和初始化
  auto coffee = df.make_drink("coffee");
  return 0;
}
