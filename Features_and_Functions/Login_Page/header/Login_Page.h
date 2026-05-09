/*
* File: Login_Page.h
 * Description: Sets up the rules for the login system. It tracks the different
 * types of users and the different responses the system can give when someone
 * tries to log in.
 */

#ifndef LOGIN_PAGE_H
#define LOGIN_PAGE_H

#include <string>

/*
 * Represents the exact reason a login attempt succeeded or failed.
 * This helps the visual interface display the correct error message to the user.
 */
enum class Login_Status {
    SUCCESS,
    PASSWORD_INCORRECT,
    USER_NOT_FOUND,
    ROLE_MISMATCH
};

/*
 * Represents the three types of accounts available in the system.
 * The numbers match the active screen IDs in the main visual interface.
 */
enum class User_Role {
    STUDENT = 1,
    TEACHER = 2,
    ADMIN = 3
};

/*
 * A secure manager that checks the database to verify emails and passwords.
 * It is designed so that the application only uses one single manager at a time
 * to save memory and prevent data clashes.
 */
class Login_Manager {
private:
    // A hidden setup function ensures the system cannot accidentally create duplicate managers
    Login_Manager() {}

public:
    // Returns the single active login manager for the application to use
    static Login_Manager& get_instance() {
        static Login_Manager instance;
        return instance;
    }

    // Prevents the system from making copies of the manager in the background
    Login_Manager(const Login_Manager&) = delete;
    Login_Manager& operator=(const Login_Manager&) = delete;

    /*
     * Takes the email, password, and the portal the user selected (like Student).
     * Scans the database and returns a specific status detailing the result.
     */
    Login_Status login(const std::string& email, const std::string& password, User_Role role);
};

#endif