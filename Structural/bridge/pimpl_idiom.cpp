/*
 * =============================================================================
 * 设计模式：桥接模式 / Pimpl 惯用法（Bridge Pattern / Pimpl Idiom）
 * =============================================================================
 *
 * 【一句话概括】
 * 通过将实现细节隐藏在 .cpp 文件中，减少头文件依赖，提高编译速度和二进制兼容性。
 *
 * 【适用场景 —— 通用】
 * - 类的实现细节频繁变化、包含平台相关代码，或希望减少编译依赖时
 * - SDK/API 发布：对外暴露稳定头文件，内部实现随意修改不影响客户编译
 *
 * 【金融工程应用】
 * - 交易 API SDK 封装：对外发布的交易接口头文件保持稳定（不暴露内部实现），
 *   内部 CTP/XTP 版本升级时客户无需重新编译，只需替换动态库
 * - 定价引擎封装：头文件只暴露 clean 接口，内部数值计算（矩阵运算库/GPU加速）变更
 *   不影响依赖方重新编译，适合大型量化系统的模块化开发
 * - 回测框架内核：回测引擎核心逻辑隐藏在 .cpp 中，API 用户只看到稳定的 IBacktester 接口，
 *   内部优化（并行化/缓存策略）不影响上层代码
 *
 * 【本示例说明】
 * Person 类的实现细节（PersonImpl）完全隐藏在 .cpp 文件中。
 * 头文件中只暴露前向声明的 PersonImpl 指针。
 */

#include "pimpl_idiom.hpp"
#include <iostream>
#include <string>

using namespace std;

// PersonImpl 的实现完全隐藏在 .cpp 源文件中
// 客户端只看到头文件中的前向声明，无需知道 greet 的具体实现
class Person::PersonImpl {
public:
  void greet(Person *p);
};

// PersonImpl::greet 的具体实现
// 通过传入的 Person 指针访问其公有成员 name
void Person::PersonImpl::greet(Person *p) {
  cout << "Hello, my name is " << p->name << "." << endl;
}

// Person 的构造函数：在堆上创建 PersonImpl 实例
Person::Person() : impl(new PersonImpl) {}

// Person 的析构函数：释放 PersonImpl 实例
Person::~Person() { delete impl; }

// Person 的 greet 方法：将调用委托给 PersonImpl
// 这种委托是桥接模式的核心 —— 抽象调用被转发到实现
void Person::greet() { impl->greet(this); }

int main() {
  Person p;
  p.name = "Peter";
  p.greet();
  return 0;
}
