#ifndef LOGIN_PAGE_H
#define LOGIN_PAGE_H

#include <string>

// Enum for handling various login outcomes
enum class Login_Status {
    SUCCESS = 0,
    PASSWORD_INCORRECT = 1,
    ROLE_MISMATCH = 2,
    USER_NOT_FOUND = 3
};

// Enum for User Roles
enum class User_Role {
    Admin = 3,
    Teacher = 2,
    Student = 1
};

class Login_Manager {
private:
    // Private constructor for Singleton pattern
    Login_Manager();

public:
    // Static method to get the single instance of the class
    static Login_Manager& get_instance();

    // Prevent copying or assignment of the Singleton
    Login_Manager(const Login_Manager&) = delete;
    void operator=(const Login_Manager&) = delete;

    Login_Status login(const std::string& input_field, const std::string& password, User_Role expected_role);
};

#endif // LOGIN_PAGE_H