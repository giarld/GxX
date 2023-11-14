cmake_minimum_required(VERSION 3.20)

add_executable(TestGxX
        src/test_window.cpp
)

target_link_libraries(TestGxX gx-x)
