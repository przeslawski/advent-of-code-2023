cmake_minimum_required(VERSION 3.24)

project(advent_of_code_2023 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# find_package(GTest REQUIRED)
find_package(range-v3 REQUIRED)
find_package(fmt REQUIRED)
find_package(Boost REQUIRED)

set(libs Boost::boost fmt::fmt range-v3::range-v3)
# set(test_libs GTest::gtest GTest::gtest_main)

set(inc ${CMAKE_SOURCE_DIR}/inc)

add_subdirectory(day1)
add_subdirectory(day2)
add_subdirectory(day3)
add_subdirectory(day4)
add_subdirectory(day5)
add_subdirectory(day6)
add_subdirectory(day7)
add_subdirectory(day8)
add_subdirectory(day9)
add_subdirectory(day10)
add_subdirectory(day11)
