#include <iostream>
#include <string>
#include <filesystem>
#include "Features_and_Functions/Login_Page/header/Login_Page.h"
#include "Features_and_Functions/Profile_Pages/Student_Profile/header/Student_Profile.h"
#include "Features_and_Functions/Profile_Pages/ImageManager.h"
#include "Features_and_Functions/Course_Management/Admin_Course_Management/Accept_and_Refuse_Courses/header/Accept_and_Refuse_Courses.h"
#include "main.h"

using namespace std;

int main() {
    auto ui = Main_App::create();
    Login_Manager& db = Login_Manager::get_instance();

    // Instantiate our encapsulated manager
    AdminCourseManager adminManager("Databases/");

    ui->on_check_credentials([&](const slint::SharedString& id, const slint::SharedString& pass, int role) {
        string string_id = string(id);
        Login_Status status = db.login(string_id, string(pass), static_cast<User_Role>(role));

        if (status == Login_Status::SUCCESS) {
            ui->set_login_error_state(false);

            if (role == 1) { // Student logic
                auto& student = Student_Profile::get_instance();
                student.load_profile(string_id);
                ui->set_user_name(student.get_name().c_str());
                ui->set_user_id(student.get_id().c_str());
                ui->set_user_Faculty(student.get_faculty().c_str());
                ui->set_student_gpa(student.get_gpa().c_str());
                ui->set_user_email(student.get_email().c_str());

                string pfp_path = ImageManager::get_user_pfp_path(string_id);
                auto img = slint::Image::load_from_path(pfp_path.c_str());
                ui->set_user_profile_pic(img);
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