cmake_minimum_required(VERSION 3.16)

project(TodoList VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "C:/dev-builds/todolist")

find_package(Qt6 COMPONENTS Widgets Core Network Sql REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(TodoList
    main.cpp
    todoList.cpp
    weather.cpp
    database.cpp
    verse.cpp
    calendar.cpp
    hourlySchedule.cpp
)
target_link_libraries(TodoList PRIVATE Qt6::Core Qt6::Network Qt6::Sql)

add_executable(TodoListGui
    gui_main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)
target_link_libraries(TodoListGui PRIVATE Qt6::Widgets Qt6::Core Qt6::Network Qt6::Sql)