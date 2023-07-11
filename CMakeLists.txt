cmake_minimum_required(VERSION 3.0.0)
project(search-prompt VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()


add_executable(search-prompt server.cpp Trie.cpp)
# add_executable(search-prompt client.cpp)

find_package(Threads REQUIRED)
target_link_libraries(search-prompt Threads::Threads)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
include(CPack)