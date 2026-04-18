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
        Features_and_Functions/Course_Management/Teacher_Course_Management/Main_Panel_For_All_Courses/header/Main_Panel_For_All_Courses.h
        Features_and_Functions/Course_Management/Teacher_Course_Management/Main_Panel_For_All_Courses/src/Main_Panel_For_All_Courses.cpp
        Features_and_Functions/Course_Management/Teacher_Course_Management/Sub_Panel_Per_Chosen_Course/header/Sub_Panel_Per_Chosen_Course.h
        Features_and_Functions/Course_Management/Teacher_Course_Management/Sub_Panel_Per_Chosen_Course/src/Sub_Panel_Per_Chosen_Course.cpp
        Features_and_Functions/Academic_Requests/Attendance_Excuses/Admin_Attendance_Excuses/header/Admin_Attendance_Excuses.h
        Features_and_Functions/Academic_Requests/Attendance_Excuses/Admin_Attendance_Excuses/src/Admin_Attendance_Excuses.cpp
        Features_and_Functions/Academic_Requests/Attendance_Excuses/Student_Attendance_Excuses/header/Student_Attendance_Excuses.h
        Features_and_Functions/Academic_Requests/Attendance_Excuses/Student_Attendance_Excuses/src/Student_Attendance_Excuses.cpp
        Features_and_Functions/Profile_Pages/ImageManager.cpp
        Features_and_Functions/Profile_Pages/ImageManager.h
        # Note: Make sure there is a closing parenthesis here after the last .cpp file
)

# 3. Add Slint files
slint_target_sources(main_app
        Features_and_Functions/main.slint


)

# 4. Link everything
target_link_libraries(main_app PRIVATE Slint::Slint)