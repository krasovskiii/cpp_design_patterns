# SOLID Design Principles

Useful principles of Obejct Oriented Design. These were introduced by Uncle Bob (Robert C. Martin).

> **学习引导**：SOLID 五大原则是所有设计模式的**基石**，建议贯穿整个学习过程、优先掌握。
> 返回主目录：[README](../README.md) · 查看学习路线：[金融工程视角四梯队](../README.md#金融工程视角设计模式学习优先级排序)

---

## STUPID vs SOLID


### STUPID 糟糕代码六大反模式
| 字母 | 全称                    | 释义                                   | 核心口诀 | 带来的问题 |
| :-: | :---------------------- | :------------------------------------- | :--------: | :-------- |
| S    | Singleton              | 滥用单例模式，全局唯一实例随处调用 | 单例泛滥 | 隐藏依赖，单元测试困难，状态污染 |
| T    | Tight Coupling         | 紧耦合，类与实现直接硬绑定 | 强绑定 | 修改一处，连锁引发多处bug |
| U    | Untestability          | 代码难以编写单元测试 | 难于测试 | 依赖硬编码、外部资源，自动化测试无法开展 |
| P    | Premature Optimization | 过早优化，在无性能瓶颈时提前优化 | 过早优化 | 增加代码复杂度，浪费开发时间 |
| I    | Indescriptive Naming   | 命名模糊、语义不清 | 命名混乱 | 可读性差，他人无法快速理解代码意图 |
| D    | Duplication            | 大量重复代码，复制粘贴开发 | 代码冗余 | 逻辑难以统一维护，修改容易遗漏 |

### SOLID 面向对象设计五大原则
| 字母 | 全称                    | 释义                                   | 核心口诀 | 违反后果 |
| :-: | :---------------------- | :------------------------------------- | :--------: | :-------- |
| S    | Single Responsibility  | 单一职责, 一个类只做一件事             | 一类一事 | 类臃肿，修改一处多处受影响 |
| O    | Open-close             | 开闭原则, 对扩展开放，对修改关闭       | 扩不改旧 | 需求新增就要改动原有代码 |
| L    | Liskov substitution    | 里氏替换, 子类型必须能够替换父类型     | 子类替父 | 继承体系混乱，出现异常行为 |
| I    | Interface segregation | 接口隔离, 避免大而全的接口             | 接口要小 | 实现类被迫实现无用方法 |
| D    | Dependency inversion   | 依赖倒置, 依赖于抽象而不是具体实现     | 依赖抽象 | 紧耦合，难以单元测试、替换组件 |


## Single Responsibility

A class should have a single reason to change. - **Separation of Concerns**

## Open-Closed

The system should be **open to extension and closed to modification**.

- We want to avoid modifying code that already works.
- Extension can be done by inheritance.

## Liskov Substitution

Subtypes should be inmediatly substitutable by their base types.

- Methods designed to work on the Base class should keep working
  when a Derived instance is used.
- Named after Robert Liskov.

## Interface Segregation

Avoid putting too much into a single interface. Don't force
implementers to code too large interfaces.

- Partial implementers contain dummy methods and extra code.
- Partial implementers give a wrong API to the user.

## Dependency Inversion

1. High-level modules should not depend on low-level modules. Both should depend on abstractions.
2. Abstractions should not depend on details. Details should depend on abstractions.

Prefer depending on abstractions (abstract and interfaces) rather than specific classes. *Avoid depending on the implementation details*.
