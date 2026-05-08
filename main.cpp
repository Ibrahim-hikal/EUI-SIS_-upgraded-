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
#include "Features_and_Functions/Manage_Members/Admin/Add_Students/header/Add_Students.h"
#include "Features_and_Functions/Manage_Members/Admin/Add_Teachers/header/Add_Teachers.h"
#include "Features_and_Functions/Manage_Members/Admin/Assign_Courses/header/Assign_Courses.h"
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
        // Fetch and format Withdrawals
        auto w_reqs = AdminWithdrawalManager::fetch();
        std::vector<WithdrawalRequest> slint_w_reqs;
        for (const auto &r: w_reqs) {
            slint_w_reqs.push_back({slint::SharedString(r.student_id), slint::SharedString(r.course_code), slint::SharedString(r.reason)});
        }
        ui->set_pending_withdrawals(std::make_shared<slint::VectorModel<WithdrawalRequest>>(slint_w_reqs));

        // Fetch and format Excuses
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
        std::cout << "Withdrawal Submitted: " << code << std::endl;
    });

    ui->on_submit_excuse_req([&](slint::SharedString code, slint::SharedString week, slint::SharedString reason) {
        StudentExcuseManager::submit(current_id, std::string(code), std::string(week), std::string(reason));
        std::cout << "Excuse Submitted: " << code << std::endl;
    });

    ui->on_admin_handle_withdrawal([&](slint::SharedString id, slint::SharedString course, bool approved) {
        // The Manager now handles BOTH the request database and the student database
        AdminWithdrawalManager::process(std::string(id), std::string(course), approved);
        refresh_admin_requests();
    });

    ui->on_admin_handle_excuse(
        [&](slint::SharedString id, slint::SharedString course, slint::SharedString week, bool approved) {
            AdminExcuseManager::process(std::string(id), std::string(course), std::string(week), approved);
            refresh_admin_requests();
        });

    ui->on_admin_end_semester([&]() {
        Admin_Profile::get_instance().end_semester();
    });
    ui->on_admin_check_teacher_email([&](slint::SharedString email) -> slint::SharedString {
        return slint::SharedString(Assign_Course::get_teacher_name(string(email)));
    });

    ui->on_admin_check_assign_course([&](slint::SharedString code) -> slint::SharedString {
        return slint::SharedString(Assign_Course::get_course_name(string(code)));
    });

    ui->on_admin_assign_course_submit([&](slint::SharedString email, slint::SharedString code, slint::SharedString l1, slint::SharedString l2, slint::SharedString t1, slint::SharedString t2) {
        string success_msg = Assign_Course::assign_course(string(email), string(code), string(l1), string(l2), string(t1), string(t2));
        ui->set_assign_course_status_message(slint::SharedString(success_msg));
    });

    ui->on_check_credentials([&](const slint::SharedString &id, const slint::SharedString &pass, int role) {
        string string_id = string(id);
        Login_Status status = db.login(string_id, string(pass), static_cast<User_Role>(role));

        if (status == Login_Status::SUCCESS) {
            ui->set_login_error_state(false);
            current_id = string_id;

            if (role == 1) {
                // Student logic
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

                RegisteredCoursesManager rc_manager(current_id);
                auto courses_vec = rc_manager.get_registered_courses_with_attendance();
                std::vector<CourseInfo> std_courses;
                for (int i = 0; i < courses_vec.size(); ++i) std_courses.push_back(courses_vec[i]);
                auto courses_model = std::make_shared<slint::VectorModel<CourseInfo> >(std_courses);
                ui->set_my_courses(courses_model);

                std::vector<slint::SharedString> codes_only;
                for (const auto &course: std_courses) {
                    codes_only.push_back(course.course_code);
                }
                auto codes_model = std::make_shared<slint::VectorModel<slint::SharedString> >(codes_only);

                // 3. Send the string list to the UI
                ui->set_my_course_codes(codes_model);
                studentScheduleManager.load_student_schedule(current_id);
                studentScheduleManager.initUI(ui.operator->());
                global_request_manager = std::make_unique<Request_Courses>(current_id);

                // LOAD PREVIOUS ENROLLMENTS (FIXED)
                PreviousEnrollmentsManager pe(current_id);
                pe.load_student_data(ui.operator->());

                auto refresh_request_tables = [&ui]() {
                    if (!global_request_manager) return;
                    Student *student_ptr = global_request_manager->get_student();
                    if (!student_ptr) return;

                    std::vector<CourseInfo> available_slint_list;
                    std::vector<CourseInfo> requested_slint_list;

                    auto eligible_courses = student_ptr->get_eligible_courses();
                    auto requested_courses = student_ptr->requestedCourses;

                    // 1. Build Available Courses Model
                    for (const auto &c: eligible_courses) {
                        CourseInfo info;
                        info.course_code = slint::SharedString(c.code);
                        info.course_name = slint::SharedString(c.name);

                        // Check if this course is already in the requested list
                        info.is_requested = false;
                        for (const auto &req: requested_courses) {
                            if (req.code == c.code) {
                                info.is_requested = true;
                                break;
                            }
                        }
                        // Check if the student failed this in the past
                        info.is_failed = (student_ptr->failedCourses.find(c.code) != std::string::npos);

                        available_slint_list.push_back(info);
                    }

                    // 2. Build Requested Courses Model (Tracks 0/5 count!)
                    for (const auto &req: requested_courses) {
                        CourseInfo info;
                        info.course_code = slint::SharedString(req.code);
                        info.course_name = slint::SharedString(req.name);
                        requested_slint_list.push_back(info);
                    }

                    // 3. Send BOTH to Slint
                    ui->set_available_courses(std::make_shared<slint::VectorModel<CourseInfo> >(available_slint_list));
                    ui->set_requested_courses(std::make_shared<slint::VectorModel<CourseInfo> >(requested_slint_list));
                };

                // Trigger once immediately so the UI populates on login
                refresh_request_tables();

                // --- UPDATED BUTTON CLICK CALLBACK ---
                // --- NEW BUTTON CLICK CALLBACK WITH CONFLICT DETECTION ---
                ui->on_request_course([&ui, refresh_request_tables](slint::SharedString course_code,
                                                                    slint::SharedString l_day,
                                                                    slint::SharedString l_slot,
                                                                    slint::SharedString t_day,
                                                                    slint::SharedString t_slot) {
                    if (global_request_manager) {
                        string code_str = string(course_code);
                        size_t underscore_pos = code_str.find_last_of('_');
                        string clean_code = (underscore_pos != string::npos)
                                                ? code_str.substr(0, underscore_pos)
                                                : code_str;

                        Student *student_ptr = global_request_manager->get_student();

                        // 1. Check for time conflict FIRST
                        if (student_ptr->
                            has_time_conflict(string(l_day), string(l_slot), string(t_day), string(t_slot))) {
                            ui->set_status_message("Conflict: Time slot overlaps with an existing schedule.");
                            return;
                        }

                        // 2. Try to register (will fail if limit 5 reached)
                        bool success = global_request_manager->request_course(clean_code, string(l_day), string(l_slot), string(t_day), string(t_slot));

                        if (success) {
                            ui->set_status_message(
                                "Course " + slint::SharedString(clean_code) + " Requested Successfully!");
                            refresh_request_tables(); // INSTANT UI UPDATE
                        } else {
                            ui->set_status_message("Request Failed (Limit 5 reached).");
                        }
                    }
                });
            } else if (role == 2) {
                // Teacher Logic
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
                // Admin Logic
                auto &admin = Admin_Profile::get_instance();
                admin.load_profile(current_id);

                ui->set_user_name(admin.get_name().c_str());
                ui->set_user_email(admin.get_email().c_str());
                ui->set_admin_position(admin.get_position().c_str());

                string pfp_path = ImageManager::get_user_pfp_path(current_id);
                auto img = slint::Image::load_from_path(pfp_path.c_str());
                ui->set_user_profile_pic(img);

                adminManager.initUI(ui.operator->());
                refresh_admin_requests();

                // ---> THE CLEAN REACTIVE ID SETTER! <---
                ui->set_next_student_id(slint::SharedString(Add_Student::generate_student_id()));
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

    // ---> THE CLEAN REACTIVE SUBMIT BUTTON! <---
    ui->on_admin_add_student_submit([&](slint::SharedString name, slint::SharedString fac , slint::SharedString pass) {
        string success_string = Add_Student::add_student(string(name), string(fac) , string(pass));
        ui->set_add_student_status_message(slint::SharedString(success_string));
        ui->set_next_student_id(slint::SharedString(Add_Student::generate_student_id()));
    });
    ui->on_admin_add_teacher_submit([&](slint::SharedString first_name, slint::SharedString last_name , slint::SharedString pass) {
        string success_string = Add_Teacher::add_teacher(string(first_name), string(last_name) , string(pass));
        ui->set_add_teacher_status_message(slint::SharedString(success_string));
    });
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
        Admin_Profile::get_instance().reset();
        current_id = "";
        ui->set_user_name("");
        ui->set_user_id("");
        ui->set_user_profile_pic(slint::Image());
        ui->set_active_panel(0);
        ui->set_login_error_state(false);
    });
    ui->on_admin_handle_withdrawal([&](slint::SharedString id, slint::SharedString course, bool approved) {
        // The Manager now handles BOTH the request database and the student database
        AdminWithdrawalManager::process(std::string(id), std::string(course), approved);

        // Refresh the UI display
        refresh_admin_requests();
    });
    ui->run();
    return 0;
}