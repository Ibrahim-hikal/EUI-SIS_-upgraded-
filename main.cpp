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
#include "Features_and_Functions/Profile_Pages/Admin_Profile/header/Admin_Profile.h"
#include "Features_and_Functions/Schedule/Student_Schedule/header/Student_Schedule_Manager.h"
#include "Features_and_Functions/Schedule/Teacher_Schedule/header/Teacher_Schedule_Manager.h"

using namespace std;

std::unique_ptr<Request_Courses> global_request_manager;

int main() {
    auto ui = Main_App::create();
    Login_Manager &db = Login_Manager::get_instance();
    TeacherGradesManager teacherGradesManager;
    AdminCourseManager adminManager("Databases/");
    TeacherAttendanceManager teacherAttendanceManager;
    StudentScheduleManager scheduleManager;
    TeacherScheduleManager teacherScheduleManager;

    string current_id = "";
    string current_email = "";

    ui->on_check_credentials([&](const slint::SharedString &id, const slint::SharedString &pass, int role) {
        string string_id = string(id);
        Login_Status status = db.login(string_id, string(pass), static_cast<User_Role>(role));

        if (status == Login_Status::SUCCESS) {
            ui->set_login_error_state(false);
            current_id = string_id;

            if (role == 1) {
                auto &student = Student_Profile::get_instance();
                student.load_profile(current_id);

                ui->set_user_name(student.get_name().c_str());
                ui->set_user_id(student.get_id().c_str());
                ui->set_user_Faculty(student.get_faculty().c_str());
                ui->set_student_gpa(student.get_gpa().c_str());
                ui->set_user_email(student.get_email().c_str());

                string pfp_path = ImageManager::get_user_pfp_path(current_id);
                auto img = slint::Image::load_from_path(pfp_path.c_str());
                ui->set_user_profile_pic(img);

                // --- ADDED THIS FOR SCHEDULE ---
                scheduleManager.load_student_schedule(current_id);
                scheduleManager.initUI(ui.operator->());
                // -------------------------------

                RegisteredCoursesManager rc_manager(current_id);
                auto courses_vec = rc_manager.get_registered_courses_with_attendance();
                std::vector<CourseInfo> std_courses;
                for (int i = 0; i < courses_vec.size(); ++i) std_courses.push_back(courses_vec[i]);
                auto courses_model = std::make_shared<slint::VectorModel<CourseInfo> >(std_courses);
                ui->set_my_courses(courses_model);

                global_request_manager = std::make_unique<Request_Courses>(current_id);
                std::vector<CourseInfo> available_vec;
                auto iterator = global_request_manager->get_available_courses_inorder();
                while(iterator->has_next()) {
                    Course& c = iterator->next();
                    available_vec.push_back({slint::SharedString(c.code), slint::SharedString(c.name)});
                }
                if (available_vec.empty()) {
                    available_vec.push_back({slint::SharedString("TEST101"), slint::SharedString("Debug Course")});
                    available_vec.push_back({slint::SharedString("TEST102"), slint::SharedString("Another Debug Course")});
                }
                ui->set_available_courses(std::make_shared<slint::VectorModel<CourseInfo>>(available_vec));
                auto available_model = std::make_shared<slint::VectorModel<CourseInfo>>(available_vec);
                ui->set_available_courses(available_model);

                ui->on_request_course([&, ui](slint::SharedString course_code) {
                    if (global_request_manager) {
                        string code_str = string(course_code);
                        size_t underscore_pos = code_str.find_last_of('_');
                        string clean_code = (underscore_pos != string::npos)
                                            ? code_str.substr(0, underscore_pos)
                                            : code_str;

                        bool success = global_request_manager->request_course(clean_code);

                        if (success) {
                            ui->set_status_message("Course " + slint::SharedString(clean_code) + " Requested!");
                        } else {
                            ui->set_status_message("Request Failed (Limit 5 reached or Not found).");
                        }
                    }
                });


            } else if (role == 2) {
                auto &teacher = Teacher_Profile::get_instance();
                teacher.load_profile(current_id);
                teacherScheduleManager.load_teacher_schedule(teacher.get_name());
                teacherScheduleManager.initUI(ui.operator->());
                current_email = teacher.get_email();
                string pfp_path = ImageManager::get_user_pfp_path(current_id);
                auto img = slint::Image::load_from_path(pfp_path.c_str());
                ui->set_user_profile_pic(img);
                ui->set_user_name(teacher.get_name().c_str());
                ui->set_user_email(current_email.c_str());

                std::vector<slint::SharedString> display_list;
                for (const auto &course_str: teacher.get_courses_taught()) {
                    display_list.push_back(slint::SharedString(course_str));
                }
                auto display_model = std::make_shared<slint::VectorModel<slint::SharedString> >(display_list);

                ui->set_teacher_profile_display_list(display_model);
                teacherAttendanceManager.initUI(ui.operator->(), teacher.get_name());
                teacherGradesManager.initUI(ui.operator->(), teacher.get_name());
            } else if (role == 3) {
                auto &admin = Admin_Profile::get_instance();
                admin.load_profile(current_id);
                ui->set_user_name(admin.get_name().c_str());
                ui->set_user_email(admin.get_email().c_str());
                ui->set_admin_position(admin.get_position().c_str());

                string pfp_path = ImageManager::get_user_pfp_path(current_id);
                auto img = slint::Image::load_from_path(pfp_path.c_str());
                ui->set_user_profile_pic(img);

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

    ui->on_admin_next_student([&]() { adminManager.nextStudent(ui.operator->()); });
    ui->on_admin_previous_student([&]() { adminManager.prevStudent(ui.operator->()); });
    ui->on_admin_submit_decisions([&]() { adminManager.submitDecisions(ui.operator->()); });

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

    ui->on_logout([&]() {
        Student_Profile::get_instance().reset();
        Teacher_Profile::get_instance().reset();
        Admin_Profile::get_instance().reset();
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