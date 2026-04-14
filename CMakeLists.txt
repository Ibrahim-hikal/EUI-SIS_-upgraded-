cmake_minimum_required(VERSION 3.31)
project(Project)

set(CMAKE_CXX_STANDARD 20)

add_executable(Project main.cpp
        Login_Page/src/Login_page.cpp
        Login_Page/header/Login_Page.h
        Profile_Pages/Admin_Profile/header/Admin_Profile.h
        Profile_Pages/Admin_Profile/src/Admin_Profile.cpp
        Profile_Pages/Student_Profile/header/Student_Profile.h
        Profile_Pages/Student_Profile/src/Student_Profile.cpp
        Profile_Pages/Teacher_Profile/header/Teacher_Profile.h
        Profile_Pages/Teacher_Profile/src/Teacher_Profile.cpp)
