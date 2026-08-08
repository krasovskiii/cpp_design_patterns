/*
 * 设计模式：桥接模式 / Pimpl 惯用法（Bridge Pattern / Pimpl Idiom）
 * 核心思想：将类的实现细节隐藏在 .cpp 文件中，头文件只暴露最小接口。
 * 适用场景：减少编译依赖、隐藏实现细节、提高二进制兼容性（ABI stability）。
 *
 * Person 类的头文件：只包含公开接口和实现类的前向声明。
 * 客户端代码看不到 PersonImpl 的具体实现，也无法访问 secret_internal_method。
 */

#include <string>

class Person {
public:
  std::string name;

  // 实现类的前向声明 —— 客户端不知道 PersonImpl 的具体内容
  class PersonImpl;
  PersonImpl *impl;

  Person();
  ~Person();

  void greet();

  // Pimpl 惯用法帮助我们隐藏不想暴露的 API 组件。
  // 即使声明为 private，头文件使用者仍能看到其存在。
  // 通过将实现细节移到 .cpp 中，真正实现了信息隐藏。
private:
  void secret_internal_method();
};
