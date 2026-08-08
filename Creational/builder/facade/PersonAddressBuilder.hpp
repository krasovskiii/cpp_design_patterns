/*
 * ===========================================================================
 * 设计模式：Facade Builder（外观建造者模式）
 * 文件：PersonAddressBuilder.hpp —— 地址维度 Builder
 * ===========================================================================
 *
 * 【核心思想】
 * PersonAddressBuilder 专注于构建 Person 的地址相关属性。
 * 它继承自 PersonBuilderBase，因此可以通过 lives()/works() 切换回其他 Builder。
 * 每个方法返回 Self&（自身引用），实现流式调用。
 *
 * 【职责】
 * - at():          设置街道地址
 * - with_postcode(): 设置邮政编码
 * - in():          设置城市
 */

#pragma once
#include "PersonBuilder.hpp"
#include <string>

/*
 * PersonAddressBuilder: 地址维度建造者
 *
 * 提供语义化的地址构建 API：
 * - .at("Gudrunstrasse")          设置街道地址
 * - .with_postcode("80364")       设置邮政编码
 * - .in("Munich")                 设置城市
 *
 * 所有方法返回 Self&，支持流式调用。
 * 由于继承自 PersonBuilderBase，可以通过 .lives()/.works() 切换到其他 Builder。
 */
class PersonAddressBuilder : public PersonBuilderBase {
  typedef PersonAddressBuilder Self;  // 类型别名，方便返回自身引用

public:
  // 构造函数接收 Person 引用，传递给基类
  PersonAddressBuilder(Person &person) : PersonBuilderBase(person) {}

  // 设置街道地址
  Self &at(std::string street_address) {
    person.street_address = street_address;
    return *this;
  }

  // 设置邮政编码
  Self &with_postcode(std::string post_code) {
    person.post_code = post_code;
    return *this;
  }

  // 设置城市
  Self &in(std::string city) {
    person.city = city;
    return *this;
  }
};
