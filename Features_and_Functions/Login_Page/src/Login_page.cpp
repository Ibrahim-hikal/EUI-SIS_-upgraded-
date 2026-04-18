#include "../header/Login_Page.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

// Constructor implementation
Login_Manager::Login_Manager() {}

// Singleton implementation
Login_Manager& Login_Manager::get_instance() {
    static Login_Manager instance;
    return instance;
}

Login_Status Login_Manager::login(const string& input_field, const string& password, User_Role expected_role) {
    ifstream file("Databases/Login_Credentials.csv");

    if (!file.is_open()) {
        cerr << "Error: Could not open Login_Credentials.csv" << endl;
        return Login_Status::USER_NOT_FOUND;
    }

    string line;
    getline(file, line); // Skip Header

    while (getline(file, line)) {
        if (line.empty()) continue;

        istringstream iss(line);
        string email, pass, role_str, name, id;

        getline(iss, email, ',');
        getline(iss, pass, ',');
        getline(iss, role_str, ',');
        getline(iss, name, ',');
        getline(iss, id, ',');

        User_Role stored_role;
        if (role_str == "ADMIN") stored_role = User_Role::Admin;
        else if (role_str == "TEACHER") stored_role = User_Role::Teacher;
        else if (role_str == "STUDENT") stored_role = User_Role::Student;
        else continue;

        // 1. Find the user by EITHER Email or ID
        if (input_field == email || input_field == id) {

            // 2. Validate Password
            if (pass != password) {
                return Login_Status::PASSWORD_INCORRECT;
            }

            // 3. Students use ID, Staff use Email
            if (stored_role == User_Role::Student && input_field != id) {
                // found the student account via email, but that's not allowed for login
                return Login_Status::ROLE_MISMATCH;
            }
            if ((stored_role == User_Role::Teacher || stored_role == User_Role::Admin) && input_field != email) {
                // found the staff account via ID, but that's not allowed for login
                return Login_Status::ROLE_MISMATCH;
            }

            // 4. Did you click the right button in the gui?
            if (stored_role != expected_role) {
                return Login_Status::ROLE_MISMATCH;
            }

            return Login_Status::SUCCESS;
        }
    }

    return Login_Status::USER_NOT_FOUND;
}