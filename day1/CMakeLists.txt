add_executable(day1_task1 task1.cpp)
target_link_libraries(day1_task1 PRIVATE ${libs})

add_executable(day1_task2 task2.cpp)
target_link_libraries(day1_task2 PRIVATE ${libs})
target_include_directories(day1_task2 PRIVATE ${inc})
