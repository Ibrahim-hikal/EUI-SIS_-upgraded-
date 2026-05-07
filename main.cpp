#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "Features_and_Functions/Login_Page/header/Login_Page.h"
#include "Features_and_Functions/Profile_Pages/Student_Profile/header/Student_Profile.h"
#include "Features_and_Functions/Profile_Pages/Teacher_Profile/header/Teacher_Profile.h"
#include "Features_and_Functions/Profile_Pages/ImageManager.h"
#include "Features_and_Functions/Course_Management/Admin_Course_Management/Accept_and_Refuse_Courses/header/Accept_and_Refuse_Courses.h"
#include "main.h"
#include "Features_and_Functions/Course_Management/Attendance/Teacher/header/Teacher_Attendance.h"
#include "Features_and_Functions/Course_Management/Student_Course_Management/Request_Courses/header/Request_Courses.h"
#include "Features_and_Functions/Course_Management/Student_Course_Management/Registered_Courses/header/Registered_Courses.h"
#include "Features_and_Functions/Course_Management/Student_Course_Management/Previous_Enrollments/header/Previous_Enrollments.h"
#include "Features_and_Functions/Course_Management/Grades/Teacher/header/Teacher_Grades.h"
#include "Features_and_Functions/Profile_Pages/Admin_Profile/header/Admin_Profile.h"
#include "Features_and_Functions/Schedule/Student_Schedule/header/Student_Schedule_Manager.h"
#include "Features_and_Functions/Schedule/Teacher_Schedule/header/Teacher_Schedule_Manager.h"

#include "Features_and_Functions/Academic_Requests/Course_Withdrawal/Student_Course_Withdrawal/header/Student_Course_Withdrawal.h"
#include "Features_and_Functions/Academic_Requests/Course_Withdrawal/Admin_Course_Withdrawal/header/Admin_Course_Withdrawal.h"
#include "Features_and_Functions/Academic_Requests/Attendance_Excuses/Student_Attendance_Excuses/header/Student_Attendance_Excuses.h"
#include "Features_and_Functions/Academic_Requests/Attendance_Excuses/Admin_Attendance_Excuses/header/Admin_Attendance_Excuses.h"

using namespace std;

std::unique_ptr<Request_Courses> global_request_manager;

int main() {
    auto ui = Main_App::create();
    Login_Manager &db = Login_Manager::get_instance();
    TeacherGradesManager teacherGradesManager;

    AdminCourseManager adminManager("Databases/");
    TeacherAttendanceManager teacherAttendanceManager;
    TeacherScheduleManager teacherScheduleManager;
    StudentScheduleManager studentScheduleManager;
    string current_id = "";
    string current_email = "";

    // 1. ADMIN REQUESTS REFRESH LOGIC
    auto refresh_admin_requests = [&]() {
        auto w_reqs = AdminWithdrawalManager::fetch();
        std::vector<WithdrawalRequest> slint_w_reqs;
        for (const auto &r: w_reqs) {
            slint_w_reqs.push_back({slint::SharedString(r.student_id), slint::SharedString(r.course_code), slint::SharedString(r.reason)});
        }
        ui->set_pending_withdrawals(std::make_shared<slint::VectorModel<WithdrawalRequest>>(slint_w_reqs));

        auto e_reqs = AdminExcuseManager::fetch();
        std::vector<ExcuseRequest> slint_e_reqs;
        for (const auto &r: e_reqs) {
            slint_e_reqs.push_back({slint::SharedString(r.student_id), slint::SharedString(r.course_code), slint::SharedString(r.week), slint::SharedString(r.reason)});
        }
        ui->set_pending_excuses(std::make_shared<slint::VectorModel<ExcuseRequest>>(slint_e_reqs));
    };

    // 2. ACADEMIC REQUEST CALLBACKS
    ui->on_submit_withdrawal_req([&](slint::SharedString code, slint::SharedString reason) {
        StudentWithdrawalManager::submit(current_id, std::string(code), std::string(reason));
    });

    ui->on_submit_excuse_req([&](slint::SharedString code, slint::SharedString week, slint::SharedString reason) {
        StudentExcuseManager::submit(current_id, std::string(code), std::string(week), std::string(reason));
    });

    ui->on_admin_handle_withdrawal([&](slint::SharedString id, slint::SharedString course, bool approved) {
        AdminWithdrawalManager::process(std::string(id), std::string(course), approved);
        refresh_admin_requests();
    });

    ui->on_admin_handle_excuse([&](slint::SharedString id, slint::SharedString course, slint::SharedString week, bool approved) {
        AdminExcuseManager::process(std::string(id), std::string(course), std::string(week), approved);
        refresh_admin_requests();
    });

    // 3. MAIN LOGIN LOGIC
    ui->on_check_credentials([&](const slint::SharedString &id, const slint::SharedString &pass, int role) {
        string string_id = string(id);
        Login_Status status = db.login(string_id, string(pass), static_cast<User_Role>(role));

        if (status == Login_Status::SUCCESS) {
            ui->set_login_error_state(false);
            current_id = string_id;

            if (role == 1) { // Student logic
                auto &student = Student_Profile::get_instance();
                student.load_profile(current_id);
                ui->set_user_name(student.get_name().c_str());
                ui->set_user_id(student.get_id().c_str());
                ui->set_user_Faculty(student.get_faculty().c_str());
                ui->set_student_gpa(student.get_gpa().c_str());
                ui->set_user_email(student.get_email().c_str());
                ui->set_user_profile_pic(slint::Image::load_from_path(ImageManager::get_user_pfp_path(current_id).c_str()));

                RegisteredCoursesManager rc_manager(current_id);
                auto courses_vec = rc_manager.get_registered_courses_with_attendance();
                std::vector<CourseInfo> std_courses;
                std::vector<slint::SharedString> codes_only;
                for (const auto &c : courses_vec) {
                    std_courses.push_back(c);
                    codes_only.push_back(c.course_code);
                }
                ui->set_my_courses(std::make_shared<slint::VectorModel<CourseInfo>>(std_courses));
                ui->set_my_course_codes(std::make_shared<slint::VectorModel<slint::SharedString>>(codes_only));

                studentScheduleManager.load_student_schedule(current_id);
                studentScheduleManager.initUI(ui.operator->());
                global_request_manager = std::make_unique<Request_Courses>(current_id);

                auto refresh_request_tables = [&ui]() {
                    if (!global_request_manager) return;
                    Student *s_ptr = global_request_manager->get_student();
                    if (!s_ptr) return;
                    std::vector<CourseInfo> avail, reqs;
                    for (const auto &c : s_ptr->get_eligible_courses()) {
                        CourseInfo info;
                        info.course_code = slint::SharedString(c.code);
                        info.course_name = slint::SharedString(c.name);
                        info.is_requested = false;
                        for (const auto &r : s_ptr->requestedCourses) if (r.code == c.code) info.is_requested = true;
                        info.is_failed = (s_ptr->failedCourses.find(c.code) != std::string::npos);
                        avail.push_back(info);
                    }
                    for (const auto &r : s_ptr->requestedCourses) {
                        reqs.push_back({slint::SharedString(r.code), slint::SharedString(r.name)});
                    }
                    ui->set_available_courses(std::make_shared<slint::VectorModel<CourseInfo>>(avail));
                    ui->set_requested_courses(std::make_shared<slint::VectorModel<CourseInfo>>(reqs));
                };
                refresh_request_tables();


                auto refresh_prev = [&ui, &current_id]() {
    std::ifstream file("Data_on_Each_Student.csv");
    if (!file.is_open()) return;

    std::string line;
    std::string sid, sname, spraw, sfraw; // Define them here so they are in scope

    std::getline(file, line); // Skip header

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);

        // 1. Get ID and Name
        if (!std::getline(ss, sid, ',')) continue;
        if (!std::getline(ss, sname, ',')) continue;

        // 2. Get the strings for Passed and Failed courses
        if (!std::getline(ss, spraw, ',')) spraw = "";
        if (!std::getline(ss, sfraw, ',')) sfraw = "";

        // Trim the ID to make sure comparison works
        sid.erase(0, sid.find_first_not_of(" \t\r\n"));
        sid.erase(sid.find_last_not_of(" \t\r\n") + 1);

        if (sid == current_id) {
            auto parse_to_model = [](std::string r) {
                auto v = std::make_shared<slint::VectorModel<CourseInfo>>();

                // Remove spaces and carriage returns from the string
                r.erase(std::remove(r.begin(), r.end(), '\r'), r.end());
                r.erase(std::remove(r.begin(), r.end(), ' '), r.end());

                if (r == "-1" || r.empty()) return v;

                std::stringstream rss(r);
                std::string seg;
                // Using '^' as the separator between courses in the same column
                while (std::getline(rss, seg, '^')) {
                    if (seg.empty()) continue;
                    CourseInfo info;
                    info.course_code = slint::SharedString(seg);
                    info.course_name = slint::SharedString("Course " + seg);
                    info.is_failed = false;
                    info.is_requested = false;
                    v->push_back(info);
                }
                return v;
            };

            // Now 'spraw' and 'sfraw' are definitely resolved and available
            ui->set_passed_courses(parse_to_model(spraw));
            ui->set_failed_courses(parse_to_model(sfraw));
            break;
         }
        }

};



                ui->on_request_course([&ui, refresh_request_tables](slint::SharedString c, slint::SharedString ld, slint::SharedString ls, slint::SharedString td, slint::SharedString ts) {
                    if (global_request_manager) {
                        string code_str = string(c);
                        if (global_request_manager->get_student()->has_time_conflict(string(ld), string(ls), string(td), string(ts))) {
                            ui->set_status_message("Conflict: Time slot overlaps.");
                            return;
                        }
                        if (global_request_manager->request_course(code_str, string(ld), string(ls), string(td), string(ts))) {
                            ui->set_status_message("Requested Successfully!");
                            refresh_request_tables();
                        } else ui->set_status_message("Limit reached.");
                    }
                });

            } else if (role == 2) { // Teacher Logic
                auto &t = Teacher_Profile::get_instance();
                t.load_profile(current_id);
                teacherScheduleManager.load_teacher_schedule(t.get_name());
                teacherScheduleManager.initUI(ui.operator->());
                ui->set_user_name(t.get_name().c_str());
                ui->set_user_email(t.get_email().c_str());
                ui->set_user_profile_pic(slint::Image::load_from_path(ImageManager::get_user_pfp_path(current_id).c_str()));
                std::vector<slint::SharedString> disp;
                for (const auto &c : t.get_courses_taught()) disp.push_back(slint::SharedString(c));
                ui->set_teacher_profile_display_list(std::make_shared<slint::VectorModel<slint::SharedString>>(disp));
                teacherAttendanceManager.initUI(ui.operator->(), t.get_name());
                teacherGradesManager.initUI(ui.operator->(), t.get_name());

            } else if (role == 3) { // Admin Logic
                auto &a = Admin_Profile::get_instance();
                a.load_profile(current_id);
                ui->set_user_name(a.get_name().c_str());
                ui->set_user_email(a.get_email().c_str());
                ui->set_admin_position(a.get_position().c_str());
                ui->set_user_profile_pic(slint::Image::load_from_path(ImageManager::get_user_pfp_path(current_id).c_str()));
                adminManager.initUI(ui.operator->());
                refresh_admin_requests();
            }
            ui->set_active_panel(role);
        } else {
            ui->set_login_error_state(true);
            if (status == Login_Status::PASSWORD_INCORRECT) ui->set_login_error_msg("Incorrect password.");
            else if (status == Login_Status::USER_NOT_FOUND) ui->set_login_error_msg("Account not found.");
            else if (status == Login_Status::ROLE_MISMATCH) ui->set_login_error_msg("Role mismatch.");
        }
    }); // This closes on_check_credentials

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
            ui->set_user_profile_pic(slint::Image::load_from_path(new_path.c_str()));
        }
    });

    // Logout Callback
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