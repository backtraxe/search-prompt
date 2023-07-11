cmake_minimum_required(VERSION 3.0.0)
project(search-prompt VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

# add_executable(search-prompt server.cpp Trie.cpp)
add_executable(search-prompt client.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
