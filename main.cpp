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

        switch (status) {
            case Login_Status::SUCCESS:
                cout << "Login Successful! Role ID: " << role_index << endl;
                break;
            case Login_Status::PASSWORD_INCORRECT:
                cerr << "Login Failed: Incorrect Password." << endl;
                break;
            case Login_Status::ROLE_MISMATCH:
                cerr << "Login Failed: Role Mismatch." << endl;
                break;
            case Login_Status::USER_NOT_FOUND:
                cerr << "Login Failed: User Not Found." << endl;
                break;
        }
    });

    ui->run();
    return 0;
}