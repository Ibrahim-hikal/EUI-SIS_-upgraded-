cmake_minimum_required(VERSION 3.31)
project(Project)

set(CMAKE_CXX_STANDARD 20)

add_executable(Project main.cpp
        Login_Page/src/Login_page.cpp
        Login_Page/header/Login_Page.h)
