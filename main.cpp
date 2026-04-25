#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "Features_and_Functions/Login_Page/header/Login_Page.h"
#include "Features_and_Functions/Profile_Pages/Student_Profile/header/Student_Profile.h"
#include "Features_and_Functions/Profile_Pages/Teacher_Profile/header/Teacher_Profile.h"
#include "Features_and_Functions/Profile_Pages/ImageManager.h"
#include "Features_and_Functions/Course_Management/Admin_Course_Management/Accept_and_Refuse_Courses/header/Accept_and_Refuse_Courses.h"
#include "main.h"
#include "Features_and_Functions/Course_Management/Attendance/Teacher/header/Teacher_Attendance.h"
#include "Features_and_Functions/Course_Management/Student_Course_Management/Request_Courses/header/Request_Courses.h"
#include "Features_and_Functions/Course_Management/Student_Course_Management/Registered_Courses/header/Registered_Courses.h"
#include "Features_and_Functions/Course_Management/Grades/Teacher/header/Teacher_Grades.h"
using namespace std;

int main() {
    auto ui = Main_App::create();
    Login_Manager& db = Login_Manager::get_instance();
    TeacherGradesManager teacherGradesManager;

    // Declare Managers (so they live for the duration of the program)
    AdminCourseManager adminManager("Databases/");
    TeacherAttendanceManager teacherAttendanceManager;

    string current_id = "";
    string current_email = "";

    ui->on_check_credentials([&](const slint::SharedString& id, const slint::SharedString& pass, int role) {
        string string_id = string(id);
        Login_Status status = db.login(string_id, string(pass), static_cast<User_Role>(role));

        if (status == Login_Status::SUCCESS) {
            ui->set_login_error_state(false);
            current_id = string_id;

            if (role == 1) { // Student logic
                auto& student = Student_Profile::get_instance();
                student.load_profile(current_id);

                ui->set_user_name(student.get_name().c_str());
                ui->set_user_id(student.get_id().c_str());
                ui->set_user_Faculty(student.get_faculty().c_str());
                ui->set_student_gpa(student.get_gpa().c_str());
                ui->set_user_email(student.get_email().c_str());

                string pfp_path = ImageManager::get_user_pfp_path(current_id);
                auto img = slint::Image::load_from_path(pfp_path.c_str());
                ui->set_user_profile_pic(img);

                RegisteredCoursesManager rc_manager(current_id);
                auto courses_vec = rc_manager.get_registered_courses_with_attendance();
                std::vector<CourseInfo> std_courses;
                for (int i = 0; i < courses_vec.size(); ++i) std_courses.push_back(courses_vec[i]);
                auto courses_model = std::make_shared<slint::VectorModel<CourseInfo>>(std_courses);
                ui->set_my_courses(courses_model);
            }
            else if (role == 2) { // Teacher Logic
                auto& teacher = Teacher_Profile::get_instance();
                // Load the profile using the email (which is the current_id for teachers)
                teacher.load_profile(current_id);
                current_email = teacher.get_email();
                string pfp_path = ImageManager::get_user_pfp_path(current_id);
                auto img = slint::Image::load_from_path(pfp_path.c_str());
                ui->set_user_profile_pic(img);
                // Set common profile info
                ui->set_user_name(teacher.get_name().c_str());
                ui->set_user_email(current_email.c_str());

                //converting vectors to models for slint
                std::vector<slint::SharedString> display_list;
                for (const auto& course_str : teacher.get_courses_taught()) {
                    display_list.push_back(slint::SharedString(course_str));
                }
                auto display_model = std::make_shared<slint::VectorModel<slint::SharedString>>(display_list);

                // send the courses to the UI
                ui->set_teacher_profile_display_list(display_model);

                // Keep main.cpp clean - delegate entirely to the Manager
                teacherAttendanceManager.initUI(ui.operator->(), teacher.get_name());

                teacherGradesManager.initUI(ui.operator->(), teacher.get_name());
            }
            else if (role == 3) { // Admin Logic
                adminManager.initUI(ui.operator->());
            }

            ui->set_active_panel(role);
        } else {
            ui->set_login_error_state(true);
            if (status == Login_Status::PASSWORD_INCORRECT) {
                ui->set_login_error_msg("The password you entered is incorrect.");
            } else if (status == Login_Status::USER_NOT_FOUND) {
                ui->set_login_error_msg("Account not found. Check your ID/Email.");
            } else if (status == Login_Status::ROLE_MISMATCH) {
                ui->set_login_error_msg("Incorrect portal selected for this account.");
            }
        }
    });

    // Admin UI Callbacks
    ui->on_admin_next_student([&]() { adminManager.nextStudent(ui.operator->()); });
    ui->on_admin_previous_student([&]() { adminManager.prevStudent(ui.operator->()); });
    ui->on_admin_submit_decisions([&]() { adminManager.submitDecisions(ui.operator->()); });

    // Profile Pic Callbacks
    ui->on_change_picture([&]() {
        string source = ImageManager::select_image_dialog();
        if (source.empty()) return;
        string new_path = ImageManager::save_profile_picture(source, current_id);

        if (!new_path.empty()) {
            ui->set_user_profile_pic(slint::Image());
            auto fresh_img = slint::Image::load_from_path(new_path.c_str());
            ui->set_user_profile_pic(fresh_img);
        }
    });

    // Logout Callback
    ui->on_logout([&]() {
        Student_Profile::get_instance().reset();
        Teacher_Profile::get_instance().reset();
        current_id = "";
        ui->set_user_name("");
        ui->set_user_id("");
        ui->set_user_profile_pic(slint::Image());
        ui->set_active_panel(0);
        ui->set_login_error_state(false);
    });

    ui->run();
    return 0;
}