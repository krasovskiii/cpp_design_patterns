/*
 * ===========================================================================
 * 设计模式：Facade Builder（外观建造者模式 / 多 Builder 组合）
 * 文件：Person.hpp —— Product（产品）角色
 * ===========================================================================
 *
 * 【核心思想】
 * 当一个对象有多个不同维度的属性（如地址信息、工作信息），可以为每个维度
 * 创建专门的子 Builder，然后通过一个外观（Facade）Builder 来协调切换。
 * 这样每个子 Builder 只关注自己的职责，同时又能无缝切换。
 *
 * 【适用场景】
 * - 对象属性可以按维度分组（如 Person 的地址 vs 工作）
 * - 希望每个维度的构建 API 具有语义化的方法名
 * - 构建过程涉及多种不同类型的配置，需要清晰的职责分离
 *
 * 【UML 关键参与者】
 * - Product（产品）：Person —— 包含地址和工作两方面的属性
 * - BuilderBase（建造者基类）：PersonBuilderBase —— 持有 Person 引用，
 *   提供 lives()/works() 两个切换方法
 * - ConcreteBuilder（具体建造者）：
 *   - PersonAddressBuilder —— 构建地址相关属性
 *   - PersonJobBuilder —— 构建工作相关属性
 * - Facade（外观）：PersonBuilder —— 唯一持有 Person 实例的 Builder
 *
 * 【设计要点】
 * - Person 只有 PersonBuilder 真正拥有其实例（成员变量 Person p）
 * - 其他 Builder 通过 PersonBuilderBase 中的 Person& 引用操作同一个对象
 * - 这样避免了在 Builder 切换时的多次拷贝/析构
 * - Person 的构造函数是 private 的，只能通过 Person::create() 创建
 */

#pragma once
#include <iostream>
#include <string>

class PersonBuilder;

/*
 * Person: 产品类
 *
 * 包含两个维度的属性：
 * 1. 地址维度：街道地址、邮政编码、城市
 * 2. 工作维度：公司名称、职位、年收入
 *
 * 构造函数为 private，只能通过静态工厂方法 Person::create() 创建。
 * PersonBuilder 及其子 Builder 作为 friend 类访问私有成员。
 */
class Person {
  // ===== 地址相关属性 =====
  std::string street_address, post_code, city;

  // ===== 工作相关属性 =====
  std::string company_name, position;
  int annual_income{0};

  // 构造函数私有 —— 只能通过 Person::create() 创建
  Person() { std::cout << "[Person Created]" << std::endl; }

public:
  ~Person() { std::cout << "[Person Destroyed]" << std::endl; }

  // 移动构造函数 —— 支持从 Builder 中移动出 Person 对象
  Person(Person &&other)
      : street_address{std::move(other.street_address)},
        post_code{std::move(other.post_code)}, city{std::move(other.city)},
        company_name{std::move(other.company_name)}, position{std::move(
                                                         other.position)},
        annual_income{other.annual_income} {
    std::cout << "[Person Move Constructed]" << std::endl;
  }

  // 移动赋值运算符
  Person &operator=(Person &&other) {
    if (this == &other)
      return *this;
    street_address = std::move(other.street_address);
    post_code = std::move(other.post_code);
    city = std::move(other.city);
    company_name = std::move(other.company_name);
    position = std::move(other.position);
    annual_income = other.annual_income;
    std::cout << "[Person Move Assigned]" << std::endl;
    return *this;
  }

  // 静态工厂方法 —— 创建 PersonBuilder 并开始构建
  static PersonBuilder create();

  // 输出 Person 的所有信息
  friend std::ostream &operator<<(std::ostream &os, const Person &person) {
    return os << "street address: " << person.street_address << std::endl
              << "post code: " << person.post_code << std::endl
              << "city: " << person.city << std::endl
              << "company name: " << person.company_name << std::endl
              << "position: " << person.position << std::endl
              << "annual income: " << person.annual_income << std::endl;
  }

  // Builder 类需要访问 Person 的私有成员
  friend class PersonBuilder;
  friend class PersonJobBuilder;
  friend class PersonAddressBuilder;
};
