cmake_minimum_required(VERSION 3.31)

project(mem_units)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(${PROJECT_NAME}
  inc/mem_units.hpp
  tests/test_units.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC inc)

target_link_libraries(${PROJECT_NAME} PRIVATE
  GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
