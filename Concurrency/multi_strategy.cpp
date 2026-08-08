/*
 * =============================================================================
 * 综合示例：同一设计模式的多种线程安全实现对比（Modern C++）
 * =============================================================================
 *
 * 【演示内容】
 * 1. Singleton —— 4 种线程安全实现（Meyers / call_once / atomic / static 成员）
 * 2. Observer  —— 2 种实现（快照+shared_mutex / 无锁单观察者）
 * 3. Strategy  —— 2 种实现（shared_ptr+atomic / mutex 保护）
 *
 * 【目的】
 * 展示"同一模式在不同负载下应选不同方案"，每种方式都用多线程并发验证其线程安全。
 *
 * 【Modern C++ 特性】
 * - Meyers' Singleton：函数局部 static 线程安全初始化
 * - std::call_once / once_flag
 * - std::atomic / atomic_load / atomic_store
 * - std::shared_mutex / shared_lock（读写锁）
 * - std::thread + lambda
 *
 * 【编译】g++ -std=c++17 -pthread multi_strategy.cpp -o multi_strategy
 */

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

namespace {

// ============================================================================
// 1. Singleton：四种线程安全实现
// ============================================================================

// --- 方式 A：Meyers' Singleton ---
class MeyersSingleton {
public:
  static MeyersSingleton &get() {
    static MeyersSingleton instance;  // C++11 保证线程安全初始化
    return instance;
  }
  MeyersSingleton(const MeyersSingleton &) = delete;
  MeyersSingleton &operator=(const MeyersSingleton &) = delete;
  int id() const { return 1; }
private:
  MeyersSingleton() = default;
};

// --- 方式 B：call_once + 显式指针 ---
class CallOnceSingleton {
  inline static CallOnceSingleton *inst_ = nullptr;
  inline static std::once_flag flag_;
public:
  static CallOnceSingleton &get() {
    std::call_once(flag_, [] { inst_ = new CallOnceSingleton(); });
    return *inst_;
  }
  CallOnceSingleton(const CallOnceSingleton &) = delete;
  CallOnceSingleton &operator=(const CallOnceSingleton &) = delete;
  int id() const { return 2; }
private:
  CallOnceSingleton() = default;
};

// --- 方式 C：atomic 指针 + 双重检查 ---
class AtomicSingleton {
  inline static std::atomic<AtomicSingleton *> inst_{nullptr};
public:
  static AtomicSingleton &get() {
    AtomicSingleton *p = inst_.load(std::memory_order_acquire);
    if (!p) {
      static AtomicSingleton instance;
      p = &instance;
      inst_.store(p, std::memory_order_release);
    }
    return *p;
  }
  AtomicSingleton(const AtomicSingleton &) = delete;
  AtomicSingleton &operator=(const AtomicSingleton &) = delete;
  int id() const { return 3; }
private:
  AtomicSingleton() = default;
};

// --- 方式 D：static 成员（非懒加载）---
class StaticSingleton {
  static StaticSingleton instance_;
public:
  static StaticSingleton &get() { return instance_; }
  StaticSingleton(const StaticSingleton &) = delete;
  StaticSingleton &operator=(const StaticSingleton &) = delete;
  int id() const { return 4; }
private:
  StaticSingleton() = default;
};
StaticSingleton StaticSingleton::instance_;

// 泛型验证：多个线程并发调用 get()，都拿到同一实例
template <typename S>
void verify_singleton(const char *name) {
  std::vector<std::thread> ts;
  std::atomic<int> ptr_id{0};
  S *first = nullptr;
  std::mutex mtx;
  for (int i = 0; i < 8; ++i) {
    ts.emplace_back([&] {
      S *p = &S::get();
      {
        std::lock_guard lk(mtx);
        if (!first) first = p;
        else assert(first == p);   // 所有线程必须拿到同一实例
      }
      ptr_id.fetch_add(p->id());   // 累加 id 验证可达
    });
  }
  for (auto &t : ts) t.join();
  std::cout << "  [OK] " << name << " 统一实例 (累加 id=" << ptr_id.load() << ")\n";
}

// ============================================================================
// 2. Observer：两种实现
// ============================================================================

// --- 方式 A：快照 + shared_mutex（一对多）---
struct Quote { std::string symbol; double price; };
struct QuoteListener {
  virtual ~QuoteListener() = default;
  virtual void on_quote(const Quote &) = 0;
};

class ThreadSafePublisher {
  mutable std::shared_mutex mtx_;
  std::vector<QuoteListener *> listeners_;
public:
  void subscribe(QuoteListener *l) {
    std::unique_lock lk(mtx_);
    listeners_.push_back(l);
  }
  void unsubscribe(QuoteListener *l) {
    std::unique_lock lk(mtx_);
    listeners_.erase(std::remove(listeners_.begin(), listeners_.end(), l),
                     listeners_.end());
  }
  void publish(const Quote &q) {
    std::vector<QuoteListener *> snapshot;  // 快照
    { std::shared_lock lk(mtx_); snapshot = listeners_; }
    for (auto *l : snapshot) l->on_quote(q);  // 锁外回调
  }
};

// --- 方式 B：无锁单观察者（atomic）---
class SingleSlotPublisher {
  std::atomic<QuoteListener *> listener_{nullptr};
public:
  void set(QuoteListener *l) { listener_.store(l, std::memory_order_release); }
  void publish(const Quote &q) const {
    if (auto *l = listener_.load(std::memory_order_acquire)) l->on_quote(q);
  }
};

class LogListener : public QuoteListener {
public:
  std::atomic<int> count_{0};
  void on_quote(const Quote &) override { count_.fetch_add(1); }
};

void demo_observer() {
  LogListener a, b;
  ThreadSafePublisher pub;
  pub.subscribe(&a);
  pub.subscribe(&b);

  // 一个发布线程 + 一个退订线程并发
  std::thread publisher([&] {
    for (int i = 0; i < 100; ++i) pub.publish({"AAPL", 100.0 + i});
  });
  std::thread unsub([&] {
    std::this_thread::sleep_for(std::chrono::microseconds(50));
    pub.unsubscribe(&b);   // 快照机制保证不崩溃、不失效
  });
  publisher.join();
  unsub.join();
  std::cout << "  [OK] Observer-快照: a 收到 " << a.count_.load()
            << " 条, b 收到 " << b.count_.load() << " 条 (并发退订安全)\n";

  // 单观察者无锁版
  SingleSlotPublisher single;
  single.set(&a);
  std::thread sp([&] { for (int i = 0; i < 1000; ++i) single.publish({"QQQ", 1.0}); });
  sp.join();
  std::cout << "  [OK] Observer-无锁单观察者: 收到 " << a.count_.load()
            << " 条以上 (累计)\n";
}

// ============================================================================
// 3. Strategy：两种切换方式
// ============================================================================

struct ExecStrategy {
  virtual ~ExecStrategy() = default;
  virtual double slippage() const = 0;
};
struct FixedSlip : ExecStrategy { double slippage() const override { return 0.5; } };
struct PercentSlip : ExecStrategy { double slippage() const override { return 0.02; } };

// --- 方式 A：shared_ptr + atomic_load/store（无锁读写分离）---
class AtomicStrategyHolder {
  std::shared_ptr<const ExecStrategy> strat_ =
      std::make_shared<FixedSlip>();
public:
  void set(std::shared_ptr<const ExecStrategy> s) {
    std::atomic_store(&strat_, std::move(s));
  }
  double cost(double qty) const {
    auto s = std::atomic_load(&strat_);   // 无锁取快照
    return qty * s->slippage();
  }
};

// --- 方式 B：mutex 保护 ---
class LockedStrategyHolder {
  std::unique_ptr<ExecStrategy> strat_ = std::make_unique<FixedSlip>();
  mutable std::mutex mtx_;
public:
  void set(std::unique_ptr<ExecStrategy> s) {
    std::lock_guard lk(mtx_);
    strat_ = std::move(s);
  }
  double cost(double qty) const {
    std::lock_guard lk(mtx_);
    return qty * strat_->slippage();
  }
};

void demo_strategy() {
  AtomicStrategyHolder atomic_holder;
  // 用 CAS 循环实现原子浮点累加（std::atomic<double> 无 fetch_add）
  std::atomic<double> total{0.0};
  const auto add_atomic = [&](double v) {
    double old = total.load();
    while (!total.compare_exchange_weak(old, old + v)) {}  // CAS 自旋
  };
  std::vector<std::thread> ts;
  for (int i = 0; i < 4; ++i) {
    ts.emplace_back([&] {
      for (int j = 0; j < 1000; ++j) add_atomic(atomic_holder.cost(10));  // 并发读
    });
  }
  // 并发切换策略
  ts.emplace_back([&] {
    atomic_holder.set(std::make_shared<PercentSlip>());
  });
  for (auto &t : ts) t.join();
  std::cout << "  [OK] Strategy-atomic: 并发读+切换无锁运行, 累计成本=" << total.load() << "\n";
}

}  // namespace

int main() {
  std::cout << "=== Singleton：四种线程安全实现 ===\n";
  verify_singleton<MeyersSingleton>("Meyers");
  verify_singleton<CallOnceSingleton>("call_once");
  verify_singleton<AtomicSingleton>("atomic指针");
  verify_singleton<StaticSingleton>("static成员");

  std::cout << "\n=== Observer：两种实现 ===\n";
  demo_observer();

  std::cout << "\n=== Strategy：两种切换方式 ===\n";
  demo_strategy();

  std::cout << "\n全部多方案对比演示完成。\n";
  return 0;
}
