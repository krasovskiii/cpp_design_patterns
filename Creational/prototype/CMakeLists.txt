project(prototype)

# motivation
add_executable(motivation motivation.cpp)
install(TARGETS motivation DESTINATION Creational/${PROJECT_NAME})

# prototype
add_executable(prototype prototype.cpp)
install(TARGETS prototype DESTINATION Creational/${PROJECT_NAME})

# prototype_factory
add_executable(prototype_factory prototype_factory.cpp)
install(TARGETS prototype_factory DESTINATION Creational/${PROJECT_NAME})

# prototype_serialization
add_executable(prototype_serialization prototype_serialization.cpp)
install(TARGETS prototype_serialization DESTINATION Creational/${PROJECT_NAME})

# exercise
add_executable(prototype_exercise exercise.cpp)
install(TARGETS prototype_exercise DESTINATION Creational/${PROJECT_NAME})
