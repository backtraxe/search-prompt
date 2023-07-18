cmake_minimum_required(VERSION 3.0.0)
project(search-prompt VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(search-prompt src/Server.cpp src/Utility.cpp src/Trie.cpp src/ThreadPool.cpp)

set(CMAKE_CXX_STANDARD 11)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -Wall")
include(CPack)

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)