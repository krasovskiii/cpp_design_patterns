project(flyweight)

# flyweight_handmade
add_executable(flyweight_handmade flyweight_handmade.cpp)
install(TARGETS flyweight_handmade DESTINATION Structural/${PROJECT_NAME})

# flyweight_boost - now uses custom flyweight, no boost dependency
add_executable(flyweight_boost flyweight_boost.cpp)
install(TARGETS flyweight_boost DESTINATION Structural/${PROJECT_NAME})

# flyweight_text
add_executable(flyweight_text flyweight_text.cpp)
install(TARGETS flyweight_text DESTINATION Structural/${PROJECT_NAME})
