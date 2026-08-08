project(concurrency)

# 线程安全设计模式综合示例
find_package(Threads REQUIRED)
add_executable(thread_safe_patterns thread_safe_patterns.cpp)
target_link_libraries(thread_safe_patterns PRIVATE Threads::Threads)
install(TARGETS thread_safe_patterns DESTINATION Concurrency/${PROJECT_NAME})

# 同一模式多线程安全方案对比示例
add_executable(multi_strategy multi_strategy.cpp)
target_link_libraries(multi_strategy PRIVATE Threads::Threads)
install(TARGETS multi_strategy DESTINATION Concurrency/${PROJECT_NAME})
