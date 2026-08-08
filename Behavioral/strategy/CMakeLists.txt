project(strategy)

# dynamic_strategy
add_executable(dynamic_strategy dynamic_strategy.cpp)
install(TARGETS dynamic_strategy DESTINATION Behavioral/${PROJECT_NAME})

# static_strategy
add_executable(static_strategy static_strategy.cpp)
install(TARGETS static_strategy DESTINATION Behavioral/${PROJECT_NAME})

# functional_strategy (std::function based, 呼应"避免过度设计")
add_executable(strategy_functional strategy_functional.cpp)
install(TARGETS strategy_functional DESTINATION Behavioral/${PROJECT_NAME})
