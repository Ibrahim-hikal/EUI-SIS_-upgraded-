/*
 * File: IProfile.h
 * Description: This is the shared blueprint for all users in the system.
 * Instead of writing "name" and "email" three different times for Students,
 * Teachers, and Admins, they all share this file to get those variables automatically.
 */

#ifndef IPROFILE_H
#define IPROFILE_H

#include <string>
#include <iostream>

class IProfile {
protected:
    // Shared variables that all users will have
    std::string name;
    std::string email;

public:
    // A virtual destructor ensures safe cleanup when the app closes
    virtual ~IProfile() = default;

    /* * Required Actions
     * Any user (Student, Teacher, Admin) MUST have these three functions.
     * The "= 0" means they are forced to write their own custom version of it.
     */
    virtual void load_profile(const std::string& target_email) = 0;
    virtual void reset() = 0;
    virtual std::string get_role() const = 0;

    /*
     * Helper functions to set the user's basic info.
     * You can either set just the email, or both the email and the name.
     */
    void set_credentials(const std::string& e) { email = e; }
    void set_credentials(const std::string& e, const std::string& n) { email = e; name = n; }

    // Simple functions to safely read the user's info
    std::string get_name() const { return name; }
    std::string get_email() const { return email; }

    /*
     * Custom Rules for comparing users
     * If you ask the code "Is User A == User B?", this tells the code to
     * check if their emails match.
     */
    bool operator==(const IProfile& other) const {
        return this->email == other.email;
    }

    /*
     * Custom Rules for printing
     * This lets us easily print a user to the console like this: cout << profile;
     * It will automatically format it as: [Student] John Doe (john@eui.edu.eg)
     */
    friend std::ostream& operator<<(std::ostream& os, const IProfile& profile) {
        os << "[" << profile.get_role() << "] " << profile.name << " (" << profile.email << ")";
        return os;
    }
};

#endif