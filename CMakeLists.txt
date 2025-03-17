cmake_minimum_required(VERSION 3.30)
project(HW1)

set(CMAKE_CXX_STANDARD 11)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLEW
find_package(GLEW REQUIRED)

find_package(glfw3 3.3.10 REQUIRED)

# Add the executable
add_executable(HW1 main.cpp
        BezierCurveGenerator.h
        BezierCurveGenerator.cpp
        BezierMeshGenerator.cpp
        BezierMeshGenerator.h)

# Link OpenGL, GLEW, and GLFW to the target executable
target_link_libraries(HW1 OpenGL::GL GLEW::GLEW glfw)
