/*
* FILE: Login_Page.h
* DESCRIPTION: Login system management and authentication framework
*
* PURPOSE: Defines the authentication system including:
*          - Enum values for login status responses (why a login succeeded or failed)
*          - Enum values for user role identification
*          - Singleton Login Manager that validates credentials against the database
*
* DESIGN PATTERN: Singleton - ensures only ONE login manager exists throughout the app lifetime
*                 This prevents duplicate database access and data conflicts
*
* DEPENDENCIES: Reads from Databases/Login_Credentials.csv for authentication
*/

#ifndef LOGIN_PAGE_H
#define LOGIN_PAGE_H

#include <string>

// ==========================================
// LOGIN STATUS ENUMERATION
// ==========================================
/*
 * Enum: Login_Status
 * PURPOSE: Represents the exact result of a login attempt
 *
 * Used by the C++ backend to communicate WHY a login attempt failed to the GUI
 * This enables the GUI to display the appropriate error message to the user
 *
 * VALUES:
 *   - SUCCESS: Login credentials are valid, user authenticated
 *   - PASSWORD_INCORRECT: Email/ID found but password doesn't match
 *   - USER_NOT_FOUND: Email/ID not found in the database at all
 *   - ROLE_MISMATCH: Credentials valid but user clicked wrong portal (e.g., student tried admin)
 */
enum class Login_Status {
    SUCCESS,              // User successfully authenticated
    PASSWORD_INCORRECT,   // Email/ID found but password is wrong
    USER_NOT_FOUND,       // Email/ID not found in database
    ROLE_MISMATCH         // Valid credentials but wrong user type portal
};

// ==========================================
// USER ROLE ENUMERATION
// ==========================================
/*
 * Enum: User_Role
 * PURPOSE: Represents the three types of user accounts in the system
 *
 * These values correspond to the panel IDs in the main GUI:
 * (0=Login Portal, 1=Student, 2=Teacher, 3=Admin)
 *
 * IMPORTANT: The integer values MUST match the active_panel values in main.slint
 *            This is how the backend communicates which dashboard to show
 *
 * VALUES:
 *   - STUDENT (1): Regular student accessing course enrollment, grades, schedule
 *   - TEACHER (2): Faculty member managing attendance, grades, and course schedule
 *   - ADMIN (3): Administrator managing users, course offerings, and academic processes
 */
enum class User_Role {
    STUDENT = 1,    // Student portal ID
    TEACHER = 2,    // Teacher portal ID
    ADMIN = 3       // Administrator portal ID
};

// ==========================================
// LOGIN MANAGER CLASS (Singleton Pattern)
// ==========================================
/*
 * CLASS: Login_Manager
 * PURPOSE: Single, centralized manager for all user authentication
 *
 * DESIGN PATTERN: Singleton
 *   - Only ONE instance of this class exists for the entire application lifetime
 *   - Prevents multiple competing login processes
 *   - Saves memory by avoiding duplicate authentication objects
 *   - Accessed via: Login_Manager::get_instance()
 *
 * DATABASE: Reads from "Databases/Login_Credentials.csv"
 *           CSV format: email, password, role, ..., id
 *
 * USAGE:
 *   Login_Status result = Login_Manager::get_instance().login(email, password, role);
 *   if (result == Login_Status::SUCCESS) {  authenticate user  }
 */
class Login_Manager {
private:
    // ==========================================
    // PRIVATE CONSTRUCTOR (Singleton pattern)
    // ==========================================
    // A hidden setup function prevents accidental creation of new instances
    // Users cannot call: Login_Manager manager;  (will not compile)
    // Users must use: Login_Manager::get_instance();  (the only allowed way)
    Login_Manager() {}

public:
    // ==========================================
    // SINGLETON ACCESS METHOD
    // ==========================================

    /*
     * Returns the single active login manager instance for the entire application
     * Static method ensures the same instance is returned every time it's called
     *
     * USAGE:
     *   Login_Manager& manager = Login_Manager::get_instance();
     *   manager.login(email, pass, role);
     */
    static Login_Manager& get_instance() {
        static Login_Manager instance;  // Created once, reused forever
        return instance;
    }

    // ==========================================
    // PREVENTED OPERATIONS (for Singleton safety)
    // ==========================================

    // Prevent accidental copying of the singleton manager
    // These deleted methods ensure no duplicate managers can be created
    Login_Manager(const Login_Manager&) = delete;             // Copy constructor blocked
    Login_Manager& operator=(const Login_Manager&) = delete;  // Assignment operator blocked

    // ==========================================
    // AUTHENTICATION METHOD
    // ==========================================

    /*
     * Attempts to authenticate a user with the provided credentials
     *
     * PROCESS:
     *   1. Opens the login database CSV file
     *   2. Reads each row and extracts email, password, role, and ID
     *   3. Cleans/normalizes text for comparison (removes spaces, converts to lowercase)
     *   4. Checks if email OR ID matches the input
     *   5. Validates password matches (case-sensitive for security)
     *   6. Verifies user selected the correct portal (role matches)
     *   7. Returns appropriate status code
     *
     * PARAMETERS:
     *   - email: User's email or ID (tries both from database)
     *   - password: User's password (must match exactly - case sensitive)
     *   - role: The portal type user is trying to access (STUDENT, TEACHER, or ADMIN)
     *
     * RETURNS: Login_Status indicating success or the specific reason for failure
     *          - SUCCESS: All credentials valid
     *          - USER_NOT_FOUND: Email/ID not in database
     *          - PASSWORD_INCORRECT: Email found but password wrong
     *          - ROLE_MISMATCH: Credentials valid but trying wrong portal
     *
     * SECURITY NOTES:
     *   - Passwords are NOT case-insensitive (uppercase/lowercase matters)
     *   - Email/ID comparisons are case-insensitive for user convenience
     *   - Database file must be readable; missing file returns USER_NOT_FOUND
     *
     * USAGE:
     *   Login_Status result = Login_Manager::get_instance()
     *                         .login("john@eui.edu.eg", "MyPassword123", User_Role::STUDENT);
     *   if (result == Login_Status::SUCCESS) { proceed to dashboard }
     */
    Login_Status login(const std::string& email, const std::string& password, User_Role role);
};

#endif // LOGIN_PAGE_H
