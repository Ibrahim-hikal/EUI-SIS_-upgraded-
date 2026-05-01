#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <memory>

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

using namespace std;

unique_ptr<Request_Courses> global_request_manager;

// ======================================================================
// NATIVE DATABASE LOGIC (Reads/Writes directly to Student CSV)
// ======================================================================
vector<string> split_csv(const string& line) {
    vector<string> cols; string current; bool in_quotes = false;
    for (char c : line) {
        if (c == '"') in_quotes = !in_quotes;
        else if (c == ',' && !in_quotes) { cols.emplace_back(current); current.clear(); }
        else if (c != '\r') current += c;
    }
    cols.emplace_back(current); return cols;
}

string join_csv(const vector<string>& cols) {
    string line;
    for (size_t i = 0; i < cols.size(); ++i) { line += cols[i]; if (i < cols.size() - 1) line += ","; }
    return line;
}

string clean_text(string s) {
    replace(s.begin(), s.end(), ',', ' ');
    replace(s.begin(), s.end(), '|', ' ');
    replace(s.begin(), s.end(), '^', ' ');
    return s;
}

void append_student_request(const string& student_id, size_t col_index, const string& request_data) {
    ifstream file("Databases/Data_on_Each_Student.csv");
    vector<string> lines; string line; bool found = false;
    while (getline(file, line)) {
        if (line.empty()) continue;
        auto cols = split_csv(line);
        if (!cols.empty() && cols[0] == student_id) {
            while (cols.size() <= max(static_cast<size_t>(6), static_cast<size_t>(8))) cols.emplace_back("");
            if (!cols[col_index].empty() && cols[col_index] != "<null>") cols[col_index] += "^" + request_data;
            else cols[col_index] = request_data;
            line = join_csv(cols); found = true;
        }
        lines.emplace_back(line);
    }
    file.close();
    if (found) { ofstream out("Databases/Data_on_Each_Student.csv"); for (const auto& l : lines) out << l << "\n"; }
}

void load_admin_requests(slint::ComponentHandle<Main_App>& ui) {
    vector<WithdrawalRequest> w_reqs;
    vector<ExcuseRequest> e_reqs;
    ifstream file("Databases/Data_on_Each_Student.csv");
    string line; getline(file, line); // Skip header

    while (getline(file, line)) {
        if (line.empty()) continue;
        auto cols = split_csv(line);
        if (cols.size() > 8) {
            if (!cols[6].empty() && cols[6] != "<null>") {
                stringstream ss(cols[6]); string req_str;
                while (getline(ss, req_str, '^')) {
                    stringstream sp(req_str); string course, week, reason, status;
                    getline(sp, course, '|'); getline(sp, week, '|'); getline(sp, reason, '|'); getline(sp, status, '|');
                    if (status == "Pending")
                        e_reqs.push_back({slint::SharedString(cols[0]), slint::SharedString(course), slint::SharedString(week), slint::SharedString(reason)});
                }
            }
            if (!cols[8].empty() && cols[8] != "<null>") {
                stringstream ss(cols[8]); string req_str;
                while (getline(ss, req_str, '^')) {
                    stringstream sp(req_str); string course, reason, status;
                    getline(sp, course, '|'); getline(sp, reason, '|'); getline(sp, status, '|');
                    if (status == "Pending")
                        w_reqs.push_back({slint::SharedString(cols[0]), slint::SharedString(course), slint::SharedString(reason)});
                }
            }
        }
    }
    ui->set_pending_withdrawals(std::make_shared<slint::VectorModel<WithdrawalRequest>>(w_reqs));
    ui->set_pending_excuses(std::make_shared<slint::VectorModel<ExcuseRequest>>(e_reqs));
}

void update_admin_request(const string& target_id, size_t col_index, const string& target_course, bool approved) {
    ifstream file("Databases/Data_on_Each_Student.csv");
    vector<string> lines; string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        auto cols = split_csv(line);
        if (cols.size() > col_index && cols[0] == target_id) {
            stringstream ss(cols[col_index]); string req_str; string updated_col;
            while (getline(ss, req_str, '^')) {
                if (!updated_col.empty()) updated_col += "^";
                if (req_str.find(target_course) != string::npos && req_str.find("Pending") != string::npos) {
                    string new_status = approved ? "Approved" : "Rejected";
                    req_str.replace(req_str.find("Pending"), 7, new_status);
                }
                updated_col += req_str;
            }
            cols[col_index] = updated_col;
            line = join_csv(cols);
        }
        lines.emplace_back(line);
    }
    file.close();
    ofstream out("Databases/Data_on_Each_Student.csv");
    for (const auto& l : lines) out << l << "\n";
}
// ======================================================================

int main() {
    auto ui = Main_App::create();
    Login_Manager &db = Login_Manager::get_instance();
    TeacherGradesManager teacherGradesManager;

    AdminCourseManager adminManager("Databases/");
    TeacherAttendanceManager teacherAttendanceManager;

    string current_id;
    string current_email;

    // ======================================================================
    // FIX: Register on_request_course ONCE here, outside on_check_credentials.
    // Registering it inside the login callback stacks a new handler every
    // login, causing double-fires and stale captures after re-login.
    // ======================================================================
    ui->on_request_course([&](const slint::SharedString& course_code) {
        if (!global_request_manager) return;

        string code_str = string(course_code.data());
        size_t underscore_pos = code_str.find_last_of('_');
        string clean_code = (underscore_pos != string::npos) ? code_str.substr(0, underscore_pos) : code_str;
        clean_code.erase(remove(clean_code.begin(), clean_code.end(), '\r'), clean_code.end());
        clean_code.erase(remove(clean_code.begin(), clean_code.end(), '\n'), clean_code.end());
        size_t endpos = clean_code.find_last_not_of(" \t");
        if (endpos != string::npos) clean_code = clean_code.substr(0, endpos + 1);

        bool success = global_request_manager->request_course(clean_code);
        if (success) {
            ui->set_status_message("Course " + slint::SharedString(clean_code) + " Requested!");
            ui->set_request_submitted(true);

            auto student_requests = global_request_manager->get_student()->requestedCourses;
            vector<CourseInfo> req_vec;
            for (const auto& c : student_requests) {
                CourseInfo info;
                info.course_code = slint::SharedString(c.code);
                info.course_name = slint::SharedString(c.name);
                req_vec.emplace_back(info);
            }
            ui->set_requested_courses(make_shared<slint::VectorModel<CourseInfo>>(req_vec));

            vector<CourseInfo> updated_available_vec;
            auto new_iterator = global_request_manager->get_available_courses_inorder();
            while (new_iterator->has_next()) {
                Course& c = new_iterator->next();
                CourseInfo info;
                info.course_code = slint::SharedString(c.code);
                info.course_name = slint::SharedString(c.name);
                updated_available_vec.emplace_back(info);
            }
            ui->set_available_courses(make_shared<slint::VectorModel<CourseInfo>>(updated_available_vec));
        } else {
            ui->set_status_message("Request Failed (Limit 5 reached or Not found).");
            ui->set_request_submitted(false);
        }
    });

    ui->on_check_credentials([&](const slint::SharedString &id, const slint::SharedString &pass, int role) {
        string string_id = string(id);
        Login_Status status = db.login(string_id, string(pass), static_cast<User_Role>(role));

        if (status == Login_Status::SUCCESS) {
            ui->set_login_error_state(false);
            current_id = string_id;

            if (role == 1) { // --- STUDENT ---
                auto &student = Student_Profile::get_instance();
                student.load_profile(current_id);
                ui->set_user_name(student.get_name().c_str());
                ui->set_user_id(student.get_id().c_str());
                ui->set_user_Faculty(student.get_faculty().c_str());
                ui->set_student_gpa(student.get_gpa().c_str());
                ui->set_user_email(student.get_email().c_str());

                string pfp_path = ImageManager::get_user_pfp_path(current_id);
                ui->set_user_profile_pic(slint::Image::load_from_path(pfp_path.c_str()));

                RegisteredCoursesManager rc_manager(current_id);
                auto courses_vec = rc_manager.get_registered_courses_with_attendance();
                vector<CourseInfo> std_courses;
                for (const auto& course : courses_vec) std_courses.emplace_back(course);
                ui->set_my_courses(make_shared<slint::VectorModel<CourseInfo>>(std_courses));

                global_request_manager = make_unique<Request_Courses>(current_id);
                vector<CourseInfo> available_vec;
                auto iterator = global_request_manager->get_available_courses_inorder();
                while (iterator->has_next()) {
                    Course& c = iterator->next();
                    CourseInfo info;
                    info.course_code = slint::SharedString(c.code);
                    info.course_name = slint::SharedString(c.name);
                    available_vec.emplace_back(info);
                }
                ui->set_available_courses(make_shared<slint::VectorModel<CourseInfo>>(available_vec));

                // Clear any stale requested-courses list from a previous session
                ui->set_requested_courses(make_shared<slint::VectorModel<CourseInfo>>(vector<CourseInfo>{}));
                ui->set_status_message("");
                ui->set_request_submitted(false);

            } else if (role == 2) { // --- TEACHER ---
                auto &teacher = Teacher_Profile::get_instance();
                teacher.load_profile(current_id);
                current_email = teacher.get_email();

                string pfp_path = ImageManager::get_user_pfp_path(current_id);
                ui->set_user_profile_pic(slint::Image::load_from_path(pfp_path.c_str()));
                ui->set_user_name(teacher.get_name().c_str());
                ui->set_user_email(current_email.c_str());

                vector<slint::SharedString> display_list;
                for (const auto &course_str : teacher.get_courses_taught())
                    display_list.emplace_back(slint::SharedString(course_str));
                ui->set_teacher_profile_display_list(make_shared<slint::VectorModel<slint::SharedString>>(display_list));

                teacherAttendanceManager.initUI(ui.operator->(), teacher.get_name());
                teacherGradesManager.initUI(ui.operator->(), teacher.get_name());

            } else if (role == 3) { // --- ADMIN ---
                auto &admin = Admin_Profile::get_instance();
                admin.load_profile(current_id);
                ui->set_user_name(admin.get_name().c_str());
                ui->set_user_email(admin.get_email().c_str());
                ui->set_admin_position(admin.get_position().c_str());

                string pfp_path = ImageManager::get_user_pfp_path(current_id);
                ui->set_user_profile_pic(slint::Image::load_from_path(pfp_path.c_str()));

                adminManager.initUI(ui.operator->());
                load_admin_requests(ui);
            }

            ui->set_active_panel(role);

        } else {
            ui->set_login_error_state(true);
            if (status == Login_Status::PASSWORD_INCORRECT)
                ui->set_login_error_msg("The password you entered is incorrect.");
            else if (status == Login_Status::USER_NOT_FOUND)
                ui->set_login_error_msg("Account not found. Check your ID/Email.");
            else if (status == Login_Status::ROLE_MISMATCH)
                ui->set_login_error_msg("Incorrect portal selected for this account.");
        }
    });

    ui->on_admin_next_student([&]() { adminManager.nextStudent(ui.operator->()); });
    ui->on_admin_previous_student([&]() { adminManager.prevStudent(ui.operator->()); });
    ui->on_admin_submit_decisions([&]() { adminManager.submitDecisions(ui.operator->()); });

    // ==========================================
    // STUDENT ACADEMIC REQUEST CALLBACKS
    // ==========================================
    ui->on_submit_withdrawal_req([&](const slint::SharedString& code, const slint::SharedString& reason) {
        append_student_request(current_id, 8, clean_text(string(code)) + "|" + clean_text(string(reason)) + "|Pending");
    });

    ui->on_submit_excuse_req([&](const slint::SharedString& code, const slint::SharedString& week, const slint::SharedString& reason) {
        append_student_request(current_id, 6, clean_text(string(code)) + "|" + clean_text(string(week)) + "|" + clean_text(string(reason)) + "|Pending");
    });

    // ==========================================
    // ADMIN ACADEMIC REQUEST CALLBACKS
    // ==========================================
    ui->on_admin_handle_withdrawal([&](const slint::SharedString& id, const slint::SharedString& course, bool approved) {
        update_admin_request(string(id), 8, string(course), approved);
        load_admin_requests(ui);
    });

    ui->on_admin_handle_excuse([&](const slint::SharedString& id, const slint::SharedString& course, const slint::SharedString& week, bool approved) {
        update_admin_request(string(id), 6, string(course), approved);
        load_admin_requests(ui);
    });

    ui->on_change_picture([&]() {
        string source = ImageManager::select_image_dialog();
        if (source.empty()) return;
        string new_path = ImageManager::save_profile_picture(source, current_id);
        if (!new_path.empty()) {
            ui->set_user_profile_pic(slint::Image());
            ui->set_user_profile_pic(slint::Image::load_from_path(new_path.c_str()));
        }
    });

    // ======================================================================
    // LOGOUT CALLBACK — fully cleaned up
    // ======================================================================
    ui->on_logout([&]() {
        // Reset all profile singletons
        Student_Profile::get_instance().reset();
        Teacher_Profile::get_instance().reset();
        Admin_Profile::get_instance().reset();

        // Clear session identifiers
        current_id.clear();
        current_email.clear();

        // Reset all UI text fields
        ui->set_user_name("");
        ui->set_user_id("");
        ui->set_user_email("");
        ui->set_user_Faculty("");
        ui->set_student_gpa("");
        ui->set_admin_position("");

        // Clear profile picture
        ui->set_user_profile_pic(slint::Image());

        // Clear course models so stale data never shows on next login
        ui->set_my_courses(make_shared<slint::VectorModel<CourseInfo>>(vector<CourseInfo>{}));
        ui->set_available_courses(make_shared<slint::VectorModel<CourseInfo>>(vector<CourseInfo>{}));
        ui->set_requested_courses(make_shared<slint::VectorModel<CourseInfo>>(vector<CourseInfo>{}));

        // FIX: Clear admin request models so they don't linger for the next user
        ui->set_pending_withdrawals(make_shared<slint::VectorModel<WithdrawalRequest>>(vector<WithdrawalRequest>{}));
        ui->set_pending_excuses(make_shared<slint::VectorModel<ExcuseRequest>>(vector<ExcuseRequest>{}));

        // Clear teacher display list
        ui->set_teacher_profile_display_list(make_shared<slint::VectorModel<slint::SharedString>>(vector<slint::SharedString>{}));

        // Release the request manager
        global_request_manager.reset();

        // Reset status/request state
        ui->set_status_message("");
        ui->set_request_submitted(false);

        // Reset tab positions
        ui->set_student_active_tab(0);
        ui->set_admin_active_tab(0);

        // Clear login error state
        ui->set_login_error_state(false);
        ui->set_login_error_msg("");

        // FIX: Return to login panel FIRST, then sync Logout_value.
        // Reversing this order caused a Slint state flicker in the original code.
        ui->set_active_panel(0);
        ui->set_Logout_value(0);
    });

    ui->run();
    return 0;
}