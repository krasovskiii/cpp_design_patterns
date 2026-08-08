/*
 * =============================================================================
 * 综合示例：从线程安全角度优化三种核心设计模式（Modern C++）
 * =============================================================================
 *
 * 【演示内容】
 * 1. Singleton —— Meyers' Singleton（C++11 保证构造线程安全）
 * 2. Observer  —— 快照 + shared_mutex（可重入且线程安全）
 * 3. Command   —— mutex + condition_variable（线程安全命令队列）
 *
 * 【设计模式优先级对照】
 * - Singleton：Tier 1
 * - Observer：Tier 1
 * - Command：Tier 2
 *
 * 【Modern C++ 特性】
 * - Meyers' Singleton：函数局部 static 的线程安全初始化
 * - std::shared_mutex / std::shared_lock：读写锁（读多写少）
 * - std::condition_variable + 谓词：生产者-消费者
 * - std::jthread / std::thread + lambda
 * - std::function：任意可调用对象作为命令
 * - 快照通知：先复制再回调，避免死锁与迭代器失效
 *
 * 【编译】g++ -std=c++17 -pthread thread_safe_patterns.cpp -o thread_safe_patterns
 */

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

// ============================================================================
// 1. 线程安全 Singleton（Meyers' Singleton）
// ============================================================================
// C++11 保证：函数局部 static 对象的初始化是线程安全的（编译器自动同步）。
// 无需任何手写锁即可实现懒加载 + 线程安全。
class Logger {
public:
  static Logger &get() {
    static Logger instance;  // 关键：线程安全初始化
    return instance;
  }

  // 单例对象不允许拷贝/赋值
  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

  // 注意：Meyers 只保证"构造"线程安全。
  // 若单例持有可变共享状态（这里输出流），访问仍需加锁。
  void log(const std::string &msg) {
    std::lock_guard<std::mutex> lk(out_mtx_);  // 保护共享输出
    std::cout << "[thread " << std::this_thread::get_id() << "] " << msg
              << std::endl;
  }

private:
  Logger() = default;  // 私有构造
  std::mutex out_mtx_; // 保护共享的输出流
};

// ============================================================================
// 2. 线程安全 Observer（快照通知 + shared_mutex）
// ============================================================================
struct PriceObserver {
  virtual ~PriceObserver() = default;
  virtual void on_price(const std::string &symbol, double price) = 0;
};

// 被观察者：行情源（读多写少：订阅/退订少，推送行情多）
class QuotePublisher {
  mutable std::shared_mutex mtx_;       // 读写锁
  std::vector<PriceObserver *> subs_;   // 观察者列表

public:
  // 订阅（写）：独占锁
  void subscribe(PriceObserver *o) {
    std::unique_lock<std::shared_mutex> lk(mtx_);
    subs_.push_back(o);
  }

  // 退订（写）：独占锁
  void unsubscribe(PriceObserver *o) {
    std::unique_lock<std::shared_mutex> lk(mtx_);
    // erase-remove 惯用法（Effective C++ 条款 20 / 标准库惯用法）
    subs_.erase(std::remove(subs_.begin(), subs_.end(), o), subs_.end());
  }

  // 发布行情（读列表 + 通知）
  // 关键：先复制观察者快照，释放锁后再逐个通知。
  // 这样避免：1) 通知期间另一线程退订导致迭代器失效
  //           2) 持锁调用用户回调导致死锁
  void publish(const std::string &symbol, double price) {
    std::vector<PriceObserver *> snapshot;
    {
      std::shared_lock<std::shared_mutex> lk(mtx_);  // 共享锁：多个发布者可并发
      snapshot = subs_;
    }
    for (auto *o : snapshot) o->on_price(symbol, price);
  }
};

// 一个简单的行情观察者：收到行情就记录（这里只打印计数）
class Subscriber : public PriceObserver {
public:
  void on_price(const std::string &symbol, double price) override {
    Logger::get().log("got " + symbol + " = " + std::to_string(price));
  }
};

// ============================================================================
// 3. 线程安全 Command 队列（mutex + condition_variable）
// ============================================================================
// 生产者-消费者模型：多个线程往队列塞命令，工作线程逐个执行。
class ThreadSafeQueue {
  std::deque<std::function<void()>> q_;  // std::function 封装任意命令
  mutable std::mutex mtx_;
  std::condition_variable cv_;

public:
  // 生产者：入队
  void push(std::function<void()> cmd) {
    {
      std::lock_guard<std::mutex> lk(mtx_);  // 只保护队列操作
      q_.push_back(std::move(cmd));
    }
    cv_.notify_one();  // 在锁外唤醒，避免唤醒线程争锁
  }

  // 消费者：阻塞取一条命令（队列空则等待）
  std::function<void()> pop() {
    std::unique_lock<std::mutex> lk(mtx_);
    // 带谓词等待，防止虚假唤醒
    cv_.wait(lk, [this] { return !q_.empty(); });
    auto cmd = std::move(q_.front());
    q_.pop_front();
    return cmd;
  }
};

// ============================================================================
// 主函数：并发演示
// ============================================================================
int main() {
  using namespace std::chrono_literals;

  Logger::get().log("=== 线程安全 Singleton：多线程并发访问单例 ===");
  {
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
      threads.emplace_back([i] {
        // 所有线程访问同一个 Logger 单例，Meyers 保证只初始化一次
        Logger::get().log("accessing singleton from worker " + std::to_string(i));
      });
    }
    for (auto &t : threads) t.join();
  }

  Logger::get().log("");
  Logger::get().log("=== 线程安全 Observer：订阅/退订/发布并发 ===");
  {
    QuotePublisher quotes;
    Subscriber a, b;
    quotes.subscribe(&a);
    quotes.subscribe(&b);

    std::thread publisher([&] {
      for (int i = 0; i < 5; ++i) {
        quotes.publish("AAPL", 100.0 + i);
        std::this_thread::sleep_for(10ms);
      }
    });

    // 并发退订一个观察者，验证快照机制不会导致崩溃
    std::thread unsubscriber([&] {
      std::this_thread::sleep_for(15ms);
      quotes.unsubscribe(&b);
      Logger::get().log("subscriber b unsubscribed");
    });

    publisher.join();
    unsubscriber.join();
  }

  Logger::get().log("");
  Logger::get().log("=== 线程安全 Command 队列：生产者-消费者 ===");
  {
    ThreadSafeQueue queue;

    // 一个工作线程持续消费命令
    std::thread worker([&] {
      while (auto cmd = queue.pop()) {  // pop 返回空 function 表示停止
        cmd();
      }
    });

    // 多个生产者线程往队列塞命令
    std::thread producer1([&] {
      for (int i = 0; i < 3; ++i)
        queue.push([i] { Logger::get().log("command from producer1 #" + std::to_string(i)); });
    });
    std::thread producer2([&] {
      for (int i = 0; i < 3; ++i)
        queue.push([i] { Logger::get().log("command from producer2 #" + std::to_string(i)); });
    });

    producer1.join();
    producer2.join();
    queue.push(std::function<void()>());  // 发送空命令通知 worker 退出
    worker.join();
  }

  Logger::get().log("");
  Logger::get().log("全部演示完成，线程安全示例运行正常。");
  return 0;
}
