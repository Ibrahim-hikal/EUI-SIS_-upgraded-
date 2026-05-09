#ifndef ADMIN_PROFILE_H
#define ADMIN_PROFILE_H

#include <string>
#include "../../IProfile.h" // Links the Abstract Base Class

// ---> 6. INHERITANCE ("Admin is a Profile") <---
class Admin_Profile : public IProfile {
private:
    std::string position;
    Admin_Profile() {}
public:
    // ---> 7. DESIGN PATTERN 1: SINGLETON <---
    static Admin_Profile& get_instance() { static Admin_Profile instance; return instance; }
    Admin_Profile(const Admin_Profile&) = delete;
    Admin_Profile& operator=(const Admin_Profile&) = delete;

    // ---> RUNTIME POLYMORPHISM (Overriding) <---
    void load_profile(const std::string& current_email) override;
    void reset() override;
    std::string get_role() const override { return "Admin"; }

    std::string end_semester();
    std::string get_position() const { return position; }
};
#endif