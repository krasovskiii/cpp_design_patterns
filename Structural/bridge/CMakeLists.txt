project(bridge)

# pimpl_idiom
add_executable(bridge_pimpl_idiom pimpl_idiom.cpp)
install(TARGETS bridge_pimpl_idiom DESTINATION Structural/${PROJECT_NAME})

# pimpl_library —— 泛型 pimpl 模板库（无 main，作为静态库编译，避免链接错误）
add_library(pimpl_library STATIC pimpl_library.cpp)
install(TARGETS pimpl_library DESTINATION Structural/${PROJECT_NAME})


# bridge
add_executable(bridge bridge.cpp)
install(TARGETS bridge DESTINATION Structural/${PROJECT_NAME})
