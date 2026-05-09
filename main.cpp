/*
 * File: main.cpp
 * Description: The core brain of the application. It launches the visual interface
 * and listens for user actions (like clicking login or changing a profile picture).
 * When an action happens, it calls the correct C++ backend function to do the real work.
 */

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "Features_and_Functions/Login_Page/header/Login_Page.h"
#include "Features_and_Functions/Profile_Pages/IProfile.h"
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

// Pointer to help track the current student's course requests
std::unique_ptr<Request_Courses> global_request_manager;

/*
 * Helper Tool: ProfileFactory
 * Instead of manually writing out "if student, use this; if teacher, use that"
 * every time, this helper tool simply asks for the user's role number (1, 2, or 3)
 * and automatically hands us the correct profile data back.
 */
class ProfileFactory {
public:
    static IProfile& get_profile(int role_type) {
        if (role_type == 1) return Student_Profile::get_instance();
        if (role_type == 2) return Teacher_Profile::get_instance();
        if (role_type == 3) return Admin_Profile::get_instance();
        throw std::invalid_argument("Error: Could not identify the user role.");
    }
};

int main() {
    // 1. Create the visual app window
    auto ui = Main_App::create();

    // 2. Prepare the backend databases and managers
    Login_Manager &db = Login_Manager::get_instance();
    TeacherGradesManager teacherGradesManager;
    AdminCourseManager adminManager("Databases/");
    TeacherAttendanceManager teacherAttendanceManager;
    TeacherScheduleManager teacherScheduleManager;
    StudentScheduleManager studentScheduleManager;

    // Variables to remember who is currently logged in
    string current_id = "";
    string current_email = "";

    /*
     * Custom task that grabs the newest requests from students (like dropping
     * a course or sending an absence excuse) and pushes them to the Admin screen.
     */
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

    // ------------------------------------------------------------------------
    // User Interface Button Links
    // Whenever a user clicks a button in Slint, these run the matching C++ code
    // ------------------------------------------------------------------------

    ui->on_submit_withdrawal_req([&](slint::SharedString code, slint::SharedString reason) {
        StudentWithdrawalManager::submit(current_id, std::string(code), std::string(reason));
        std::cout << "Withdrawal Submitted: " << code << std::endl;
    });

    ui->on_submit_excuse_req([&](slint::SharedString code, slint::SharedString week, slint::SharedString reason) {
        StudentExcuseManager::submit(current_id, std::string(code), std::string(week), std::string(reason));
        std::cout << "Excuse Submitted: " << code << std::endl;
    });

    ui->on_admin_handle_withdrawal([&](slint::SharedString id, slint::SharedString course, bool approved) {
        AdminWithdrawalManager::process(std::string(id), std::string(course), approved);
        refresh_admin_requests(); // Update the screen immediately
    });

    ui->on_admin_handle_excuse([&](slint::SharedString id, slint::SharedString course, slint::SharedString week, bool approved) {
        AdminExcuseManager::process(std::string(id), std::string(course), std::string(week), approved);
        refresh_admin_requests();
    });

    ui->on_admin_end_semester([&]() -> slint::SharedString {
        return slint::SharedString(Admin_Profile::get_instance().end_semester());
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

    // ------------------------------------------------------------------------
    // Master Login and Setup
    // Checks passwords and builds the user's dashboard based on who they are
    // ------------------------------------------------------------------------

    ui->on_check_credentials([&](const slint::SharedString &id, const slint::SharedString &pass, int role) {
        string string_id = string(id);

        // Ask the database if the password is correct
        Login_Status status = db.login(string_id, string(pass), static_cast<User_Role>(role));

        if (status == Login_Status::SUCCESS) {
            current_id = string_id;
            ui->set_login_error_state(false);
            ui->set_active_panel(role); // Swap the screen to Student, Teacher, or Admin

            // Get the specific profile data for whoever just logged in
            IProfile& active_profile = ProfileFactory::get_profile(role);
            active_profile.load_profile(current_id);

            // Print their name safely to the developer console
            std::cout << "LOGIN SUCCESS: " << active_profile << std::endl;

            // Show their name and picture on the screen
            ui->set_user_name(slint::SharedString(active_profile.get_name()));
            string pfp_path = ImageManager::get_user_pfp_path(current_id);
            auto img = slint::Image::load_from_path(pfp_path.c_str());
            ui->set_user_profile_pic(img);

            // Setup if it is a Student
            if (role == 1) {
                auto &student = Student_Profile::get_instance();
                ui->set_user_id(student.get_id().c_str());
                ui->set_user_Faculty(student.get_faculty().c_str());
                ui->set_student_gpa(student.get_gpa().c_str());
                ui->set_user_email(student.get_email().c_str());

                // Find the courses the student is taking
                RegisteredCoursesManager rc_manager(current_id);
                auto courses_vec = rc_manager.get_registered_courses_with_attendance();
                std::vector<CourseInfo> std_courses;
                for (int i = 0; i < courses_vec.size(); ++i) std_courses.push_back(courses_vec[i]);
                ui->set_my_courses(std::make_shared<slint::VectorModel<CourseInfo>>(std_courses));

                std::vector<slint::SharedString> codes_only;
                for (const auto &course: std_courses) codes_only.push_back(course.course_code);
                ui->set_my_course_codes(std::make_shared<slint::VectorModel<slint::SharedString>>(codes_only));

                // Build their schedule and past grades
                studentScheduleManager.load_student_schedule(current_id);
                studentScheduleManager.initUI(ui.operator->());
                global_request_manager = std::make_unique<Request_Courses>(current_id);

                PreviousEnrollmentsManager pe(current_id);
                pe.load_student_data(ui.operator->());

                // Task to update the list of courses they can request to join
                auto refresh_request_tables = [&ui]() {
                    if (!global_request_manager) return;
                    Student *student_ptr = global_request_manager->get_student();
                    if (!student_ptr) return;

                    std::vector<CourseInfo> available_slint_list;
                    std::vector<CourseInfo> requested_slint_list;

                    auto eligible_courses = student_ptr->get_eligible_courses();
                    auto requested_courses = student_ptr->requestedCourses;

                    // Fill the "Available" list
                    for (const auto &c: eligible_courses) {
                        CourseInfo info;
                        info.course_code = slint::SharedString(c.code);
                        info.course_name = slint::SharedString(c.name);
                        info.is_requested = false;
                        for (const auto &req: requested_courses) {
                            if (req.code == c.code) {
                                info.is_requested = true;
                                break;
                            }
                        }
                        info.is_failed = (student_ptr->failedCourses.find(c.code) != std::string::npos);
                        available_slint_list.push_back(info);
                    }

                    // Fill the "Already Requested" list
                    for (const auto &req: requested_courses) {
                        CourseInfo info;
                        info.course_code = slint::SharedString(req.code);
                        info.course_name = slint::SharedString(req.name);
                        requested_slint_list.push_back(info);
                    }

                    ui->set_available_courses(std::make_shared<slint::VectorModel<CourseInfo>>(available_slint_list));
                    ui->set_requested_courses(std::make_shared<slint::VectorModel<CourseInfo>>(requested_slint_list));
                };

                refresh_request_tables();

                // Button to request joining a course
                ui->on_request_course([&ui, refresh_request_tables](slint::SharedString course_code, slint::SharedString l_day, slint::SharedString l_slot, slint::SharedString t_day, slint::SharedString t_slot) {
                    if (global_request_manager) {
                        string code_str = string(course_code);
                        size_t underscore_pos = code_str.find_last_of('_');
                        string clean_code = (underscore_pos != string::npos) ? code_str.substr(0, underscore_pos) : code_str;

                        Student *student_ptr = global_request_manager->get_student();

                        // Check if the time slot clashes with a course they already have
                        if (student_ptr->has_time_conflict(string(l_day), string(l_slot), string(t_day), string(t_slot))) {
                            ui->set_status_message("Conflict: Time slot overlaps with an existing schedule.");
                            return;
                        }

                        // Save the request
                        bool success = global_request_manager->request_course(clean_code, string(l_day), string(l_slot), string(t_day), string(t_slot));
                        if (success) {
                            ui->set_status_message("Course " + slint::SharedString(clean_code) + " Requested Successfully!");
                            refresh_request_tables();
                        } else {
                            ui->set_status_message("Request Failed (Limit 5 reached).");
                        }
                    }
                });

            // Setup if it is a Teacher
            } else if (role == 2) {
                auto &teacher = Teacher_Profile::get_instance();
                current_email = teacher.get_email();
                ui->set_user_email(current_email.c_str());

                // Find the classes they teach and build their schedule
                teacherScheduleManager.load_teacher_schedule(teacher.get_name());
                teacherScheduleManager.initUI(ui.operator->());

                std::vector<slint::SharedString> display_list;
                for (const auto &course_str: teacher.get_courses_taught()) {
                    display_list.push_back(slint::SharedString(course_str));
                }
                ui->set_teacher_profile_display_list(std::make_shared<slint::VectorModel<slint::SharedString>>(display_list));

                // Start the Attendance and Grading dashboards
                teacherAttendanceManager.initUI(ui.operator->(), teacher.get_name());
                teacherGradesManager.initUI(ui.operator->(), teacher.get_name());

            // Setup if it is an Admin
            } else if (role == 3) {
                auto &admin = Admin_Profile::get_instance();
                ui->set_user_email(admin.get_email().c_str());
                ui->set_admin_position(admin.get_position().c_str());

                adminManager.initUI(ui.operator->());
                refresh_admin_requests();

                // Show a preview of what the next generated Student ID will look like
                ui->set_next_student_id(slint::SharedString(Add_Student::generate_student_id()));
            }

        // Show an error on the screen if the password or ID was wrong
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

    ui->on_admin_add_student_submit([&](slint::SharedString name, slint::SharedString fac , slint::SharedString pass) {
        string success_string = Add_Student::add_student(string(name), string(fac) , string(pass));
        ui->set_add_student_status_message(slint::SharedString(success_string));
        ui->set_next_student_id(slint::SharedString(Add_Student::generate_student_id()));
    });

    ui->on_admin_add_teacher_submit([&](slint::SharedString first_name, slint::SharedString last_name , slint::SharedString pass) {
        string success_string = Add_Teacher::add_teacher(string(first_name), string(last_name) , string(pass));
        ui->set_add_teacher_status_message(slint::SharedString(success_string));
    });

    // ------------------------------------------------------------------------
    // General Settings (Profile Images & Logout)
    // ------------------------------------------------------------------------

    // Let the user select an image from their PC and save it to the system
    ui->on_change_picture([&]() {
        string source = ImageManager::select_image_dialog();
        if (source.empty()) return;

        string new_path = ImageManager::save_profile_picture(source, current_id);
        if (!new_path.empty()) {
            ui->set_user_profile_pic(slint::Image()); // Remove the old image
            auto fresh_img = slint::Image::load_from_path(new_path.c_str());
            ui->set_user_profile_pic(fresh_img); // Show the new image
        }
    });

    // Safely wipe out all active data and kick the user back to the login page
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

    // Keeps the visual application running until the user clicks the 'X' to close it
    ui->run();
    return 0;
}