/*
 * ===========================================================================
 * 设计模式：Facade Builder（外观建造者模式）
 * 文件：Person.cpp —— Person::create() 实现
 * ===========================================================================
 *
 * Person::create() 是构建过程的入口点。
 * 它创建一个 PersonBuilder 对象，用户通过这个 Builder 开始流式构建。
 *
 * 为什么这里需要单独的实现文件？
 * Person::create() 返回 PersonBuilder 对象，而 PersonBuilder 的完整定义
 * 在 PersonBuilder.hpp 中。Person.hpp 中只有前向声明，因此 create() 的实现
 * 必须放在能看到 PersonBuilder 完整定义的地方。
 */

#include "Person.hpp"
#include "PersonBuilder.hpp"

using namespace std;

// 静态工厂方法：创建 PersonBuilder 并返回，开始构建流程
PersonBuilder Person::create() { return PersonBuilder(); }
