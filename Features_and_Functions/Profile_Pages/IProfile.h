/*
 * FILE: IProfile.h
 * DESCRIPTION: Abstract base class (interface) for all user types in the system
 *
 * PURPOSE: Defines the shared blueprint/contract for all user profiles (Student, Teacher, Admin)
 *          Prevents code duplication by centralizing common user properties and methods
 *
 * DESIGN PATTERN: Abstract Base Class (Interface)
 *                 All users MUST inherit from IProfile and implement required methods
 *
 * USAGE: Never instantiate IProfile directly. Instead:
 *        - Create a Student object that extends IProfile
 *        - Create a Teacher object that extends IProfile
 *        - Create an Admin object that extends IProfile
 */

#ifndef IPROFILE_H
#define IPROFILE_H

// Standard library includes for string operations and output
#include <string>       // For std::string - stores user name and email
#include <iostream>     // For std::ostream - enables console output


// ==========================================
// INTERFACE: IPROFILE (Abstract Base Class)
// ==========================================
/*
 * This is an interface/abstract class that all user types must follow.
 * Think of it as a "contract" that says:
 * "If you inherit from IProfile, you MUST implement these methods in your own way"
 */
class IProfile {
protected:
    // ==========================================
    // PROTECTED SHARED PROPERTIES
    // ==========================================
    // These properties are shared by ALL user types and are protected
    // (cannot be accessed directly from outside, but child classes can use them)

    std::string name;    // User's full name (set during profile load)
    std::string email;   // User's email address (unique identifier)

public:
    // ==========================================
    // VIRTUAL DESTRUCTOR
    // ==========================================
    // A virtual destructor ensures safe cleanup when the app closes
    // It allows derived classes to perform their own cleanup operations
    virtual ~IProfile() = default;

    // ==========================================
    // PURE VIRTUAL METHODS (Must be implemented by child classes)
    // ==========================================
    // The "= 0" syntax means these are ABSTRACT - child classes MUST override them
    // If a child class doesn't implement these, the code won't compile

    /*
     * Load the user's profile data from the database
     * Each user type loads data differently (students from one table, teachers from another, etc.)
     * Parameters:
     *   - target_email: The email address of the user to load
     */
    virtual void load_profile(const std::string& target_email) = 0;

    /*
     * Reset all user data to empty/default values
     * Used when logging out or switching users
     */
    virtual void reset() = 0;

    /*
     * Get the user's role/type as a string
     * Returns: "Student", "Teacher", or "Admin"
     */
    virtual std::string get_role() const = 0;

    // ==========================================
    // CONCRETE METHODS (Same implementation for all child classes)
    // ==========================================

    /*
     * Helper functions to set the user's basic credentials
     * Overloaded to accept either just email, or both email and name
     * Enables:
     *   profile.set_credentials("john@eui.edu.eg");
     *   profile.set_credentials("john@eui.edu.eg", "John Doe");
     */
    void set_credentials(const std::string& e) { email = e; }
    void set_credentials(const std::string& e, const std::string& n) { email = e; name = n; }

    // ==========================================
    // GETTER METHODS
    // ==========================================
    // Simple functions to safely read (but not modify) the user's info
    // These are const methods - they promise not to change any data

    std::string get_name() const { return name; }   // Returns the user's name
    std::string get_email() const { return email; } // Returns the user's email

    // ==========================================
    // OPERATOR OVERLOADING
    // ==========================================

    /*
     * Overload the == operator for user comparison
     * Allows: if (user1 == user2) { ... }
     * Implementation: Two users are equal if their emails match (unique identifier)
     */
    bool operator==(const IProfile& other) const {
        return this->email == other.email;
    }

    /*
     * Overload the << operator for easy console printing
     * Allows: cout << profile;  // Automatically formats nicely
     * Output format: [Student] John Doe (john@eui.edu.eg)
     *
     * The 'friend' keyword allows this external function to access private data
     */
    friend std::ostream& operator<<(std::ostream& os, const IProfile& profile) {
        os << "[" << profile.get_role() << "] " << profile.name << " (" << profile.email << ")";
        return os;
    }
};

#endif // IPROFILE_H
