cmake_minimum_required(VERSION 3.15)
project(rbgame3D VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_executable(rbgame3D 
                main.cpp
                model.cpp
                shader.cpp
                game.cpp
                glad.c)     
target_link_libraries(rbgame3D PRIVATE glfw assimp GL)
