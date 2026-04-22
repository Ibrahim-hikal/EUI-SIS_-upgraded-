#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "Features_and_Functions/Login_Page/header/Login_Page.h"
#include "Features_and_Functions/Profile_Pages/Student_Profile/header/Student_Profile.h"
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

    // Variables to track current user session state globally
    string current_email = "";
    string current_id = "";

    // For Teacher Attendance Logic
    ui->on_load_students([&current_email, ui](slint::SharedString course, int week) {
        TeacherAttendanceManager manager(current_email);
        auto vec = manager.get_students_for_week(course.data(), week);

        // CONVERSION: Convert SharedVector to std::vector
        std::vector<StudentAttendanceData> std_vec;
        for (int i = 0; i < vec.size(); ++i) {
            std_vec.push_back(vec[i]);
        }

        // Wrap the std::vector into a slint::VectorModel
        auto model = std::make_shared<slint::VectorModel<StudentAttendanceData>>(std_vec);
        ui->set_students_data(model);
    });

    // CONVERSION: Receive std::shared_ptr<slint::Model<T>> from Slint
    ui->on_save_attendance([&current_email](std::shared_ptr<slint::Model<StudentAttendanceData>> data, slint::SharedString course, int week) {
        TeacherAttendanceManager manager(current_email);

        // Extract the rows back into a SharedVector so our C++ manager can use it
        slint::SharedVector<StudentAttendanceData> vec;
        for (int i = 0; i < data->row_count(); ++i) {
            if (auto row = data->row_data(i)) {
                vec.push_back(*row);
            }
        }

        bool success = manager.save_attendance(course.data(), week, vec);
        if(success) {
            cout << "Attendance for " << course << " Week " << week << " saved successfully!" << endl;
        } else {
            cout << "Failed to save attendance." << endl;
        }
    }); //end of teacher attendance

    // Instantiate our encapsulated manager
    AdminCourseManager adminManager("Databases/");

    ui->on_check_credentials([&](const slint::SharedString& id, const slint::SharedString& pass, int role) {
        string string_id = string(id);
        Login_Status status = db.login(string_id, string(pass), static_cast<User_Role>(role));

        if (status == Login_Status::SUCCESS) {
            ui->set_login_error_state(false);
            current_id = string_id; // Store logged-in user ID

            RegisteredCoursesManager rc_manager(current_id);
            auto courses_vec = rc_manager.get_registered_courses_with_attendance();

            // CONVERSION: Convert SharedVector to std::vector
            std::vector<CourseInfo> std_courses;
            for (int i = 0; i < courses_vec.size(); ++i) {
                std_courses.push_back(courses_vec[i]);
            }

            // Wrap the std::vector into a slint::VectorModel
            auto courses_model = std::make_shared<slint::VectorModel<CourseInfo>>(std_courses);
            ui->set_my_courses(courses_model);

            if (role == 1) { // Student logic
                auto& student = Student_Profile::get_instance();
                student.load_profile(current_id);
                current_email = student.get_email(); // Save email for the session

                ui->set_user_name(student.get_name().c_str());
                ui->set_user_id(student.get_id().c_str());
                ui->set_user_Faculty(student.get_faculty().c_str());
                ui->set_student_gpa(student.get_gpa().c_str());
                ui->set_user_email(current_email.c_str());

                string pfp_path = ImageManager::get_user_pfp_path(current_id);
                auto img = slint::Image::load_from_path(pfp_path.c_str());
                ui->set_user_profile_pic(img);
            }
            else if (role == 2) { // Teacher Logic
                // We also need to extract current_email for the Teacher logic to work
                auto& teacher_data = Student_Profile::get_instance(); // Adjust if you have a Teacher_Profile singleton instead
                teacher_data.load_profile(current_id);
                current_email = teacher_data.get_email();
            }
            else if (role == 3) { // Admin Logic
                // Initialize the manager with the UI pointer
                adminManager.initUI(ui.operator->());
            }

            cout << "Success! Setting active_panel to: " << role << endl;
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

    // Hook up Admin Component UI Callbacks to the Manager object
    ui->on_admin_next_student([&]() { adminManager.nextStudent(ui.operator->()); });
    ui->on_admin_previous_student([&]() { adminManager.prevStudent(ui.operator->()); });
    ui->on_admin_submit_decisions([&]() { adminManager.submitDecisions(ui.operator->()); });

    ui->on_change_picture([&]() {
        string source = ImageManager::select_image_dialog();
        if (source.empty()) return;
        auto& student = Student_Profile::get_instance();
        string new_path = ImageManager::save_profile_picture(source, student.get_id());

        if (!new_path.empty()) {
            ui->set_user_profile_pic(slint::Image());
            auto fresh_img = slint::Image::load_from_path(new_path.c_str());
            ui->set_user_profile_pic(fresh_img);
        }
    });

    ui->on_logout([&]() {
        Student_Profile::get_instance().reset();
        current_email = "";
        current_id = "";
        ui->set_user_name("");
        ui->set_user_id("");
        ui->set_user_profile_pic(slint::Image());
        ui->set_active_panel(0);
        ui->set_login_error_state(false);
        cout << "User logged out successfully." << endl;
    });

    ui->run();
    return 0;
}