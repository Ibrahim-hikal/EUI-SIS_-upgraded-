cmake_minimum_required(VERSION 3.21)
project(EUI_SIS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Slint package
find_package(Slint REQUIRED)

# Include ALL source files
set(SOURCES
        main.cpp
        Features_and_Functions/Profile_Pages/ImageManager.cpp
        Features_and_Functions/Login_Page/src/Login_page.cpp
        Features_and_Functions/Profile_Pages/Student_Profile/src/Student_Profile.cpp
        Features_and_Functions/Profile_Pages/Teacher_Profile/src/Teacher_Profile.cpp
        Features_and_Functions/Profile_Pages/Admin_Profile/src/Admin_Profile.cpp
        Features_and_Functions/Course_Management/Student_Course_Management/Request_Courses/src/Request_Courses.cpp
        Features_and_Functions/Course_Management/Student_Course_Management/Registered_Courses/src/Registered_Courses.cpp
        Features_and_Functions/Course_Management/Student_Course_Management/Previous_Enrollments/src/Previous_Enrollments.cpp
        Features_and_Functions/Course_Management/Grades/Teacher/src/Teacher_Grades.cpp
        Features_and_Functions/Course_Management/Attendance/Teacher/src/Teacher_Attendance.cpp
        Features_and_Functions/Course_Management/Admin_Course_Management/Accept_and_Refuse_Courses/src/Accept_and_Refuse_Courses.cpp

        # --- ACADEMIC REQUESTS SOURCES ---
        Features_and_Functions/Academic_Requests/Course_Withdrawal/Student_Course_Withdrawal/src/Student_Course_Withdrawal.cpp
        Features_and_Functions/Academic_Requests/Course_Withdrawal/Admin_Course_Withdrawal/src/Admin_Course_Withdrawal.cpp
        Features_and_Functions/Academic_Requests/Attendance_Excuses/Student_Attendance_Excuses/src/Student_Attendance_Excuses.cpp
        Features_and_Functions/Academic_Requests/Attendance_Excuses/Admin_Attendance_Excuses/src/Admin_Attendance_Excuses.cpp
)

# Create executable
add_executable(main_app ${SOURCES})

# Link Slint library
target_link_libraries(main_app PRIVATE Slint::Slint)

# Slint compiler for .slint files (FIXED: Removed the word PRIVATE from here)
slint_target_sources(main_app
        Features_and_Functions/main.slint
)

# Include directories
target_include_directories(main_app PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        Features_and_Functions
)

