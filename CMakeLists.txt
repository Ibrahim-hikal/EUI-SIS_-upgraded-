cmake_minimum_required(VERSION 3.31)
project(Project)

set(CMAKE_CXX_STANDARD 20)

# 1. Point to where the installer put Slint
# Change this path if you installed it elsewhere!

# 2. Find the package locally
find_package(Slint REQUIRED)

# 2. Define the app and ALL source files
add_executable(main_app
        main.cpp
        Features_and_Functions/Login_Page/src/Login_page.cpp
        Features_and_Functions/Login_Page/header/Login_Page.h
        Features_and_Functions/Profile_Pages/Admin_Profile/header/Admin_Profile.h
        Features_and_Functions/Profile_Pages/Admin_Profile/src/Admin_Profile.cpp
        Features_and_Functions/Profile_Pages/Student_Profile/header/Student_Profile.h
        Features_and_Functions/Profile_Pages/Student_Profile/src/Student_Profile.cpp
        Features_and_Functions/Profile_Pages/Teacher_Profile/header/Teacher_Profile.h
        Features_and_Functions/Profile_Pages/Teacher_Profile/src/Teacher_Profile.cpp
        Features_and_Functions/Course_Management/Admin_Course_Management/Accept_and_Refuse_Courses/header/Accept_and_Refuse_Courses.h
        Features_and_Functions/Course_Management/Admin_Course_Management/Accept_and_Refuse_Courses/src/Accept_and_Refuse_Courses.cpp
        Features_and_Functions/Course_Management/Student_Course_Management/Previous_Enrollments/header/Previous_Enrollments.h
        Features_and_Functions/Course_Management/Student_Course_Management/Previous_Enrollments/src/Previous_Enrollments.cpp
        Features_and_Functions/Course_Management/Student_Course_Management/Registered_Courses/header/Registered_Courses.h
        Features_and_Functions/Course_Management/Student_Course_Management/Registered_Courses/src/Registered_Courses.cpp
        Features_and_Functions/Course_Management/Student_Course_Management/Request_Courses/header/Request_Courses.h
        Features_and_Functions/Course_Management/Student_Course_Management/Request_Courses/src/Request_Courses.cpp
        Features_and_Functions/Academic_Requests/Attendance_Excuses/Admin_Attendance_Excuses/header/Admin_Attendance_Excuses.h
        Features_and_Functions/Academic_Requests/Attendance_Excuses/Admin_Attendance_Excuses/src/Admin_Attendance_Excuses.cpp
        Features_and_Functions/Academic_Requests/Attendance_Excuses/Student_Attendance_Excuses/header/Student_Attendance_Excuses.h
        Features_and_Functions/Academic_Requests/Attendance_Excuses/Student_Attendance_Excuses/src/Student_Attendance_Excuses.cpp
        Features_and_Functions/Profile_Pages/ImageManager.cpp
        Features_and_Functions/Profile_Pages/ImageManager.h
        Features_and_Functions/Course_Management/Attendance/Teacher/header/Teacher_Attendance.h
        Features_and_Functions/Course_Management/Attendance/Teacher/src/Teacher_Attendance.cpp
        Features_and_Functions/Course_Management/Admin_Course_Management/Accept_and_Refuse_Courses/src/Accept_and_Refuse_Courses.cpp
        Features_and_Functions/Course_Management/Shared/Student.cpp
        Features_and_Functions/Course_Management/Grades/Teacher/src/Teacher_Grades.cpp
        Features_and_Functions/Academic_Requests/Course_Withdrawal/Student_Course_Withdrawal/src/Student_Course_Withdrawal.cpp
        Features_and_Functions/Academic_Requests/Course_Withdrawal/Admin_Course_Withdrawal/src/Admin_Course_Withdrawal.cpp
        Features_and_Functions/Academic_Requests/Attendance_Excuses/Student_Attendance_Excuses/src/Student_Attendance_Excuses.cpp
        Features_and_Functions/Academic_Requests/Attendance_Excuses/Admin_Attendance_Excuses/src/Admin_Attendance_Excuses.cpp
        Features_and_Functions/Schedule/Student_Schedule/header/Student_Schedule_Manager.h
        Features_and_Functions/Schedule/Student_Schedule/src/Student_Schedule_Manager.cpp
        Features_and_Functions/Schedule/Teacher_Schedule/src/Teacher_Schedule_Manager.cpp
        Features_and_Functions/Schedule/Teacher_Schedule/header/Teacher_Schedule_Manager.h


 Features_and_Functions/Manage_Members/Admin/Add_Student/src.cpp
Features_and_Functions/Manage_Members/Admin/Add_Student/header.h

Features_and_Functions/Manage_Members/Admin/Add_Teacher/src.cpp
Features_and_Functions/Manage_Members/Admin/Add_Teacher/header.h

Features_and_Functions/Manage_Members/Admin/Assign_Courses/src.cpp
Features_and_Functions/Manage_Members/Admin/Assign_Courses/header.h
        Features_and_Functions/Manage_Members/Admin/Add_Students/src/Add_Students.cpp
        Features_and_Functions/Manage_Members/Admin/Add_Students/header/Add_Students.h
        Features_and_Functions/Manage_Members/Admin/Add_Students/src/Add_Students.cpp
        Features_and_Functions/Manage_Members/Admin/Add_Students/header/Add_Students.h
        Features_and_Functions/Manage_Members/Admin/Add_Students/src/Add_Students.cpp
        Features_and_Functions/Manage_Members/Admin/Add_Students/src/Add_Students.cpp
        Features_and_Functions/Manage_Members/Admin/Add_Students/header/Add_Students.h
        Features_and_Functions/Manage_Members/Admin/Add_Students/src/Add_Students.cpp
        Features_and_Functions/Manage_Members/Admin/Add_Students/src/Add_Students.cpp

)

# 3. Add Slint files
slint_target_sources(main_app
        Features_and_Functions/main.slint


)

# 4. Link everything
target_link_libraries(main_app PRIVATE Slint::Slint)