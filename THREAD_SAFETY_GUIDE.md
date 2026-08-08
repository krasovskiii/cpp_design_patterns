# 设计模式：线程安全与 Modern C++ 优化指南（全方式版）

> 本文档从**线程安全**与 **Modern C++ 特性**两个维度，对仓库中每种设计模式给出**多种可选的线程安全实现方式**，
> 并配以「优化过程 → 解析 → 多方案对比」结构，帮助你在不同并发场景下做出正确取舍。
>
> 前置阅读：[README 学习路线](./README.md#金融工程视角设计模式学习优先级排序)

---

## 一、贯穿全局的 Modern C++ 并发工具

| 工具 | 头文件 | 用途 | 适用模式 |
|------|--------|------|----------|
| `std::mutex` / `std::lock_guard` / `std::unique_lock` | `<mutex>` | 互斥锁，保护共享数据 | Singleton、Factory、Mediator 等 |
| `std::atomic<T>` / `std::atomic_load` / `std::atomic_store` | `<atomic>` | 无锁原子变量 | Singleton、Strategy、State、Proxy |
| `std::shared_mutex` / `std::shared_lock` | `<shared_mutex>` | 读写锁（多读单写） | Composite、Observer、Factory |
| `std::recursive_mutex` | `<mutex>` | 可重入锁（同线程可重复加锁） | Observer、Composite（回调再触发） |
| `std::call_once` / `std::once_flag` | `<mutex>` | 线程安全的一次性初始化 | Singleton、Factory、Proxy |
| `std::jthread` / `std::thread` | `<thread>` | 线程管理 | 各模式并发客户端 |
| `std::condition_variable` | `<condition_variable>` | 条件变量（生产者-消费者） | Command、Mediator |
| `std::lock` / `std::scoped_lock` | `<mutex>` | 一次锁多个互斥锁，避免死锁 | Composite、Mediator |
| `std::shared_ptr` 的 `atomic_load` | `<memory>` | 无锁读共享指针（读写分离） | Strategy、State |
| `std::async` | `<future>` | 异步执行 | 各模式并发客户端 |
| `const` 成员函数 | — | 只读操作天然线程安全 | 所有模式 |
| `unique_ptr` / `shared_ptr` | `<memory>` | RAII 资源管理 | 所有持有资源的模式 |
| Copy-on-Write（写时复制） | — | 读无锁，写时复制整个容器 | Iterator、Composite |

**核心原则（Effective C++ + 并发）**：
1. 只有**共享的可变状态**才需要加锁；`const` 成员函数、局部变量天然安全。
2. 优先用 `lock_guard`/`scoped_lock`（RAII），避免忘记解锁。
3. 读多写少用 `shared_mutex`，读多且要求无锁用 `atomic`。
4. 多把锁时用 `std::lock` 一次获取，避免死锁。
5. **消灭共享可变状态** > 加锁 > 原子操作：先消除共享，其次锁，最后才考虑原子技巧。

### 线程安全的五大实现策略（贯穿所有模式）

每种设计模式的线程安全实现，本质上逃不出以下五种策略。后文每种模式都会给出其中 2~4 种：

| 策略 | 核心思想 | 优点 | 缺点 | 适用 |
|------|----------|------|------|------|
| **A. 无状态/不可变** | 对象不持有可变状态，或构造后不可变 | 天然线程安全、零锁开销 | 不能表达有状态逻辑 | Template、Visitor、Memento、Interpreter |
| **B. 锁（mutex/shared_mutex）** | 用互斥锁保护共享可变状态 | 通用、简单、正确 | 有竞争开销、可能死锁 | Factory、Mediator、Composite |
| **C. 原子操作** | 单个指针/整数用 atomic 原子更新 | 无锁、低延迟 | 只适用于单变量 | Strategy、State、Singleton |
| **D. 快照/写时复制** | 读拿快照、释放锁再处理 | 可重入、避免回调死锁 | 拷贝有开销 | Observer、Mediator、Iterator |
| **E. 线程本地存储/实例隔离** | 每线程持有独立实例，不共享 | 完全无竞争 | 状态不共享、内存增多 | Builder、Prototype、Chain |

> 关键认知：**同一模式在不同业务负载下，应选不同策略**。没有「唯一正确」的线程安全实现，只有「最适合当前场景」的取舍。

---

## 二、创建型模式（Creational）

### 1. Singleton（单例）— Tier 1

**线程安全问题**：懒加载的 `get()` 可能被多线程同时进入导致重复创建；经典 DCLP 在 C++11 前是错的（可能读到半初始化对象）。Singleton 有**四种**主流线程安全实现。

#### 方式 A：Meyers' Singleton（函数局部 static）— ✅ 首选

```cpp
class Singleton {
public:
  static Singleton& get() {
    static Singleton instance;  // C++11 保证：函数局部 static 初始化线程安全
    return instance;
  }
  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;
private:
  Singleton() = default;
};
```

**解析**：C++11 强制编译器为「函数内 `static` 局部对象初始化」插入同步。零手写锁、初始化后零开销、懒加载。**缺点**：初始化时机不可控，构造函数不能带参数。

#### 方式 B：`std::call_once` + 显式指针（可控初始化）

```cpp
class Singleton {
  inline static Singleton* inst_ = nullptr;
  inline static std::once_flag flag_;
public:
  static Singleton& get() {
    std::call_once(flag_, [] { inst_ = new Singleton(/* 可传参 */); });
    return *inst_;
  }
};
```

**解析**：适合**需要参数或复杂初始化**的单例（lambda 里可传参）。`call_once` 原子且可重入安全。**缺点**：每次 `get()` 有 `once_flag` 检查开销，需手动 `delete`。

#### 方式 C：`std::atomic` 指针 + 双重检查（无锁快速路径）

```cpp
class Singleton {
  inline static std::atomic<Singleton*> inst_{nullptr};
public:
  static Singleton& get() {
    Singleton* p = inst_.load(std::memory_order_acquire);
    if (!p) {                       // 快速路径：无锁
      static Singleton instance;    // 兜底：Meyers 保证只构造一次
      p = &instance;
      inst_.store(p, std::memory_order_release);
    }
    return *p;
  }
};
```

**解析**：用**原子指针**作「已初始化」标志，快速路径是纯原子读（无锁极快），初始化交给 Meyers。内存序 `acquire/release` 确保「完全构造后再发布」。**缺点**：代码较复杂。

#### 方式 D：`static` 成员对象（非懒加载）

```cpp
class Singleton {
  static Singleton instance_;   // main 前的静态初始化期构造
public:
  static Singleton& get() { return instance_; }
  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;
};
Singleton Singleton::instance_;
```

**解析**：**非懒加载**，在静态初始化期构造（单线程阶段），天然线程安全。**缺点**：无法懒加载，有静态初始化顺序问题。适合必用的轻量单例（如日志器）。

**四方式对比**：

| 方式 | 懒加载 | 线程安全 | 初始化可控 | 开销 | 适用 |
|------|:---:|:---:|:---:|------|------|
| A. Meyers | ✅ | ✅ | ❌ | 零 | **绝大多数** |
| B. call_once | ✅ | ✅ | ✅ | 每次有检查 | 需参数/复杂构造 |
| C. atomic 指针 | ✅ | ✅ | ✅ | 极低 | 高频访问 |
| D. static 成员 | ❌ | ✅ | ❌ | 零 | 必用轻量单例 |

> **注意**：以上只保证**构造**线程安全。若单例持有**可变共享状态**，对其成员的并发访问**仍需 `mutex`**——这是独立于「如何构造单例」的另一层问题。

---

### 2. Factory（工厂）— Tier 1

**线程安全问题**：工厂常维护「注册表」（`map<string, factory_fn>`），多线程同时注册/查询会产生竞态。有三种实现方式。

#### 方式 A：`std::shared_mutex` 保护注册表（读多写少）— ✅ 常用

```cpp
class ModelFactory {
  using Registry = std::map<std::string, std::function<Model()>>;
  static Registry& reg() { static Registry m; return m; }
  static std::shared_mutex& mtx() { static std::shared_mutex m; return m; }
public:
  static void register_model(std::string name, std::function<Model()> f) {
    std::unique_lock lk(mtx());              // 独占写
    reg()[std::move(name)] = std::move(f);
  }
  static Model create(const std::string& name) {
    std::shared_lock lk(mtx());              // 共享读：多线程并发查询
    auto it = reg().find(name);
    if (it == reg().end()) throw std::runtime_error("unknown model");
    return it->second();
  }
};
```

**解析**：注册少、查询多 → `shared_mutex` 让读并发。`std::function` 作为工厂回调，免去子类。**缺点**：仍有一把锁，读并发有缓存行竞争。

#### 方式 B：Copy-on-Write 注册表（读无锁）

```cpp
class ModelFactory {
  // 用 shared_ptr 持有只读快照
  using Registry = std::map<std::string, std::function<Model()>>;
  static std::shared_ptr<const Registry>& snapshot() {
    static std::shared_ptr<const Registry> s = std::make_shared<Registry>();
    return s;
  }
  static std::mutex& mtx() { static std::mutex m; return m; }
public:
  static Model create(const std::string& name) {
    auto reg = snapshot();      // 读：无锁，拿到快照
    auto it = reg->find(name);  // 快照是 const，可无锁读
    return it == reg->end() ? throw std::runtime_error("unknown") : it->second();
  }
  static void register_model(std::string name, std::function<Model()> f) {
    std::lock_guard lk(mtx());               // 写：独占
    auto fresh = std::make_shared<Registry>(*snapshot());  // 复制
    (*fresh)[std::move(name)] = std::move(f);
    snapshot() = std::move(fresh);           // 原子发布新快照
  }
};
```

**解析**：读线程拿**共享快照**（`shared_ptr<const>`）无锁遍历；写线程复制整个注册表、改、再原子发布。读完全无锁，适合**极高读并发**。**缺点**：每次写要复制整张表，写开销大。

#### 方式 C：注册后不可变（启动时注册，运行只读）

```cpp
class ModelFactory {
  inline static const Registry reg_ = build_registry();  // 启动时一次性构建
public:
  static Model create(const std::string& name) {
    auto it = reg_.find(name);   // const 只读：天然线程安全
    return it == reg_.end() ? throw ... : it->second();
  }
};
```

**解析**：若工厂注册表在**启动阶段构建、运行期只读**，则直接用 `const` 容器，**零锁**。这是最优解——通过设计消除「运行期写」的需求。

**三方式对比**：

| 方式 | 读并发 | 写开销 | 锁 | 适用 |
|------|:---:|:---:|:---:|------|
| A. shared_mutex | ✅ 并发 | 小 | 一把读写锁 | 注册+查询并存 |
| B. Copy-on-Write | ✅ 无锁 | 大（复制整表） | 写锁 | 极高读、写极少 |
| C. const 注册表 | ✅ 无锁 | 无（一次性） | 无 | 启动注册、运行只读 |

---

### 3. Builder（建造者）— Tier 2

**线程安全问题**：Builder 是有状态的可变构建器，不适合并发共享。正确做法是**不共享**。有两种方式。

#### 方式 A：值语义 + `&&` ref-qualifier（每线程独立构建）— ✅ 推荐

```cpp
struct HtmlElement { std::string tag, text; std::vector<HtmlElement> children; };
class HtmlBuilder {
  HtmlElement root;
public:
  HtmlBuilder add_child(std::string tag, std::string text) && {
    root.children.push_back(HtmlElement{std::move(tag), std::move(text), {}});
    return std::move(*this);   // 返回新对象，无共享状态
  }
  HtmlElement build() && { return std::move(root); }
};
```

**解析**：用「值语义 + 移动语义」让**每个线程独立构建**，完全不共享状态 → 天然线程安全。`&&` 限定链式方法在右值上调用，强制一次性构建，避免意外共享。

#### 方式 B：线程本地 Builder（thread_local）

```cpp
// 每个线程持有自己的 Builder 实例
thread_local HtmlBuilder builder;
HtmlElement my_element() {
  // 每个线程访问的是自己那份 builder，无竞争
  return std::move(builder).add_child("td", "x").build();
}
```

**解析**：`thread_local` 让每个线程有独立实例，**彻底无竞争**。适合「每线程都要构建、但希望复用实例避免重复构造」的场景。**缺点**：各线程状态不共享，若需要汇总需额外同步。

#### 方式 C：Builder 加锁（不推荐，仅当必须共享）

```cpp
class SharedBuilder {
  std::mutex mtx_;
  HtmlElement root_;
public:
  void add_child(...) { std::lock_guard lk(mtx_); /* 修改 root_ */ }
};
```

**解析**：若必须多线程共享同一个 Builder，则需加锁。但**通常不推荐**——Builder 本质是局部构建工具，加锁违背其设计初衷（串行化构建失去意义）。

**三方式对比**：

| 方式 | 是否共享 | 竞争 | 适用 |
|------|:---:|:---:|------|
| A. 值语义 | 否 | 无 | **推荐**，独立构建 |
| B. thread_local | 否 | 无 | 每线程复用实例 |
| C. 加锁 | 是 | 有 | 极少，必须共享时 |

---

### 4. Prototype（原型）— Tier 2

**线程安全问题**：原型对象被多线程克隆。若克隆是**深拷贝（只读原型）**，则原型可只读共享；若克隆修改原型，则需保护。有两种方式。

#### 方式 A：不可变原型 + 深拷贝（只读共享）— ✅ 推荐

```cpp
struct Contact {
  std::string name;
  std::unique_ptr<Address> address;
  // 深拷贝：只读 other，不修改原型 → 多线程可并发克隆同一原型
  Contact(const Contact& o)
      : name(o.name), address(std::make_unique<Address>(*o.address)) {}
  Contact(Contact&&) noexcept = default;
  Contact& operator=(Contact&&) noexcept = default;
};
```

**解析**：把原型设计为**不可变**（构造后不修改），深拷贝构造**只读**原型。多线程并发 `clone(prototype)` 时，原型是只读共享，**无需锁**。`unique_ptr` + 完整拷贝/移动语义杜绝浅拷贝 double-free。

#### 方式 B：原型工厂 + `shared_ptr<const>` 缓存

```cpp
class PrototypeRegistry {
  // 原型以只读共享形式存放
  std::map<std::string, std::shared_ptr<const Contact>> protos_;
  mutable std::shared_mutex mtx_;
public:
  void register_proto(std::string name, std::shared_ptr<const Contact> p) {
    std::unique_lock lk(mtx_);
    protos_[std::move(name)] = std::move(p);
  }
  // 克隆：读原型表（共享锁）+ 深拷贝，原型只读
  Contact clone(const std::string& name) const {
    std::shared_lock lk(mtx_);
    return Contact(*protos_.at(name));   // 深拷贝
  }
};
```

**解析**：原型注册表读多写少 → `shared_mutex`。注册表里的原型是 `shared_ptr<const>`，克隆时深拷贝只读原型。把「注册」和「克隆」分离，克隆并发安全。

#### 方式 C：thread_local 原型（每线程持有克隆）

```cpp
// 每线程持有原型的一份克隆，彻底隔离
thread_local Contact per_thread_proto;
Contact make_contact() {
  return Contact(per_thread_proto);  // 深拷贝自己那份，无共享
}
```

**解析**：用 `thread_local` 让每线程持有独立原型副本，零竞争。适合「每个线程频繁克隆」且能容忍副本内存的场景。

**三方式对比**：

| 方式 | 原型共享 | 锁 | 适用 |
|------|:---:|:---:|------|
| A. 不可变+深拷贝 | 只读共享 | 无 | **推荐**，并发克隆 |
| B. 注册表+shared_ptr<const> | 注册表共享 | shared_mutex | 需管理多个原型 |
| C. thread_local | 每线程副本 | 无 | 每线程高频克隆 |

---

## 三、结构型模式（Structural）

### 5. Adapter（适配器）— Tier 2

**线程安全问题**：Adapter 是**薄包装**，若内部被适配对象线程安全，则 Adapter 继承之；否则需转发锁。有三种方式。

#### 方式 A：内部对象不可变/只读共享（无锁）— ✅ 优先

```cpp
class RectangleAdapter : public Rectangle {
  std::shared_ptr<const LegacyRectangle> impl_;  // 只读共享底层
public:
  explicit RectangleAdapter(std::shared_ptr<const LegacyRectangle> impl)
      : impl_(std::move(impl)) {}
  void draw() const override { impl_->oldDraw(); }  // 只读调用
};
```

**解析**：若被适配对象**只读可共享**（如只读转换表、不可变配置），用 `shared_ptr<const>` 让 Adapter 无锁线程安全。**组合优于继承**（Effective C++ 条款 22），便于原子替换底层。

#### 方式 B：内部对象可变 → Adapter 转发锁

```cpp
class RectangleAdapter : public Rectangle {
  std::unique_ptr<LegacyRectangle> impl_;
  mutable std::mutex mtx_;          // 转发锁
public:
  void draw() const {
    std::lock_guard lk(mtx_);       // 串行化底层访问
    impl_->oldDraw();
  }
};
```

**解析**：若被适配对象**可变且非线程安全**，Adapter 在转发时加锁。**缺点**：串行化，并发度低。**改进**：若操作是「读多写少」，改用 `shared_mutex`。

#### 方式 C：无状态 Adapter（线程安全且可复用）

```cpp
// Adapter 无成员状态，只做参数转换/转发，可被多线程安全共享
class StatelessAdapter {
public:
  // 每个调用都是独立的（无共享可变状态）
  static Point to_point(const LegacyRect& r) { /* 纯函数转换 */ }
};
```

**解析**：把 Adapter 设计为**无状态纯函数**（只做接口转换，不持有可变状态），则天然线程安全，可全局共享。这是最理想形态。

**三方式对比**：

| 方式 | 底层状态 | 锁 | 适用 |
|------|:---:|:---:|------|
| A. 只读共享 | 不可变 | 无 | 只读转换/配置 |
| B. 转发锁 | 可变 | mutex/shared_mutex | 底层可变 |
| C. 无状态 | 无 | 无 | 纯接口转换 |

---

### 6. Bridge（桥接）— Tier 3

**线程安全问题**：Bridge 把「抽象」与「实现」分离（如金融工具抽象与定价引擎实现解耦）。桥接的并发关键在**抽象层与实现层各自独立线程安全**，以及「实现」能否被安全替换。有三种方式。

#### 方式 A：抽象/实现各自独立锁（分层线程安全）— ✅ 推荐

```cpp
// 抽象层与实现层各自独立加锁，互不干扰
class PricingEngine {          // 实现层（可变，有状态）
  std::mutex mtx_;
  double model_params_;
public:
  void set_params(double p) { std::lock_guard lk(mtx_); model_params_ = p; }
  double price(double spot) const { std::lock_guard lk(mtx_); /* 计算 */ }
};
class Option {                 // 抽象层（持有实现引用）
  PricingEngine& engine_;      // 通过引用桥接到实现
public:
  double value(double spot) const { return engine_.price(spot); }  // 委托
};
```

**解析**：**分层线程安全**——抽象层和实现层各自管理自己的锁，谁有可变状态谁加锁。抽象层若只做委托（无状态），本身无需锁；实现层有状态则加锁。这是 Bridge 的并发最佳实践：**每层只对自己的状态负责**。

#### 方式 B：共享实现 + `shared_ptr<const>`（只读共享）

```cpp
// 若定价引擎实现是"不可变配置 + 无状态计算"，可只读共享
class Option {
  std::shared_ptr<const PricingEngine> engine_;  // 只读共享实现
public:
  double value(double spot) const {
    return engine_->price(spot);   // 无锁，多线程可并发调用
  }
};
```

**解析**：若实现层是**不可变配置 + 纯函数计算**（如定价模型参数只读），用 `shared_ptr<const>` 只读共享，抽象层多线程并发调用**零锁**。适合「模型配置在启动时定好、运行期只读」的定价场景。

#### 方式 C：实现替换 + 原子发布（运行时换实现）

```cpp
class Option {
  std::shared_ptr<const PricingEngine> engine_;  // 运行时可替换
public:
  // 原子替换实现（如从 BS 换成 Monte Carlo）
  void set_engine(std::shared_ptr<const PricingEngine> e) {
    std::atomic_store(&engine_, std::move(e));   // 无锁发布
  }
  double value(double spot) const {
    auto e = std::atomic_load(&engine_);         // 无锁取快照
    return e->price(spot);
  }
};
```

**解析**：若运行时需要**切换实现**（Bridge 的本质优势），用 `atomic_load/store(shared_ptr)` 原子替换，读线程拿快照无锁使用（参考 Strategy 方式 A）。实现层保持不可变，替换是无锁的。

**三方式对比**：

| 方式 | 实现层 | 锁 | 适用 |
|------|:---:|:---:|------|
| A. 分层锁 | 可变有锁 | 各自加锁 | **推荐**，实现有状态 |
| B. 只读共享 | 不可变 | 无 | 配置只读 |
| C. 原子替换 | 不可变 | 无 | 运行时换实现 |

---

### 7. Composite（组合）— Tier 3

**线程安全问题**：组合树是**读多写少**结构（遍历绘制、计算收益率），修改树结构（增删节点）较少。有三种方式。

#### 方式 A：`shared_mutex` + `unique_ptr`（读多写少）— ✅ 常用

```cpp
struct Group : GraphicObject {
  std::vector<std::unique_ptr<GraphicObject>> objects;  // 拥有子节点，RAII
  mutable std::shared_mutex mtx_;
  void add(std::unique_ptr<GraphicObject> obj) {
    std::unique_lock lk(mtx_);            // 独占：改结构
    objects.push_back(std::move(obj));
  }
  void draw() const {
    std::shared_lock lk(mtx_);            // 共享：并发遍历
    for (auto& o : objects) o->draw();
  }
};
```

**解析**：`unique_ptr` 解决所有权/释放，`shared_mutex` 匹配「读多写少」。**缺点**：读并发时仍共享一把锁，遍历长时会阻塞写。

#### 方式 B：Copy-on-Write（读无锁，遍历时快照）

```cpp
class Portfolio {
  std::shared_ptr<const std::vector<std::unique_ptr<Asset>>> children_;
  mutable std::mutex mtx_;
public:
  double total_value() const {
    auto snapshot = children_;            // 读：原子取快照，无锁
    double sum = 0;
    for (const auto& a : *snapshot) sum += a->value();  // 遍历只读快照
    return sum;
  }
  void add(std::unique_ptr<Asset> a) {
    std::lock_guard lk(mtx_);
    auto fresh = std::make_shared<std::vector<std::unique_ptr<Asset>>>(*children_);
    fresh->push_back(std::move(a));
    children_ = std::move(fresh);         // 原子发布
  }
};
```

**解析**：读线程拿**只读快照**无锁遍历；写线程复制、修改、原子发布。适合遍历频繁、结构少改。**缺点**：写时复制整棵子树开销大。

#### 方式 C：`recursive_mutex`（支持嵌套递归遍历）

```cpp
struct Group : GraphicObject {
  mutable std::recursive_mutex mtx_;   // 可重入：遍历子节点时可递归加锁
  void draw() const {
    std::lock_guard lk(mtx_);
    for (auto& o : objects) o->draw();  // 子节点也可能要锁
  }
};
```

**解析**：当**遍历会递归进入子节点**、且每层都可能加锁时，用 `recursive_mutex` 避免**自我死锁**。**缺点**：递归锁有额外开销，且掩盖了「应该在父节点一次加锁遍历」的更好设计（优先用方式 A 的一次锁整树遍历）。

**三方式对比**：

| 方式 | 读锁 | 写开销 | 适用 |
|------|:---:|:---:|------|
| A. shared_mutex | 有共享锁 | 小 | 读多写少 |
| B. Copy-on-Write | 无锁 | 大 | 遍历极频繁 |
| C. recursive_mutex | 递归锁 | 中 | 嵌套递归遍历 |

---

### 8. Decorator（装饰器）— Tier 1

**线程安全问题**：动态装饰器持有被装饰对象引用。若装饰器链被并发调用且底层可变，则需保护；若装饰器无状态/只读则安全。有三种方式。

#### 方式 A：不可变装饰器链（`shared_ptr<const>`）— ✅ 推荐

```cpp
struct ColoredShape : Shape {
  std::shared_ptr<const Shape> shape;   // 共享只读底层
  std::string color;
  explicit ColoredShape(std::shared_ptr<const Shape> s, std::string c)
      : shape(std::move(s)), color(std::move(c)) {}
  std::string str() const override {
    return shape->str() + " has the color " + color;
  }
};
```

**解析**：装饰器链**构建后不可变**（不修改底层），用 `shared_ptr<const>` 只读共享。多线程并发调用 `str()` **无需锁**——装饰器是「叠加只读行为」的天然不可变结构。

#### 方式 B：函数式装饰器（`std::function` 无状态）

```cpp
// 用函数组合替代对象组合，无状态、天然线程安全
using ShapeRender = std::function<std::string()>;
ShapeRender with_color(ShapeRender inner, std::string color) {
  return [inner = std::move(inner), color = std::move(color)] {
    return inner() + " has the color " + color;   // 捕获只读副本
  };
}
```

**解析**：用 `std::function` + lambda 捕获构建装饰链，**纯值捕获无共享状态**，天然线程安全。呼应本仓库 `strategy_functional.cpp` 的思路。

#### 方式 C：可变装饰器 + 内部锁

```cpp
struct CountingDecorator : Shape {
  std::shared_ptr<Shape> shape;   // 共享可变底层
  mutable std::mutex mtx_;
  int count_ = 0;
  std::string str() const {
    std::lock_guard lk(mtx_);     // 保护计数状态
    ++count_;                     // 有状态：并发调用需保护
    return shape->str();
  }
};
```

**解析**：若装饰器需要**有状态**（如统计调用次数），则需 `mutex` 保护其可变成员。**缺点**：失去装饰器的「只读无锁」优势，仅在确实需要状态时使用。

**三方式对比**：

| 方式 | 状态 | 锁 | 适用 |
|------|:---:|:---:|------|
| A. 不可变链 | 无 | 无 | **推荐**，叠加只读行为 |
| B. 函数式 | 无 | 无 | 函数组合 |
| C. 内部锁 | 有 | mutex | 需要统计等状态 |

---

### 9. Proxy（代理）— Tier 3

**线程安全问题**：**虚拟代理**（延迟加载）需保证「首次访问只初始化一次」；**计数代理**需原子计数。有三种方式。

#### 方式 A：`std::call_once`（懒加载只初始化一次）— ✅ 推荐

```cpp
class ImageProxy : public Graphic {
  std::unique_ptr<Image> img_;
  mutable std::once_flag flag_;
  Image* get_image() const {
    std::call_once(flag_, [this] { img_ = std::make_unique<Image>(); });
    return img_.get();
  }
};
```

**解析**：`call_once` + `once_flag` 是线程安全懒加载的标准工具，比 DCLP 不易错。`flag_` 标 `mutable` 支持 const 惰性加载。加载后 `img_` 变只读共享。

#### 方式 B：`std::atomic` 懒加载（无锁快速路径）

```cpp
class ImageProxy : public Graphic {
  std::atomic<Image*> img_{nullptr};
  Image* get_image() const {
    Image* p = img_.load(std::memory_order_acquire);
    if (!p) {
      Image* fresh = new Image();            // 可能多线程重复创建
      Image* expected = nullptr;
      if (img_.compare_exchange_strong(expected, fresh)) {
        return fresh;                        // 本线程胜出，负责管理 fresh
      }
      delete fresh;                          // 失败者释放自己创建的
      p = img_.load(std::memory_order_acquire);
    }
    return p;
  }
};
```

**解析**：用 **`compare_exchange_strong`（CAS）** 让多个线程「谁先写成功谁负责，其余释放自己那份」。无锁、低延迟。**缺点**：复杂，且失败线程每次都要 delete，高并发下浪费。

#### 方式 C：计数/保护代理（`atomic` 或 `mutex`）

```cpp
// 计数代理：用 atomic 计数，无锁
class CountingProxy {
  std::atomic<int> calls_{0};
public:
  void access() { calls_.fetch_add(1, std::memory_order_relaxed); }
  int count() const { return calls_.load(); }
};
// 访问控制代理：用 mutex 串行化临界区
class GuardProxy {
  std::mutex mtx_;
public:
  void protected_op() { std::lock_guard lk(mtx_); /* 临界区 */ }
};
```

**解析**：计数用 `atomic`（relaxed 即可，无需同步数据）；访问控制用 `mutex` 保护临界区。原子计数比锁更高效。

**三方式对比**：

| 方式 | 用途 | 锁 | 复杂度 |
|------|:---:|:---:|:---:|
| A. call_once | 懒加载 | 内建 | 低 |
| B. atomic CAS | 无锁懒加载 | 无 | 高 |
| C. atomic/mutex | 计数/访问控制 | 视情况 | 中 |

---

### 10. Façade（外观）— Tier 4

**线程安全问题**：Facade 是薄接口层，**无状态即线程安全**。三种方式取决于子系统如何暴露。

#### 方式 A：无状态 Facade（依赖注入，不缓存状态）— ✅ 推荐

```cpp
class TradingFacade {
  IPricingEngine& pricing_;   // 子系统经构造函数注入
  IRiskEngine& risk_;
public:
  TradingFacade(IPricingEngine& p, IRiskEngine& r) : pricing_(p), risk_(r) {}
  // 只做编排，不修改自身 → 线程安全
  double value_portfolio(const Portfolio& pf) const {
    return pricing_.price(pf) - risk_.adjust(pf);
  }
};
```

**解析**：把可变状态委托给子系统，Facade 只做**编排**（读取 + 返回），无状态即线程安全。**依赖注入**（Modern C++）便于测试替换。

#### 方式 B：Facade 缓存 + 锁

```cpp
class CachingFacade {
  std::mutex mtx_;
  std::map<key, double> cache_;   // 若需缓存计算结果
public:
  double get_price(key k) {
    std::lock_guard lk(mtx_);
    auto it = cache_.find(k);
    if (it != cache_.end()) return it->second;
    double v = expensive_calc(k);
    cache_[k] = v;                // 写缓存
    return v;
  }
};
```

**解析**：若 Facade 需缓存计算（避免重复昂贵计算），则共享缓存需 `mutex`。**优化**：读多写少时改 `shared_mutex`，或读线程 `try_lock` 不阻塞。

#### 方式 C：子系统各自线程安全，Facade 免锁

```cpp
// 若注入的子系统本身是线程安全的，Facade 天然安全
class TradingFacade {
  ThreadSafePricingEngine pricing_;   // 内部有锁/无状态
public:
  double value(const Portfolio& pf) const { return pricing_.price(pf); }
};
```

**解析**：把线程安全的责任**下放到子系统**，Facade 不做重复防护。这是分层架构的正确做法——每层只对自己的状态负责。

**三方式对比**：

| 方式 | 状态 | 锁 | 适用 |
|------|:---:|:---:|------|
| A. 无状态 | 无 | 无 | **推荐**，纯编排 |
| B. 缓存+锁 | 缓存 | mutex | 需缓存计算结果 |
| C. 子系统安全 | 无 | 无 | 子系统已线程安全 |

---

### 11. Flyweight（享元）— Tier 4

**线程安全问题**：享元共享「内在状态」（合约代码、交易所名驻留），**内在状态不可变则只读安全**；真正的并发挑战在**共享池的构建**。有三种方式。

#### 方式 A：`shared_mutex` + double-check 驻留表 — ✅ 常用

```cpp
class StringPool {
  mutable std::shared_mutex mtx_;
  std::unordered_map<std::string, uint16_t> reverse_;
  std::vector<std::string> forward_;
public:
  uint16_t intern(const std::string& s) {
    std::shared_lock rl(mtx_);
    auto it = reverse_.find(s);
    if (it != reverse_.end()) return it->second;
    rl.unlock();
    std::unique_lock wl(mtx_);              // 升级写
    it = reverse_.find(s);                  // double-check
    if (it != reverse_.end()) return it->second;
    uint16_t id = forward_.size();
    forward_.push_back(s);
    reverse_.emplace(s, id);
    return id;
  }
  const std::string& get(uint16_t id) const {
    std::shared_lock lk(mtx_);
    return forward_[id];   // 持锁期间引用有效
  }
};
```

**解析**：驻留表**读多写少**（大量查询已驻留、少数新增）→ `shared_mutex`。**double-check** 防止并发新增同一字符串导致重复驻留。**缺点**：`get()` 返回的引用在并发下有悬垂风险。

#### 方式 B：不可变享元 + 构造期建池（零锁）

```cpp
// 合约元数据（内在状态）在构造后不可变
struct ContractMeta {
  std::string symbol; uint16_t multiplier; Date expiry;
  // 只读访问器，无修改接口 → 天然线程安全
};
// 启动时一次性构建合约池，运行期只读
class ContractPool {
  inline static const std::vector<ContractMeta> all = build_all();
public:
  static const ContractMeta& get(size_t id) { return all[id]; }  // 无锁
};
```

**解析**：**运行期只读 + 构造期建池**是享元最优解。内在状态不可变、池子构造后不增删 → **零锁**。适合合约代码、交易所等「全量预加载」场景。

#### 方式 C：thread_local 池（每线程缓存查询）

```cpp
class LruCache {
  // 每线程持有自己的最近使用缓存，避免全局锁竞争
  thread_local static std::unordered_map<uint16_t, std::string> local_;
public:
  const std::string& get(uint16_t id) {
    auto it = local_.find(id);
    if (it == local_.end()) {
      std::shared_lock lk(global_mtx_);     // 仅未命中时查全局
      local_[id] = global_[id];
      return local_[id];                    // 之后读自己的，无锁
    }
    return it->second;
  }
};
```

**解析**：用 `thread_local` 缓存「最近访问」的享元，减少对全局池的锁竞争。适合每个线程访问模式相对固定的场景。**缺点**：内存增多，缓存一致性由调用方保证。

**三方式对比**：

| 方式 | 池共享 | 锁 | 适用 |
|------|:---:|:---:|------|
| A. shared_mutex | 全局 | 读写锁 | 运行期动态驻留 |
| B. 不可变建池 | 全局只读 | 无 | 全量预加载 |
| C. thread_local | 每线程缓存 | 无（命中） | 局部访问模式 |

---

## 四、行为型模式（Behavioral）

### 12. Observer（观察者）— Tier 1

**线程安全问题**：订阅/退订/通知是经典并发难题——通知时另一线程退订会导致迭代器失效；在持锁时回调会死锁。有多种方式。

#### 方式 A：快照通知 + `shared_mutex` — ✅ 推荐

```cpp
template <typename T> class Observable {
  mutable std::shared_mutex mtx_;
  std::vector<Observer<T>*> observers_;
public:
  void subscribe(Observer<T>* o) {
    std::unique_lock lk(mtx_);
    observers_.push_back(o);
  }
  void unsubscribe(Observer<T>* o) {
    std::unique_lock lk(mtx_);
    observers_.erase(std::remove(observers_.begin(), observers_.end(), o),
                     observers_.end());
  }
  void notify(T& src, const std::string& field) const {
    std::vector<Observer<T>*> snapshot;    // 快照
    { std::shared_lock lk(mtx_); snapshot = observers_; }  // 复制后释放锁
    for (auto* o : snapshot) o->field_changed(src, field); // 锁外回调
  }
};
```

**解析**：**快照通知**——先复制观察者列表、释放锁、再逐个回调。避免迭代器失效 + 持锁回调死锁。`shared_mutex` 让订阅/退订（写）互斥、通知读（共享）并发。这是**可重入且线程安全**的标准做法。

#### 方式 B：无锁单生产者（`atomic` 观察者）

```cpp
// 单一观察者场景：用 atomic 指针无锁发布
class SingleObserver {
  std::atomic<Observer<Quote>*> obs_{nullptr};
public:
  void set(Observer<Quote>* o) { obs_.store(o, std::memory_order_release); }
  void notify(const Quote& q) const {
    auto* o = obs_.load(std::memory_order_acquire);  // 原子取快照
    if (o) o->on_quote(q);
  }
};
```

**解析**：若**只有一个观察者**（如 1:1 推送），用 `atomic` 指针代替容器，**完全无锁**。`acquire/release` 保证发布正确性。**缺点**：不适用于一对多。

#### 方式 C：`recursive_mutex`（回调中再次触发通知）

```cpp
class ReentrantObservable {
  mutable std::recursive_mutex mtx_;
  std::vector<Observer*>* obs_;
public:
  void notify() {
    std::lock_guard lk(mtx_);   // 可重入：回调里再 notify 不会死锁
    for (auto* o : *obs_) o->update();
  }
};
```

**解析**：若观察者回调里会**再次触发通知**（嵌套通知），用 `recursive_mutex` 让同一线程可重复加锁，避免**自我死锁**。**缺点**：递归锁开销大，掩盖了「回调里改列表」的设计问题（更推荐方式 A 的快照）。

#### 方式 D：`condition_variable` 异步推送（解耦回调线程）

```cpp
class AsyncObservable {
  std::mutex mtx_;
  std::condition_variable cv_;
  std::deque<Event> queue_;
public:
  void publish(Event e) {
    { std::lock_guard lk(mtx_); queue_.push_back(std::move(e)); }
    cv_.notify_all();               // 唤醒所有订阅线程
  }
  Event wait_event() {
    std::unique_lock lk(mtx_);
    cv_.wait(lk, [this] { return !queue_.empty(); });
    auto e = std::move(queue_.front()); queue_.pop_front();
    return e;
  }
};
```

**解析**：把「通知」改为**异步队列**：发布者入队 + `notify_all`，观察者各自在**自己的线程**上 `wait` 取出。彻底解耦「发布」与「回调」，观察者处理慢不会阻塞发布者。适合行情推送。

**四方式对比**：

| 方式 | 并发场景 | 锁 | 适用 |
|------|:---:|:---:|------|
| A. 快照+shared_mutex | 一对多 | 读写锁 | **推荐** |
| B. atomic 单观察者 | 一对一 | 无 | 单观察者 |
| C. recursive_mutex | 嵌套通知 | 递归锁 | 回调再通知 |
| D. 异步队列 | 发布-订阅解耦 | mutex+cv | 行情异步推送 |

---

### 13. Command（命令）— Tier 2

**线程安全问题**：命令队列是生产者-消费者模型。有多种实现方式。

#### 方式 A：`mutex` + `condition_variable` 阻塞队列 — ✅ 常用

```cpp
class ThreadSafeQueue {
  std::deque<std::function<void()>> q_;
  mutable std::mutex mtx_;
  std::condition_variable cv_;
public:
  void push(std::function<void()> cmd) {
    { std::lock_guard lk(mtx_); q_.push_back(std::move(cmd)); }
    cv_.notify_one();        // 锁外唤醒
  }
  std::function<void()> pop() {
    std::unique_lock lk(mtx_);
    cv_.wait(lk, [this] { return !q_.empty(); });  // 谓词防虚假唤醒
    auto cmd = std::move(q_.front()); q_.pop_front();
    return cmd;
  }
};
```

**解析**：`condition_variable` + 谓词防虚假唤醒；`lock_guard` 最小化持锁范围；`notify` 在锁外。`std::function` 封装任意命令。

#### 方式 B：有界阻塞队列（固定容量 + 背压）

```cpp
class BoundedQueue {
  std::deque<std::function<void()>> q_;
  std::mutex mtx_;
  std::condition_variable not_empty_, not_full_;
  size_t capacity_;
public:
  void push(std::function<void()> cmd) {
    std::unique_lock lk(mtx_);
    not_full_.wait(lk, [this] { return q_.size() < capacity_; });  // 满则等
    q_.push_back(std::move(cmd));
    not_empty_.notify_one();
  }
  std::function<void()> pop() {
    std::unique_lock lk(mtx_);
    not_empty_.wait(lk, [this] { return !q_.empty(); });
    auto cmd = std::move(q_.front()); q_.pop_front();
    not_full_.notify_one();
    return cmd;
  }
};
```

**解析**：用**两个条件变量**实现**有界队列**。生产者队列满时阻塞（**背压**，防止无限堆积内存）。适合「生产快于消费、需限流」的金融订单队列。

#### 方式 C：无锁队列（SPSC / 多生产者 CAS）

```cpp
// 单生产者-单消费者（SPSC）可用无锁环形队列（如 boost::lockfree::spsc_queue）
#include <boost/lockfree/spsc_queue.hpp>
boost::lockfree::spsc_queue<std::function<void()>, 1024> q;
// push/top 在各自线程上调用，无锁
```

**解析**：低延迟场景可用无锁队列（`boost::lockfree` / `moodycamel`）。SPSC（单生产者单消费者）最简单且无锁。**缺点**：MPMC（多生产多消费）无锁队列复杂易错。

#### 方式 D：`std::async` 每次异步执行（免队列）

```cpp
void execute_async(std::function<void()> cmd) {
  std::async(std::launch::async, std::move(cmd));  // 每个命令一个线程
}
```

**解析**：命令量少时，用 `std::async` 直接异步执行，**无需队列**。**缺点**：每命令一个线程，创建开销大，不适用于大量命令。

**四方式对比**：

| 方式 | 队列 | 锁 | 适用 |
|------|:---:|:---:|------|
| A. 阻塞队列 | 无界 | mutex+cv | **通用** |
| B. 有界队列 | 有界 | 双 cv | 需背压限流 |
| C. 无锁队列 | 固定 | 无 | 低延迟（SPSC） |
| D. async | 无 | — | 命令量少 |

---

### 14. State（状态）— Tier 2

**线程安全问题**：状态机切换状态时，多线程同时调用可能读到中间状态（数据竞争）。有三种方式。

#### 方式 A：不可变状态 + `atomic<const State*>` — ✅ 推荐

```cpp
class LightSwitch {
  std::atomic<const State*> state_;  // 状态指针原子更新
public:
  void on() {
    const State* s = state_.load(std::memory_order_acquire);
    const State* next = s->on(*this);   // 状态对象返回下一个状态（不修改自身）
    state_.store(next, std::memory_order_release);  // 原子切换
  }
};
```

**解析**：**状态对象不可变**（`on()` 返回下一个状态而非改自身），可被所有线程共享。状态切换是**单个指针的原子更新**，无锁。`acquire/release` 确保新状态完全构造后再发布。

#### 方式 B：`std::mutex` 串行化状态转换

```cpp
class LightSwitch {
  std::mutex mtx_;
  std::unique_ptr<State> state_;   // 状态对象（可可变）
public:
  void on() {
    std::lock_guard lk(mtx_);      // 串行化整个转换过程
    State* next = state_->on(this);
    state_.reset(next);            // 更新状态
  }
};
```

**解析**：若状态对象**可变**（转换过程修改状态），用 `mutex` 串行化**整个转换**（包括计算下一个状态和切换），保证原子性。**缺点**：每次转换持锁，并发度低。

#### 方式 C：线程安全的状态转换表（数据驱动）

```cpp
class TransitionTable {
  std::mutex mtx_;
  std::map<State, std::vector<Transition>> rules_;  // 转换规则表
public:
  // 规则表是只读的（启动时加载）→ 查询无锁
  State next(State cur, Trigger trig) const {
    auto& ts = rules_.at(cur);      // const：只读查询，线程安全
    for (auto& t : ts) if (t.trigger == trig) return t.to;
    throw std::runtime_error("illegal transition");
  }
};
```

**解析**：若状态转换**规则表只读**（启动时配置），则查询是 `const` 只读 → **无锁线程安全**。状态机的「状态」由调用方独占持有。这是数据驱动状态机的理想形态。

**三方式对比**：

| 方式 | 状态对象 | 锁 | 适用 |
|------|:---:|:---:|------|
| A. atomic 指针 | 不可变 | 无 | **推荐**，高频切换 |
| B. mutex | 可变 | mutex | 状态转换复杂 |
| C. 只读规则表 | 规则只读 | 无 | 数据驱动状态机 |

---

### 15. Strategy（策略）— Tier 1

**线程安全问题**：策略运行时被切换（`set_strategy`），并发调用时可能读到旧/新策略。有三种方式。

#### 方式 A：`shared_ptr` + `atomic_load/store`（读写分离）— ✅ 推荐

```cpp
class TextProcessor {
  std::shared_ptr<ListStrategy> strategy_;
public:
  void set_strategy(std::shared_ptr<ListStrategy> s) {
    std::atomic_store(&strategy_, std::move(s));  // 原子发布
  }
  void append_list(const std::vector<std::string>& items) {
    auto s = std::atomic_load(&strategy_);        // 原子取快照
    for (auto& it : items) s->add_list_item(oss_, it);  // 快照保证存活
  }
};
```

**解析**：**`shared_ptr` + `atomic_load/store`** 让「切换」和「使用」都原子。读线程拿**策略快照（shared_ptr）**，即使切换仍有效且引用计数保证不释放。**无锁读写分离**，适合读频繁、切换少。

#### 方式 B：`mutex` 保护策略切换

```cpp
class TextProcessor {
  std::unique_ptr<ListStrategy> strategy_;
  std::mutex mtx_;
public:
  void set_strategy(std::unique_ptr<ListStrategy> s) {
    std::lock_guard lk(mtx_);
    strategy_ = std::move(s);
  }
  void append_list(...) {
    std::lock_guard lk(mtx_);
    strategy_->append(...);   // 整个操作持锁
  }
};
```

**解析**：若策略对象本身**非线程安全**（有内部可变状态），则用 `mutex` 保护「获取+使用」的整个临界区。**缺点**：串行化，读并发度低。

#### 方式 C：不可变策略 + `std::function`（无状态）

```cpp
class TextProcessor {
  std::function<void(std::ostringstream&, const std::string&)> strategy_;
public:
  void set_strategy(std::function<...> f) { strategy_ = std::move(f); }
};
// 若所有具体策略都无状态（纯函数），用 std::function 捕获无共享状态，天然安全
```

**解析**：呼应 `strategy_functional.cpp`——用 `std::function` 替代策略类，若策略是**无状态纯函数**，则多个线程可安全并发使用同一策略，无锁。**切换**策略仍需原子/锁（见方式 A/B）。

**三方式对比**：

| 方式 | 策略状态 | 切换 | 使用 |
|------|:---:|:---:|:---:|
| A. shared_ptr+atomic | 任意 | 无锁原子 | 无锁快照 |
| B. mutex | 可变 | 锁 | 锁 |
| C. std::function | 无状态 | 需原子/锁 | 无锁 |

---

### 16. Template Method（模板方法）— Tier 2

**线程安全问题**：模板方法基类通常无共享可变状态（子类各自持有状态），因此天然线程安全。有三种方式。

#### 方式 A：`const` 钩子方法（无共享状态）— ✅ 推荐

```cpp
class Game {
public:
  virtual ~Game() = default;
  void run() {                    // 模板方法（非虚）
    start();
    while (!have_winner())        // const：只读
      take_turn();
  }
protected:
  virtual void start() = 0;
  virtual bool have_winner() const = 0;  // const 钩子：不修改状态 → 线程安全
  virtual void take_turn() = 0;
};
```

**解析**：把只读钩子声明为 **`const`**，编译器强制不修改状态 → 多个线程可并发查询（如并发判断胜负、并发计算）。若**每个线程持有自己的 Game 实例**，则整个 `run()` 天然线程安全。

#### 方式 B：单实例 + 外部串行化

```cpp
// 若多个线程共享同一个 Game 实例，在外部加锁串行调用 run()
class GameRunner {
  Game& game_;
  std::mutex mtx_;
public:
  void run() { std::lock_guard lk(mtx_); game_.run(); }  // 串行执行
};
```

**解析**：若必须**共享同一实例**且实例有状态（回合数等），则在**调用方加锁串行**执行模板方法。**缺点**：整个算法串行，无法并发。

#### 方式 C：策略模式变体（并发安全的替代）

```cpp
// 若模板方法的分支可并行，可改用"组合式"回测框架：
// 每个阶段（load/calc/report）用独立的线程安全组件，阶段间串行、阶段内并行
struct BacktestEngine {
  std::vector<std::function<void()>> stages_;  // 阶段函数
  void run() { for (auto& s : stages_) s(); }  // 阶段串行
  // 若阶段内可并行，用 async 或任务组
};
```

**解析**：模板方法的**替代设计**——把各阶段做成**无状态可组合的步骤**（`std::function`），阶段间串行、可插入并行子步骤。比继承钩子更灵活、更利于并发。

**三方式对比**：

| 方式 | 共享 | 锁 | 适用 |
|------|:---:|:---:|------|
| A. const 钩子 | 每线程实例 | 无 | **推荐** |
| B. 外部串行 | 共享实例 | mutex | 必须共享 |
| C. 组合阶段 | 阶段隔离 | 无/局部 | 需并行阶段 |

---

### 17. Mediator（中介者）— Tier 4

**线程安全问题**：Mediator 是**中心化通信枢纽**，多个参与者线程同时发消息/命令，必须串行化处理。有三种方式。

#### 方式 A：快照广播 + `mutex` — ✅ 常用

```cpp
class Mediator {
  mutable std::mutex mtx_;
  std::vector<Colleague*> peers_;
public:
  void add_peer(Colleague* c) {
    std::lock_guard lk(mtx_);
    peers_.push_back(c);
  }
  void broadcast(const Message& msg) {
    std::vector<Colleague*> snapshot;   // 快照
    { std::lock_guard lk(mtx_); snapshot = peers_; }
    for (auto* p : snapshot) p->receive(msg);  // 锁外回调
  }
};
```

**解析**：**快照广播**（同 Observer）——先复制参与者列表、释放锁、再逐个通知，避免持锁回调死锁或迭代器失效。

#### 方式 B：单工作线程 + 命令队列（严格串行）

```cpp
class EventLoopMediator {
  ThreadSafeQueue queue_;   // 见 Command 部分
public:
  void send(Message msg) {
    queue_.push([msg] { /* 串行处理 */ });
  }
  // 单一事件循环线程：保证所有消息严格串行处理，无竞争
};
```

**解析**：若要求**严格串行、无快照**（如撮合引擎必须逐笔处理），用「**单一工作线程 + 命令队列**」，所有消息入队后由单线程串行消费。参与者只负责入队，不直接并发访问 Mediator 状态。

#### 方式 C：无共享状态 Mediator（无锁）

```cpp
// 若参与者不共享可变状态，只通过 Mediator 传递不可变消息
class StatelessMediator {
public:
  // 纯转发：消息不可变，Mediator 无状态 → 线程安全
  static void route(const Message& msg, Colleague* to) {
    if (to) to->receive(msg);   // 只读转发
  }
};
```

**解析**：若参与者各自线程安全、消息不可变、Mediator 只做**纯转发**，则无状态 Mediator **无锁线程安全**。这是「消灭共享可变状态」策略的体现。

**三方式对比**：

| 方式 | 一致性 | 锁 | 适用 |
|------|:---:|:---:|------|
| A. 快照广播 | 弱（快照） | mutex | 广播通知 |
| B. 单线程队列 | 强（严格串行） | mutex+cv | 撮合/订单 |
| C. 无状态 | — | 无 | 纯转发 |

---

### 18. Memento（备忘录）— Tier 4

**线程安全问题**：Memento 应设计为**不可变对象**（保存快照后不可修改），多线程可安全共享同一快照用于恢复。有三种方式。

#### 方式 A：不可变 Memento + 值语义（只读共享）— ✅ 推荐

```cpp
class Memento {
  std::vector<int> state_;   // 快照数据
public:
  explicit Memento(std::vector<int> state) : state_(std::move(state)) {}
  const std::vector<int>& get_state() const { return state_; }  // 只读
  // 无修改接口 → 不可变，天然线程安全
};
```

**解析**：**不可变对象**是并发最友好形态：构造后无人能改，所有线程可安全只读共享，**零锁**。`const` + 只读访问器让编译器强制不可变性。快照创建（`save()`）由 Originator 线程独占，之后可安全发布给其他线程。

#### 方式 B：`shared_ptr<const Memento>` 共享历史

```cpp
// Originator 保存的是"指向不可变快照的共享指针"
class Originator {
  std::vector<std::shared_ptr<const Memento>> history_;  // 历史栈
public:
  void save_state(const std::vector<int>& s) {
    history_.push_back(std::make_shared<Memento>(s));   // 不可变快照
  }
  // 恢复：读历史快照，多线程可并发读
};
```

**解析**：用 **`shared_ptr<const Memento>`** 共享历史快照。多个线程可**并发读取/恢复**同一历史（只读），无需锁。历史栈本身若被并发写，需额外保护。

#### 方式 C：Memento 可变（含撤销栈）+ 锁

```cpp
class UndoManager {
  std::mutex mtx_;
  std::vector<std::shared_ptr<Memento>> history_;  // 可变历史
public:
  void push(std::shared_ptr<Memento> m) {
    std::lock_guard lk(mtx_);
    history_.push_back(std::move(m));
  }
  std::shared_ptr<Memento> pop() {
    std::lock_guard lk(mtx_);
    if (history_.empty()) return nullptr;
    auto m = history_.back(); history_.pop_back();
    return m;
  }
};
```

**解析**：若 Memento 被组织成**可变的撤销栈**（压栈/弹栈），则栈结构需 `mutex` 保护。但**单个 Memento 仍是不可变**的，锁只保护「历史栈」的增删。

**三方式对比**：

| 方式 | 快照 | 历史栈 | 锁 |
|------|:---:|:---:|:---:|
| A. 不可变 | 不可变 | — | 无 |
| B. shared_ptr<const> | 不可变 | 只读共享 | 无（只读） |
| C. 撤销栈 | 不可变 | 可变 | mutex（栈） |

---

### 19. Iterator（迭代器）— Tier 4

**线程安全问题**：STL 容器迭代器**不是线程安全的**——一个线程遍历时另一个线程修改容器会导致迭代器失效（数据竞争）。有三种方式。

#### 方式 A：锁内遍历（`for_each` 接口，不暴露裸迭代器）— ✅ 常用

```cpp
class ThreadSafeVector {
  mutable std::shared_mutex mtx_;
  std::vector<int> data_;
public:
  void for_each(std::function<void(int)> f) const {
    std::shared_lock lk(mtx_);       // 持共享锁遍历
    for (auto v : data_) f(v);
  }
  void push(int v) {
    std::unique_lock lk(mtx_);
    data_.push_back(v);
  }
};
```

**解析**：**关键认知**——迭代器本身无法加锁，必须在**持有容器锁期间遍历**。因此提供「锁内遍历」接口（`for_each`）而非暴露裸迭代器。`shared_mutex` 让多个只读遍历并发。

#### 方式 B：Copy-on-Write（读无锁快照遍历）

```cpp
class CoWVector {
  std::shared_ptr<const std::vector<int>> data_;  // 只读快照
  std::mutex mtx_;
public:
  // 遍历：拿快照，无锁
  void for_each(std::function<void(int)> f) const {
    auto snap = data_;             // 原子取 shared_ptr
    for (auto v : *snap) f(v);     // 遍历只读快照，无锁
  }
  void push(int v) {
    std::lock_guard lk(mtx_);
    auto fresh = std::make_shared<std::vector<int>>(*data_);
    fresh->push_back(v);
    data_ = std::move(fresh);      // 原子发布
  }
};
```

**解析**：读线程**拿只读快照无锁遍历**；写线程复制、改、原子发布。读完全无锁，极适合读密集。**缺点**：写时复制整个容器，写开销大。

#### 方式 C：`std::lock` 多容器安全遍历（避免死锁）

```cpp
class Portfolio {
  mutable std::mutex mtx_a_, mtx_b_;
  std::vector<int> a_, b_;
public:
  // 同时遍历两个容器：用 std::lock 一次锁两把，避免死锁
  void combined() const {
    std::scoped_lock lk(mtx_a_, mtx_b_);   // 同时获取，避免交叉死锁
    // 遍历 a_ 和 b_
  }
};
```

**解析**：若遍历**同时涉及多个容器**（如组合遍历持仓+订单），用 **`std::scoped_lock`** 一次获取多把锁，保证锁定顺序一致，**避免死锁**（Effective C++ 并发）。

**三方式对比**：

| 方式 | 读锁 | 适用 |
|------|:---:|:---:|
| A. 锁内遍历 | shared_mutex | **通用** |
| B. Copy-on-Write | 无锁 | 读密集 |
| C. 多容器锁 | scoped_lock | 多容器遍历 |

---

### 20. Chain of Responsibility（责任链）— Tier 3

**线程安全问题**：责任链是「链上处理器依次处理」的流水线。若链被多线程同时处理不同请求且处理器有状态，会竞态。有三种方式。

#### 方式 A：无状态处理器 + 不可变请求 — ✅ 推荐

```cpp
class Handler {
  std::shared_ptr<Handler> next_;
public:
  void set_next(std::shared_ptr<Handler> next) { next_ = std::move(next); }
  virtual Result handle(const Request& req) const {
    if (can_handle(req)) return process(req);
    return next_ ? next_->handle(req) : Result::reject();
  }
protected:
  virtual bool can_handle(const Request&) const = 0;
  virtual Result process(const Request&) const = 0;   // 纯函数
};
```

**解析**：**处理器无状态 + 请求不可变** → 链可被多线程**并发处理不同请求**，互不干扰，**零锁**。`shared_ptr` 构建链，生命周期清晰。这是责任链的最优并发形态。

#### 方式 B：有状态处理器 + `mutex` 串行

```cpp
class RiskHandler : public Handler {
  std::mutex mtx_;
  double total_exposure_ = 0;   // 有状态：累计敞口
public:
  Result process(const Request& req) const override {
    std::lock_guard lk(mtx_);   // 保护累计状态
    total_exposure_ += req.amount;
    return total_exposure_ > LIMIT ? Result::reject() : Result::ok();
  }
};
```

**解析**：若处理器**有状态**（如累计风控敞口），则该状态需 `mutex` 保护。**缺点**：有状态处理器的并发度受限于锁；若想并发处理不同请求，应将有状态部分独立。

#### 方式 C：每线程责任链（thread_local）

```cpp
// 每个线程拥有独立的责任链实例（独立状态）
thread_local std::shared_ptr<Handler> per_thread_chain = build_chain();
Result handle_request(const Request& req) {
  return per_thread_chain->handle(req);   // 各线程独立，无竞争
}
```

**解析**：用 `thread_local` 让**每个线程持有自己的责任链**（各自状态），完全无竞争。适合「每线程需要独立的风控/处理状态」的场景。**缺点**：各线程状态不共享（如风控敞口无法跨线程累计）。

**三方式对比**：

| 方式 | 处理器 | 锁 | 适用 |
|------|:---:|:---:|------|
| A. 无状态 | 无状态 | 无 | **推荐**，并发处理 |
| B. 有状态+锁 | 有状态 | mutex | 需累计状态 |
| C. thread_local | 每线程 | 无 | 每线程独立 |

---

### 21. Visitor（访问者）— Tier 3

**线程安全问题**：访问者遍历对象结构时，若同时被遍历结构被修改，会竞态。若结构只读，则访问者可并发安全遍历。有三种方式。

#### 方式 A：无状态访问者 + 只读结构 — ✅ 推荐

```cpp
struct GreeksVisitor {
  // 无成员状态：每次 visit 是纯函数式计算
  void visit(const EuropeanOption& o, double& out) const {
    out = /* 只读计算 */;
  }
};
// 结构只读共享 + 访问者无状态 → 多线程可并发访问同一结构
```

**解析**：**访问者无状态**（`const` + 不修改被访问对象）+ **结构只读** → 多线程可**并发**对同一结构执行同一访问者（如同时算 Delta、Gamma），零锁。

#### 方式 B：访问者收集结果 + 线程安全结果集

```cpp
struct ParallelGreeksVisitor {
  // 结果收集用线程安全结构（如每个线程独立分区）
  void visit(const Option& o, double out) {
    results_[std::this_thread::get_id()].push_back(out);  // 每线程独立
  }
  std::map<std::thread::id, std::vector<double>> results_;
};
```

**解析**：若访问者需**收集结果**，用「每线程独立的结果分区」避免竞争；或对共享结果容器加锁。用 `map<thread::id, ...>` 天然隔离各线程结果。

#### 方式 C：结构修改保护（`shared_mutex` 遍历）

```cpp
class OptionTree {
  mutable std::shared_mutex mtx_;
public:
  void accept(Visitor& v) const {
    std::shared_lock lk(mtx_);     // 遍历持共享锁
    for (auto& n : nodes_) n->accept(v);
  }
  void insert(Node n) {
    std::unique_lock lk(mtx_);     // 修改持独占锁
    nodes_.push_back(std::move(n));
  }
};
```

**解析**：若被遍历的**结构会被并发修改**，则遍历期间用 `shared_mutex`（读多写少）保护。`accept` 持共享锁遍历，插入持独占锁。

**三方式对比**：

| 方式 | 访问者 | 结构 | 锁 |
|------|:---:|:---:|:---:|
| A. 无状态 | 无状态 | 只读 | 无 |
| B. 结果分区 | 有收集 | 只读 | 无（分区） |
| C. 结构保护 | 任意 | 可改 | shared_mutex |

---

### 22. Interpreter（解释器）— Tier 4

**线程安全问题**：解释器解析表达式树，通常只读求值（AST 构建后不变），因此可并发安全地「多次求值」。有三种方式。

#### 方式 A：不可变 AST + `const` 求值 — ✅ 推荐

```cpp
struct Expr {
  virtual ~Expr() = default;
  virtual double eval() const = 0;   // const：只读求值
};
struct Number : Expr {
  double val_;
  double eval() const override { return val_; }
};
struct Add : Expr {
  std::shared_ptr<const Expr> l, r;   // 共享不可变子节点
  double eval() const override { return l->eval() + r->eval(); }
};
```

**解析**：**AST 不可变 + 求值 `const`** → 构建后不再修改，多线程可**并发对同一棵树求值**（如不同情景/参数同时计算），零锁。`shared_ptr<const Expr>` 子节点只读共享。

#### 方式 B：每线程局部解释器（避免共享求值上下文）

```cpp
// 每个线程持有自己的解释器（含独立求值栈/变量绑定）
thread_local Interpreter local_interpreter;
double evaluate_local(const AST& tree) {
  return local_interpreter.eval(tree);   // 各线程独立上下文
}
```

**解析**：若解释器**有求值上下文**（变量绑定、求值栈），用 `thread_local` 让每线程独立解释器，避免共享上下文竞争。适合有状态求值。

#### 方式 C：上下文隔离 + 不可变 AST（并行情景分析）

```cpp
// 同一棵 AST（不可变）在多线程中用不同"情景上下文"并行求值
struct ScenarioContext { double vol; double spot; };
// 每个线程传入自己的 context，AST 只读 → 并行计算不同情景的结果
std::vector<double> run_scenarios(const AST& tree,
                                  const std::vector<ScenarioContext>& ctxs) {
  std::vector<double> out;
  std::mutex mtx;
  std::for_each(std::execution::par, ctxs.begin(), ctxs.end(), [&](auto& c) {
    double v = eval_with(tree, c);   // tree 只读共享
    std::lock_guard lk(mtx);
    out.push_back(v);
  });
  return out;
}
```

**解析**：用**并行算法**（`std::execution::par`）对同一**不可变 AST** 用不同情景上下文**并行求值**——正是金融「情景分析」的典型场景。AST 只读共享，各线程上下文隔离。

**三方式对比**：

| 方式 | AST | 上下文 | 锁 |
|------|:---:|:---:|:---:|
| A. 不可变 AST | 不可变 | 无 | 无 |
| B. thread_local | 任意 | 每线程 | 无 |
| C. 情景并行 | 不可变 | 隔离 | 收集结果时 |

---

## 五、总结：按并发场景选择方案

| 并发场景 | 推荐策略 | 涉及模式 |
|----------|----------|----------|
| 单例懒加载 | **Meyers** / `call_once` / `atomic` | Singleton |
| 读多写少的共享表 | **`shared_mutex`** / Copy-on-Write | Factory、Flyweight、Composite、Observer |
| 通知时列表可能被改 | **快照 + 锁外回调** | Observer、Mediator |
| 原子切换单个对象 | **`atomic_load/store(shared_ptr)`** | Strategy、State |
| 一次性初始化 | **`call_once`** / CAS | Proxy、Singleton |
| 生产者-消费者队列 | **`mutex` + `condition_variable`**（有界/无界） | Command、Mediator |
| 无共享可变状态 | **`const` + 不可变对象** | Template、Visitor、Interpreter、Memento |
| 只读共享 | **`shared_ptr<const T>`** | Decorator、Composite、Prototype、Adapter |
| 每线程独立状态 | **`thread_local`** / 值语义 | Builder、Prototype、Chain、Interpreter |

### 多方案选择的决策步骤

面对一个模式的并发需求，按以下顺序决策：

1. **能否消灭共享可变状态？** → 能则用「不可变 / 无状态 / thread_local / 值语义」，零锁最优。
2. **是读多写少吗？** → 用 `shared_mutex`（有锁但读并发），或 Copy-on-Write（读无锁、写贵）。
3. **是切换/替换单个对象吗？** → 用 `atomic_load/store(shared_ptr)` 或 `atomic` 指针，无锁。
4. **是生产-消费 / 严格串行吗？** → 用 `condition_variable` 队列（有界加背压）。
5. **是通知/回调吗？** → 先快照、释放锁、再回调（防死锁/迭代器失效）。
6. **都不适用？** → 用 `mutex` + `lock_guard`（通用兜底）。

### 最终原则

1. **消灭共享可变状态** 永远优于「加锁保护」——不可变对象、值语义、无状态是线程安全的第一选择。
2. 必须共享可变状态时，用 **RAII 锁**（`lock_guard`/`shared_lock`）缩小持锁范围。
3. 能用 `atomic` 原子切换单个指针/整数时，不要用 `mutex`（无锁、更高效）。
4. 回调/用户代码**绝不在持锁时调用**，先快照再执行（避免死锁）。
5. **多把锁用 `std::lock`/`scoped_lock`** 一次获取，避免死锁。

---

*配套可运行综合示例见 [Concurrency/thread_safe_patterns.cpp](./Concurrency/thread_safe_patterns.cpp)，演示 Meyers' Singleton、快照 + shared_mutex 的 Observer、condition_variable 命令队列的实测运行。*
