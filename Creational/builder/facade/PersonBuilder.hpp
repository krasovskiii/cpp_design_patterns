/*
 * ===========================================================================
 * 设计模式：Facade Builder（外观建造者模式）
 * 文件：PersonBuilder.hpp —— Builder 基类和 Facade
 * ===========================================================================
 *
 * 【核心思想】
 * PersonBuilderBase 作为所有子 Builder 的公共基类，持有一个 Person 引用。
 * 它提供了两个关键方法 lives() 和 works()，用于在地址 Builder 和工作 Builder
 * 之间无缝切换。PersonBuilder 是唯一的"拥有者"Builder —— 它真正持有 Person
 * 实例，而其他 Builder 只持有引用。
 *
 * 【设计要点】
 * - PersonBuilderBase 持有 Person&（引用），不拥有对象
 * - PersonBuilder 持有 Person p（成员变量），真正拥有对象
 * - lives() 返回 PersonAddressBuilder，切换至地址构建
 * - works() 返回 PersonJobBuilder，切换至工作构建
 * - operator Person() 支持将 Builder 隐式转换为 Person（移动语义）
 */

#pragma once
#include "Person.hpp"

class PersonAddressBuilder;
class PersonJobBuilder;

/*
 * PersonBuilderBase: Builder 基类
 *
 * 我们使用这个类作为基础，保持对 Person 对象的引用。
 * 这是因为我们不希望 Person 在每次切换 Builder 时被复制。
 * 实际的 Person 对象保存在 PersonBuilder 中，每个子 Builder
 * 通过继承 PersonBuilderBase 获得对同一 Person 对象的引用。
 */
class PersonBuilderBase {
protected:
  Person &person;  // 对被构建的 Person 对象的引用（不拥有所有权）

  // 构造函数 protected —— 只有子类可以构造
  PersonBuilderBase(Person &person) : person{person} {}

public:
  // 隐式类型转换：允许将 Builder 转换为 Person（使用移动语义）
  operator Person() const { return std::move(person); }

  /*
   * lives(): 切换到地址 Builder
   *
   * 返回 PersonAddressBuilder，后续调用链将使用地址相关的方法。
   * 例如：.lives().at("street").with_postcode("12345").in("city")
   */
  PersonAddressBuilder lives() const;

  /*
   * works(): 切换到工作 Builder
   *
   * 返回 PersonJobBuilder，后续调用链将使用工作相关的方法。
   * 例如：.works().at("company").as_a("engineer").earning(100000)
   */
  PersonJobBuilder works() const;
};

/*
 * PersonBuilder: 外观 Builder（唯一持有 Person 实例的 Builder）
 *
 * 这是构建的入口点。PersonBuilder 真正持有 Person 对象（成员变量 p），
 * 而其他 Builder（PersonAddressBuilder、PersonJobBuilder）通过
 * PersonBuilderBase 中的 Person& 引用访问这个对象。
 *
 * 使用流程：
 *   Person::create() -> PersonBuilder -> .lives() -> PersonAddressBuilder
 *                                    -> .works() -> PersonJobBuilder
 */
class PersonBuilder : public PersonBuilderBase {
public:
  // 构造函数初始化 PersonBuilderBase 时传入自身的 Person 成员
  PersonBuilder() : PersonBuilderBase{p} {}

private:
  Person p;  // 唯一真正持有的 Person 实例
};
