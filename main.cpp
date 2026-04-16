#include <iostream>
#include <string>
#include "Features_and_Functions/Login_Page/header/Login_Page.h"
#include "main.h"



using namespace std;

int main() {
    auto ui = Main_App::create();
    Login_Manager& db = Login_Manager::get_instance();

    ui->on_check_credentials([&](slint::SharedString id, slint::SharedString pass, int role) {

            Login_Status status = db.login(
                std::string(id),
                std::string(pass),
                static_cast<User_Role>(role)
            );

            if (status == Login_Status::SUCCESS) {
                // This is the "Magic Switch"
                // It stays in the same window but swaps the Rectangles
                ui->set_active_panel(1);
                ui->set_login_error_state(false);
                std::cout << "Switched to Student Profile" << std::endl;
            } else {
                ui->set_login_error_state(true);
        // Map the backend status to your Slint strings
        if (status == Login_Status::PASSWORD_INCORRECT) {
            ui->set_login_error_msg("The password you entered is incorrect.");
        } else if (status == Login_Status::USER_NOT_FOUND) {
            ui->set_login_error_msg("Account not found. Check your ID/Email.");
        } else if (status == Login_Status::ROLE_MISMATCH) {
            ui->set_login_error_msg("Incorrect portal selected for this account.");
        }
    }
    });

    ui->run();
    return 0;
}