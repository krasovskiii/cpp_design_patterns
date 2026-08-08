# cpp_design_patterns

> 本仓库的 C++ 设计模式代码源自 [mpavezb/cpp_design_patterns](https://github.com/mpavezb/cpp_design_patterns/)，该仓库基于 Udemy 课程 *Design Patterns in Modern C++*。
> 本 fork 在保留原代码的基础上，补充了金融工程应用场景注释、重排了学习优先级，并调整了目录结构。

Design patterns are common architectural approaches. They were popularized by the Gang of Four book (1994) for Smalltalk and C++.

> **扩展文档导航**：
> - 👉 [知乎文章：从 C++ 到金融工程](./ZHIHU_ARTICLE.md) —— 教程目的、四梯队思路与开源学习材料
> - 👉 [线程安全与 Modern C++ 优化指南](./THREAD_SAFETY_GUIDE.md) —— 每种设计模式的多方案并发实现详解

---

## 来源与代码改动说明

本仓库是在原仓库基础上的教学 fork。**原代码与示例均保留原作者实现**，改动主要集中在「补充、注释、编排」层面，尽量不改变原有示例的运行逻辑。具体改动如下：

### 1. 文档与结构改动（不涉及示例代码逻辑）


| 改动                 | 说明                                                    |
| ---------------------- | --------------------------------------------------------- |
| 补充金融工程应用注释 | 在部分`.cpp` 文件的头部注释中，加入「金融工程应用」小节 |
| 重排学习优先级       | 新增四梯队（T1~T4）排序章节与学习路线图                 |
| 调整 README 导航     | 各分类 README 新增优先级标记（T1~T4）与返回主目录引导   |
| 补充线程安全指南     | 新增 [THREAD_SAFETY_GUIDE.md](./THREAD_SAFETY_GUIDE.md)，从线程安全与 Modern C++ 两个维度解析每种设计模式 |

### 2. 新增示例代码（仅新增，不覆盖原文件）


| 文件                                             | 说明                                                                                                                                                          |
| -------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `Behavioral/strategy/strategy_functional.cpp`    | 新增一个用`std::function` 实现的轻量版 **Strategy**，演示「不通过继承/多态也能解决算法切换」的思路，并以 BS / Monte Carlo 定价为例，呼应「避免过度设计」章节 |
| `Behavioral/strategy/CMakeLists.txt`             | 为新示例注册 CMake 构建目标`strategy_functional`                                                                                                              |
| `Concurrency/thread_safe_patterns.cpp`           | 新增**线程安全**综合示例，演示 Meyers' Singleton、快照 + `shared_mutex` 的 Observer、`mutex` + `condition_variable` 的命令队列（C++17）                        |
| `Concurrency/multi_strategy.cpp`                 | 新增**同一模式多方案对比**示例：Singleton 四种实现、Observer 两种实现、Strategy 两种实现，均用多线程并发验证                                                  |
| `Concurrency/CMakeLists.txt`                     | 为线程安全示例注册 CMake 目标`thread_safe_patterns` 与 `multi_strategy`                                                                                      |

新增示例编译运行：

```bash
mkdir -p build && cd build
cmake .. && make strategy_functional thread_safe_patterns multi_strategy
./Behavioral/strategy/strategy_functional
./Concurrency/thread_safe_patterns
./Concurrency/multi_strategy
```

### 3. 未改动的部分

- 原有 27 个模式/原则示例的源码逻辑均未改动，保持与原仓库一致。
- 原仓库中 `Structural/facade/` 为空目录（仅有 UML 图），本仓库同样保留其原状，未额外实现。

---

## 教程使用指南（Quick Start）


| 步骤 | 做什么                   | 去哪里                                   |
| :----: | -------------------------- | ------------------------------------------ |
|  1  | 先读全局思路与四梯队排序 | 下方「金融工程视角」章节                 |
|  2  | 用「学习路线图」规划节奏 | 见该章节的学习路线建议                   |
|  3  | 按梯队逐个模式学习       | 索引中带有**T1~T4** 标记，表示所属优先级 |
|  4  | 动手编译运行示例         | 见文末「Building the code」              |
|  5  | 避免过度设计             | 见「过度引入设计模式的代价」章节         |
|  6  | 线程安全与 Modern C++ 优化 | 先看正文「线程安全与 Modern C++ 优化」章节，再深入 [THREAD_SAFETY_GUIDE.md](./THREAD_SAFETY_GUIDE.md) |

---

## 金融工程视角：设计模式学习优先级排序

金融工程（Quantitative Finance / Financial Engineering）涉及交易系统、定价引擎、风控、回测框架、市场数据处理等核心领域。
以下从金融工程的实际应用角度，将 22 个 GoF 设计模式按优先级分为四个梯队，帮助学习者按需取舍、循序渐进。

### Tier 1 — 必修（最常用，构建核心系统的基础）


| 排序 | 模式          | 类别       | 金融工程典型应用                                               |
| :----: | --------------- | ------------ | ---------------------------------------------------------------- |
|  1  | **Strategy**  | Behavioral | 定价模型切换（BS / Monte Carlo / PDE）、执行算法、风险度量计算 |
|  2  | **Observer**  | Behavioral | 行情数据订阅推送、实时风控告警、事件驱动交易                   |
|  3  | **Factory**   | Creational | 创建各类金融工具（期权/期货/互换）、定价模型工厂               |
|  4  | **Decorator** | Structural | 为期权叠加障碍/亚式特征、订单附加止损止盈、数据流加过滤器      |
|  5  | **Singleton** | Creational | 配置管理、行情连接池、交易日历、全局日志                       |

### Tier 2 — 重要（构建健壮系统的高频模式）


| 排序 | 模式                | 类别       | 金融工程典型应用                                            |
| :----: | --------------------- | ------------ | ------------------------------------------------------------- |
|  6  | **Command**         | Behavioral | 订单指令封装、OMS 撤单/改单、回测引擎的操作录制与回放       |
|  7  | **Builder**         | Creational | 结构化产品构建、复杂订单参数组装、FIX 消息构造              |
|  8  | **Template Method** | Behavioral | 回测框架骨架（init→onBar→onTrade→onEnd）、风控流程模板   |
|  9  | **State**           | Behavioral | 订单状态机（New→Pending→Filled→Cancelled）、市场状态切换 |
|  10  | **Adapter**         | Structural | 封装 Bloomberg/Reuters/Tushare 外部 API、归一化异构数据源   |

### Tier 3 — 有用（特定场景下的专业化模式）


| 排序 | 模式                        | 类别       | 金融工程典型应用                                        |
| :----: | ----------------------------- | ------------ | --------------------------------------------------------- |
|  11  | **Proxy**                   | Structural | 行情数据惰性加载、交易系统远程代理、访问权限控制        |
|  12  | **Bridge**                  | Structural | 金融工具与定价引擎的解耦、平台抽象与策略实现分离        |
|  13  | **Chain of Responsibility** | Behavioral | 订单风控校验链路（资金检查→持仓限制→合规审查→执行）  |
|  14  | **Composite**               | Structural | 投资组合树形结构（组合→子组合→头寸→金融工具）        |
|  15  | **Visitor**                 | Behavioral | 对期权结构计算多种 Greeks、组合层面遍历计算各类风险指标 |

### Tier 4 — 按需（特定问题的解决方案）


| 排序 | 模式            | 类别       | 金融工程典型应用                                     |
| :----: | ----------------- | ------------ | ------------------------------------------------------ |
|  16  | **Mediator**    | Behavioral | 中央订单路由、撮合引擎、组件间解耦通信               |
|  17  | **Memento**     | Behavioral | 情景分析快照/恢复、投资组合状态回溯                  |
|  18  | **Iterator**    | Behavioral | 时间序列遍历、持仓列表迭代、Bar 数据遍历             |
|  19  | **Flyweight**   | Structural | 共享大量合约实例的内在属性（合约乘数/到期日/行权价） |
|  20  | **Prototype**   | Creational | 克隆标准合约模板、复制因子配置                       |
|  21  | **Facade**      | Structural | 为复杂定价库提供简洁的估值接口、交易系统统一入口     |
|  22  | **Interpreter** | Behavioral | 交易规则 DSL、FIX/FpML 协议解析、自定义筛选表达式    |

### 学习路线建议

```
SOLID Principles（贯穿始终）
       │
       ▼
┌──────────────────────┐
│  Tier 1 (必修 5 个)   │  ← 先掌握这些，可以搭建最小可用的交易/风控系统
│  Strategy → Observer │
│  Factory → Decorator │
│  → Singleton         │
└──────────┬───────────┘
           ▼
┌──────────────────────┐
│  Tier 2 (重要 5 个)   │  ← 让系统更健壮、可扩展、可维护
│  Command → Builder   │
│  Template → State    │
│  → Adapter           │
└──────────┬───────────┘
           ▼
┌──────────────────────┐
│  Tier 3 (有用 5 个)   │  ← 解决特定领域问题，提升系统抽象层次
│  Proxy → Bridge      │
│  Chain → Composite   │
│  → Visitor           │
└──────────┬───────────┘
           ▼
┌──────────────────────┐
│  Tier 4 (按需 7 个)   │  ← 遇到具体问题时再深入
│  Mediator/Memento    │
│  Iterator/Flyweight  │
│  Prototype/Facade    │
│  /Interpreter        │
└──────────────────────┘
```

> **核心理念**：Tier 1 的模式在金融系统中几乎无处不在，是面试和实际开发的硬通货；
> Tier 2 是你构建可维护系统时的首选武器；
> Tier 3-4 的模式在你遇到特定架构问题时查阅即可，不必死记硬背所有细节。

---

## 过度引入设计模式的代价（反模式警示）

设计模式是「在特定场景下解决问题的经验总结」，而非「必须套用的银弹」。在金融工程系统中，**过早抽象、为模式而模式**往往比不用模式危害更大。以下问题在量化/交易/风控系统的实际开发中尤为常见：

### 1. 过度工程（Over-Engineering），拖慢交付

- 一个本可用 `std::function` 或简单 `if/else` 解决的策略选择，却被套上 Strategy + Factory + Registry + 配置文件四件套。
- **代价**：开发周期翻倍、代码阅读成本高、新人 onboarding 困难。量化策略迭代快，过度抽象会直接拖慢因子/策略上线速度。

### 2. 抽象层级过深，性能损耗不可接受

- 金融系统对延迟极度敏感（低延迟交易、实时风控）。过度嵌套的 Decorator / Proxy / Adapter 会增加虚函数调用、间接层和内存分配。
- **代价**：在纳秒/微秒级路径上，每一层抽象都是真金白银的延迟。高频交易系统尤其需要「裸写热点路径」，而非堆模式。

### 3. 维护地狱：类爆炸与「改一处动全身」

- 一个定价流程被拆成 Observer + Mediator + Command + Chain of Responsibility + Visitor 五层，任何一次需求变更都要翻遍十几个文件。
- **代价**：金融规则（监管、交易品种、风控阈值）频繁变动，过度模式化的代码让变更成本指数级上升，bug 也更难定位。

### 4. 隐藏的复杂度，掩盖真实的业务逻辑

- 模式本身是手段，业务（定价、对冲、结算）才是目的。当读者需要理解三个设计模式才能看懂一行定价逻辑时，代码已经本末倒置。
- **代价**：领域专家无法 review 代码，风险隐患埋在间接调用中，回测与实盘行为可能不一致。

### 5. 误用模式，引入错误语义

- 用 Singleton 管理可变全局状态（如持仓、行情缓存）→ 并发下竞态、难以测试、难以回滚。
- 用 State 模式描述本应数据驱动的流程 → 状态转移硬编码，无法由配置热更新。
- **代价**：在金融系统里，一个错误的语义抽象可能导致错单、超额敞口或合规事故。

### 如何把握「度」——金融工程中的实用准则


| 准则                                  | 说明                                                               |
| --------------------------------------- | -------------------------------------------------------------------- |
| **YAGNI（You Aren't Gonna Need It）** | 只为实现中确实存在的第二/第三个变化点而抽象，不要预判未来需求      |
| **先写直白代码，再重构出模式**        | 当同一段`if/else` 或重复逻辑第三次出现时，再考虑提取模式           |
| **性能路径保持简单**                  | 延迟敏感代码优先用值语义、模板、内联，避免运行时多态               |
| **让领域逻辑可读优先**                | 模式服务于可维护性，而非相反；无法让量化研究员看懂的模式就是坏模式 |
| **用 SOLID 而非堆 GoF**               | 很多时候遵循 SOLID 原则比生硬套用某个 GoF 模式更能解决问题         |

> **小结**：本仓库的学习优先级（Tier 1~4）建议「按需取用」。Tier 1 是高频刚需，但要克制地用；Tier 3-4 更要「遇问题再查，不主动上」。记住：**最好的设计，是让业务清晰可见、让系统易于变更、让性能满足要求——模式只是达到这些目标的工具。**

---

## 线程安全与 Modern C++ 优化

> 金融系统的并发特性（行情推送、订单撮合、多策略并行）让「线程安全」成为设计模式落地的硬要求。
> 本节提炼核心方法；每种模式的**多方案详解与代码对比**见 **[THREAD_SAFETY_GUIDE.md](./THREAD_SAFETY_GUIDE.md)**。

### 线程安全的五大策略

所有设计模式的线程安全实现，本质上逃不出以下五种策略：

| 策略 | 核心思想 | 优点 | 缺点 | 适用模式 |
|------|----------|------|------|----------|
| **A. 无状态/不可变** | 对象不持有可变状态，或构造后不可变 | 天然线程安全、零锁 | 无法表达有状态逻辑 | Template、Visitor、Memento、Interpreter |
| **B. 锁** | `mutex` / `shared_mutex` 保护共享状态 | 通用、正确 | 竞争开销、可能死锁 | Factory、Mediator、Composite |
| **C. 原子操作** | 用 `atomic` 原子更新单个指针/整数 | 无锁、低延迟 | 仅适用于单变量 | Strategy、State、Singleton |
| **D. 快照/写时复制** | 读拿快照、释放锁再处理 | 可重入、防回调死锁 | 拷贝有开销 | Observer、Mediator、Iterator |
| **E. 线程本地/隔离** | 每线程独立实例，不共享 | 完全无竞争 | 状态不共享、内存多 | Builder、Prototype、Chain |

> 关键认知：**同一模式在不同负载下应选不同策略**，没有「唯一正确」的实现，只有「最适合当前场景」的取舍。

### 常用 Modern C++ 并发工具速查

| 工具 | 用途 |
|------|------|
| `std::mutex` + `lock_guard` | 互斥锁（RAII，防忘解锁） |
| `std::shared_mutex` + `shared_lock` | 读写锁：读多写少场景（C++17） |
| `std::atomic<T>` / `atomic_load`/`atomic_store` | 无锁原子更新单变量 |
| `std::call_once` / `once_flag` | 线程安全的一次性初始化 |
| `std::condition_variable` | 生产者-消费者（命令队列） |
| `std::scoped_lock` | 一次锁多把锁，避免死锁 |
| `shared_ptr<const T>` | 只读共享，`const` 即线程安全 |
| `thread_local` | 每线程独立状态 |

### 多方案决策步骤

面对一个模式的并发需求，按以下顺序决策：

1. **能否消灭共享可变状态？** → 能则用「不可变 / 无状态 / thread_local / 值语义」，零锁最优。
2. **是读多写少吗？** → 用 `shared_mutex`（有锁但读并发），或 Copy-on-Write（读无锁、写贵）。
3. **是切换/替换单个对象吗？** → 用 `atomic_load/store(shared_ptr)` 或 `atomic` 指针，无锁。
4. **是生产-消费 / 严格串行吗？** → 用 `condition_variable` 队列（有界加背压）。
5. **是通知/回调吗？** → 先快照、释放锁、再回调（防死锁/迭代器失效）。
6. **都不适用？** → 用 `mutex` + `lock_guard`（通用兜底）。

### 各模式方案一览

| 模式 | 方案数 | 推荐方案 | 备选方案 |
|------|:---:|----------|----------|
| Singleton | 4 | Meyers'（函数局部 static） | call_once / atomic 指针 / static 成员 |
| Factory | 3 | shared_mutex 注册表 | Copy-on-Write / const 只读注册表 |
| Builder | 3 | 值语义 + `&&` | thread_local / 加锁 |
| Prototype | 3 | 不可变原型 + 深拷贝 | shared_ptr<const> 注册表 / thread_local |
| Adapter | 3 | 只读共享底层 | 转发锁 / 无状态 |
| Bridge | 3 | 抽象/实现分离（各自独立线程安全） | 外部串行 / 线程本地桥接 |
| Composite | 3 | shared_mutex + unique_ptr | Copy-on-Write / recursive_mutex |
| Decorator | 3 | 不可变装饰链 | 函数式 / 内部锁 |
| Proxy | 3 | call_once 懒加载 | atomic CAS / 计数代理 |
| Façade | 3 | 无状态编排 | 缓存+锁 / 子系统安全 |
| Flyweight | 3 | shared_mutex 驻留表 | 不可变建池 / thread_local |
| **Observer** | 4 | 快照 + shared_mutex | atomic 单观察者 / recursive_mutex / 异步队列 |
| **Command** | 4 | mutex + condition_variable | 有界队列 / 无锁 SPSC / async |
| State | 3 | atomic 状态指针 | mutex / 只读规则表 |
| Strategy | 3 | shared_ptr + atomic_load | mutex / std::function 无状态 |
| Template Method | 3 | const 钩子 | 外部串行 / 组合阶段 |
| Mediator | 3 | 快照广播 | 单线程队列 / 无状态 |
| Memento | 3 | 不可变快照 | shared_ptr<const> / 撤销栈+锁 |
| Iterator | 3 | 锁内遍历 | Copy-on-Write / scoped_lock 多容器 |
| Chain of Resp. | 3 | 无状态处理器 | 有状态+锁 / thread_local |
| Visitor | 3 | 无状态访问者 | 结果分区 / shared_mutex 遍历 |
| Interpreter | 3 | 不可变 AST | thread_local / 情景并行 |

### 最终原则

1. **消灭共享可变状态** 永远优于「加锁保护」——不可变对象、值语义、无状态是线程安全的第一选择。
2. 必须共享可变状态时，用 **RAII 锁**（`lock_guard`/`shared_lock`）缩小持锁范围。
3. 能用 `atomic` 原子切换单个指针/整数时，不要用 `mutex`（无锁、更高效）。
4. 回调/用户代码**绝不在持锁时调用**，先快照再执行（避免死锁）。
5. **多把锁用 `std::lock`/`scoped_lock`** 一次获取，避免死锁。

**可运行示例**：
- `Concurrency/thread_safe_patterns.cpp`：Meyers' Singleton + 快照 Observer + 命令队列
- `Concurrency/multi_strategy.cpp`：Singleton 四种 / Observer 两种 / Strategy 两种方案的实测对比

---

## Gamma Categorization

They are tipically split into three categories. This is called *Gamma Categorization* after Erich Gamma, one of the GoF authors.

### Creational Patterns

- Deal with the creation of objects.
- Explicit vs. Implicit: Constructor vs. [Dependency Injection, reflection, ...].
- Wholesale vs. piecewise initialization.

### Structural Patterns

- Concerned with the structure (e.g., class members).
- Many patterns are wrapers that mimic the underlying class interface.
- Stress the importance of godd API design. **Make API usable for other people**.

### Behavioral Patterns

- They are all different; no central theme.
- They solve particular problems.

## Principles and Patterns

> **优先级标记**：`[T1]`~`[T4]` 表示该模式在「金融工程视角」下的学习优先级梯队。
> 建议按 T1 → T2 → T3 → T4 顺序学习，T4 可按需查阅。

+ [SOLID Principles](./SOLID/README.md) — 贯穿始终，所有模式的基石
  - [Single Responsibility](./SOLID/README.md#single-responsibility)
  - [Open-Closed](./SOLID/README.md#open-closed)
  - [Liskov Substitution](./SOLID/README.md#liskov-substitution)
  - [Interface Segregation](./SOLID/README.md#interface-segregation)
  - [Dependency Inversion](./SOLID/README.md#dependency-inversion)
+ [Creational Patterns](./Creational/README.md)
  - [Builder](./Creational/README.md#builder--t2) — `[T2]`
  - [Factory](./Creational/README.md#factory--t1) — `[T1]`
  - [Prototype](./Creational/README.md#prototype--t4) — `[T4]`
  - [Singleton](./Creational/README.md#singleton--t1) — `[T1]`
+ [Structural Patterns](./Structural/README.md)
  - [Adapter](./Structural/README.md#adapter--t2) — `[T2]`
  - [Bridge](./Structural/README.md#bridge--t3) — `[T3]`
  - [Composite](./Structural/README.md#composite--t3) — `[T3]`
  - [Decorator](./Structural/README.md#decorator--t1) — `[T1]`
  - [Façade](./Structural/README.md#façade--t4) — `[T4]`
  - [Flyweight](./Structural/README.md#flyweight--t4) — `[T4]`
  - [Proxy](./Structural/README.md#proxy--t3) — `[T3]`
+ [Behavioral Patterns](./Behavioral/README.md)
  - [Chain of Responsibility](./Behavioral/README.md#chain-of-responsibility--t3) — `[T3]`
  - [Command](./Behavioral/README.md#command--t2) — `[T2]`
  - [Interpreter](./Behavioral/README.md#interpreter--t4) — `[T4]`
  - [Iterator](./Behavioral/README.md#iterator--t4) — `[T4]`
  - [Mediator](./Behavioral/README.md#mediator--t4) — `[T4]`
  - [Memento](./Behavioral/README.md#memento--t4) — `[T4]`
  - [Observer](./Behavioral/README.md#observer--t1) — `[T1]`
  - [State](./Behavioral/README.md#state--t2) — `[T2]`
  - [Strategy](./Behavioral/README.md#strategy--t1) — `[T1]`
  - [Template Method](./Behavioral/README.md#template-method--t2) — `[T2]`
  - [Visitor](./Behavioral/README.md#visitor--t3) — `[T3]`

## Building the code

```bash
mkdir -p build && cd build
cmake -DCMAKE_INSTALL_PREFIX=../bin .. && make
make install
```
