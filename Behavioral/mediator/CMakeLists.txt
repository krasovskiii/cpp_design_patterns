project(mediator)

# chat_room
add_executable(chat_room main.cpp Person.cpp ChatRoom.cpp)
install(TARGETS chat_room DESTINATION Behavioral/${PROJECT_NAME})

# soccer_event_broker
add_executable(soccer_event_broker soccer_event_broker.cpp)
install(TARGETS soccer_event_broker DESTINATION Behavioral/${PROJECT_NAME})
