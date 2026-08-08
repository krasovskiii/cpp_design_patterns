/*
 * ===========================================================================
 * 设计模式：Facade Builder（外观建造者模式）
 * 文件：PersonJobBuilder.hpp —— 工作维度 Builder
 * ===========================================================================
 *
 * 【核心思想】
 * PersonJobBuilder 专注于构建 Person 的工作相关属性。
 * 它继承自 PersonBuilderBase，因此可以通过 lives()/works() 切换回其他 Builder。
 * 每个方法返回 Self&（自身引用），实现流式调用。
 *
 * 【职责】
 * - at():     设置公司名称
 * - as_a():   设置职位
 * - earning(): 设置年收入
 */

#pragma once
#include "PersonBuilder.hpp"
#include <string>

/*
 * PersonJobBuilder: 工作维度建造者
 *
 * 提供语义化的工作构建 API：
 * - .at("TTTech Auto")         设置公司名称
 * - .as_a("Project Engineer")  设置职位
 * - .earning(10e6)             设置年收入
 *
 * 所有方法返回 Self&，支持流式调用。
 * 由于继承自 PersonBuilderBase，可以通过 .lives()/.works() 切换到其他 Builder。
 */
class PersonJobBuilder : public PersonBuilderBase {
  typedef PersonJobBuilder Self;  // 类型别名，方便返回自身引用

public:
  // 构造函数接收 Person 引用，传递给基类
  explicit PersonJobBuilder(Person &person) : PersonBuilderBase{person} {}

  // 设置公司名称
  Self &at(std::string company_name) {
    person.company_name = company_name;
    return *this;
  }

  // 设置职位
  Self &as_a(std::string position) {
    person.position = position;
    return *this;
  }

  // 设置年收入
  Self &earning(int annual_income) {
    person.annual_income = annual_income;
    return *this;
  }
};
