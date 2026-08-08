/*
 * =============================================================================
 * 设计模式：桥接模式 / Pimpl 惯用法（Bridge Pattern / Pimpl Idiom）—— 泛型模板库
 * =============================================================================
 *
 * 【一句话概括】
 * 将实现细节隐藏在 .cpp 文件中，使用泛型模板简化 Pimpl 惯用法的实现。
 *
 * 【适用场景】
 * - 当多个类都需要使用 Pimpl 惯用法隐藏实现时，通过模板库统一管理
 *
 * 【金融工程应用】
 * - 量化平台 SDK 发布：交易 API、行情 API、风控 API 统一使用 pimpl 模板，
 *   对外只暴露稳定的接口头文件，内部实现升级不影响客户
 *
 * 【本示例说明】
 * 实现了一个泛型 pimpl<T> 模板类，封装了 unique_ptr 的创建和访问。
 */

#include "pimpl_library.h"
#include <utility>

// -----------------------------------------------------------------------------
// PIMPL 模板库实现
// -----------------------------------------------------------------------------

// 默认构造函数：在堆上创建 T 类型的实例
template <typename T> pimpl<T>::pimpl() : impl{new T{}} {}

// 析构函数：unique_ptr 自动管理内存释放
template <typename T> pimpl<T>::~pimpl() {
  // destruction is handled by unique_ptr
}

// 可变参数模板构造函数：支持将任意参数转发给 T 的构造函数
template <typename T>
template <typename... Args>
pimpl<T>::pimpl(Args &&... args) : impl{new T{std::forward<Args>(args)...}} {}

// 箭头操作符：提供对底层实现对象的指针式访问
template <typename T> T *pimpl<T>::operator->() { return impl.get(); }

// 解引用操作符：提供对底层实现对象的引用式访问
template <typename T> T &pimpl<T>::operator*() { return *impl.get(); }

// -----------------------------------------------------------------------------
// 使用示例
// -----------------------------------------------------------------------------

// 在头文件（.h）中的声明：
class Foo {
  class Impl;          // 前向声明实现类
  pimpl<Impl> impl;    // 使用 pimpl 模板管理实现对象
                       // 只需实现 Foo::Impl 类即可完成 Pimpl 模式
};

// 在源文件（.cpp）中定义 Foo::Impl：
class Foo::Impl {};
