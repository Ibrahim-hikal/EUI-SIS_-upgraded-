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

using namespace std;

int main() {
    auto ui = Main_App::create();
    Login_Manager& db = Login_Manager::get_instance();

    // Declare Managers (so they live for the duration of the program)
    AdminCourseManager adminManager("Databases/");
    TeacherAttendanceManager teacherAttendanceManager;

    string current_id = "";

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
                teacher.load_profile(current_id);

                // Keep main.cpp clean - delegate entirely to the Manager
                teacherAttendanceManager.initUI(ui.operator->(), teacher.get_name());
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

    //Student Request courses Callback
    ui->on_request_courses_clicked([&](slint::SharedString id, slint::SharedString name, slint::SharedString faculty) {
        current_id = std::string(id.data());
        Request_Courses backend(current_id);

        ui->set_student_id(id);
        ui->set_student_name(name);
        ui->set_faculty(faculty);

        // Clean, normal code! No namespaces, no messy pointer declarations.
        auto ui_courses_model = std::make_shared<slint::VectorModel<CourseInfo>>();

        auto iterator = backend.get_available_courses_inorder();
        while (iterator->has_next()) {
            Course& c = iterator->next();

            CourseInfo ui_course;
            ui_course.course_code = slint::SharedString(c.code.c_str());
            ui_course.course_name = slint::SharedString(c.name.c_str());

            ui_courses_model->push_back(ui_course);
        }

        ui->set_available_courses(ui_courses_model); 
    });

    ui->on_request_course([&](slint::SharedString requested_code) {
        if (current_id.empty()) return;
        Request_Courses backend(current_id);
        backend.request_course(std::string(requested_code.data()));
    });

    // Handle the "Request" button click from the table
    ui->on_request_course([&](slint::SharedString requested_code) {
        if (current_id.empty()) return;

        Request_Courses backend(current_id);
        std::string code_str = std::string(requested_code.data());

        bool success = backend.request_course(code_str);

        if (success) {
            std::cout << "Successfully requested course: " << code_str << std::endl;
            // You can add a success message to the UI here if you want!
        } else {
            std::cout << "Failed to request course: " << code_str << std::endl;
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