cmake_minimum_required(VERSION 3.31)
project(filedialog
    LANGUAGES CXX
    DESCRIPTION "Simple example on using the File Dialog COM API"
    VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_GENERATOR Ninja)

add_executable(filedialog WIN32 main.cpp)