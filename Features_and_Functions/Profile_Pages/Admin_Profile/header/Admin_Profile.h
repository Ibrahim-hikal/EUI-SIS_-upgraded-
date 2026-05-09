/*
 * FILE: Admin_Profile.h
 * DESCRIPTION: Header file for administrator profile management
 *
 * PURPOSE: Represents an administrator user account in the system
 *          Inherits common profile fields from IProfile (name, email)
 *          Adds admin-specific functionality like semester management
 *
 * DESIGN PATTERNS USED:
 *   1. Inheritance: Extends IProfile abstract base class
 *   2. Singleton: Only one admin instance at a time
 *   3. Polymorphism: Overrides virtual methods from IProfile
 *
 * RESPONSIBILITIES:
 *   - Load admin profile data from database
 *   - Handle semester-end operations
 *   - Track admin position/role (e.g., "Dean", "Registrar")
 *
 * DATABASE: Databases/Data_on_each_admin.csv
 */

#ifndef ADMIN_PROFILE_H
#define ADMIN_PROFILE_H

// Standard library includes
#include <string>                  // For std::string - admin data storage
#include "../../IProfile.h"        // Links the Abstract Base Class for inheritance

// ==========================================
// ADMIN PROFILE CLASS
// ==========================================
/*
 * CLASS: Admin_Profile
 * PURPOSE: Represents a single administrator profile
 *
 * INHERITANCE: Extends IProfile
 *   - Inherits: name, email (protected members)
 *   - Inherits: set_credentials(), get_name(), get_email()
 *   - Must override: load_profile(), reset(), get_role()
 *
 * DESIGN PATTERN: Singleton
 *   - Only ONE admin can be loaded at a time
 *   - Prevents memory waste and data conflicts
 *   - Access via: Admin_Profile::get_instance()
 *
 * USAGE:
 *   Admin_Profile& admin = Admin_Profile::get_instance();
 *   admin.load_profile("admin@eui.edu.eg");
 *   cout << admin.get_position();  // e.g., "Registrar"
 */
class Admin_Profile : public IProfile {
private:
    // ==========================================
    // PRIVATE MEMBER VARIABLES
    // ==========================================
    std::string position;  // Admin's position/role (e.g., "Dean", "Registrar")

    // ==========================================
    // PRIVATE CONSTRUCTOR (Singleton Pattern)
    // ==========================================
    // Hidden constructor prevents: Admin_Profile admin;  (compilation error)
    // Ensures use of: Admin_Profile::get_instance();     (only valid way)
    Admin_Profile() {}

public:
    // ==========================================
    // SINGLETON ACCESSOR
    // ==========================================
    // Returns the single admin instance for the entire application
    // Static method ensures same instance returned every time
    static Admin_Profile& get_instance() {
        static Admin_Profile instance;  // Created once, reused forever
        return instance;
    }

    // ==========================================
    // SINGLETON SAFETY (Prevent Copies)
    // ==========================================
    // These deleted methods prevent accidental duplication
    Admin_Profile(const Admin_Profile&) = delete;             // No copy constructor
    Admin_Profile& operator=(const Admin_Profile&) = delete;  // No assignment operator

    // ==========================================
    // VIRTUAL METHOD IMPLEMENTATIONS (from IProfile)
    // ==========================================

    /*
     * Load admin profile data from the database
     * Reads from Databases/Data_on_each_admin.csv
     *
     * Parameters:
     *   - current_email: Email address to search for in database
     *
     * Populates: name, email (inherited), position (admin-specific)
     */
    void load_profile(const std::string& current_email) override;

    /*
     * Reset all admin data to empty/default values
     * Called when logging out
     */
    void reset() override;

    /*
     * Returns the user role type as a string
     * Implementation: Always returns "Admin"
     */
    std::string get_role() const override { return "Admin"; }

    // ==========================================
    // ADMIN-SPECIFIC METHODS
    // ==========================================

    /*
     * Handles end-of-semester operations
     * Processes student grades, calculates GPAs, finalizes enrollments
     *
     * RETURNS: Status message describing what was done
     */
    std::string end_semester();

    /*
     * Getter method for admin's position
     *
     * RETURNS: The admin's job title/position
     */
    std::string get_position() const { return position; }
};

#endif // ADMIN_PROFILE_H
