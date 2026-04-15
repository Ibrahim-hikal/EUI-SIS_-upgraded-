#include <iostream>
#include <string>
#include "Features_and_Functions/Login_Page/header/Login_Page.h"
#include "Login_page.h" // Matches the lowercase 'p' from your build log

using namespace std;

int main() {
    auto ui = Login_Page::create(); // Verify this matches your 'export component' name
    Login_Manager& auth_system = Login_Manager::get_instance();

    // Ensure 'login_attempted' matches your .slint callback name exactly
    ui->on_login_attempted([&](slint::SharedString id_email, slint::SharedString password, int role_index) {

        User_Role selected_role = static_cast<User_Role>(role_index);

        // Explicitly convert Slint strings to std::string
        Login_Status status = auth_system.login(
            std::string(id_email),
            std::string(password),
            selected_role
        );

        if (status == Login_Status::SUCCESS) {
        ui->set_Error_State(false);
        std::cout << "Login Success. Navigating..." << std::endl;
        // logic to switch windows would go here
    } else {
        ui->set_Error_State(true);
        // Map the backend status to your Slint strings
        if (status == Login_Status::PASSWORD_INCORRECT) {
            ui->set_Error_Message("The password you entered is incorrect.");
        } else if (status == Login_Status::USER_NOT_FOUND) {
            ui->set_Error_Message("Account not found. Check your ID/Email.");
        } else if (status == Login_Status::ROLE_MISMATCH) {
            ui->set_Error_Message("Incorrect portal selected for this account.");
        }
    }
    });

    ui->run();
    return 0;
}