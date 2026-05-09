#ifndef TEACHER_PROFILE_H
#define TEACHER_PROFILE_H

#include <string>
#include <vector>
#include "../../IProfile.h" // Links the Abstract Base Class

// ---> 6. INHERITANCE ("Teacher is a Profile") <---
class Teacher_Profile : public IProfile {
private:
    // Teaching assignments shown in the teacher dashboard.
    std::vector<std::string> courses_taught;

    // Singleton-only construction.
    Teacher_Profile() {}

public:
    // ---> 7. DESIGN PATTERN 1: SINGLETON <---
    static Teacher_Profile& get_instance() {
        static Teacher_Profile instance;
        return instance;
    }

    Teacher_Profile(const Teacher_Profile&) = delete;
    Teacher_Profile& operator=(const Teacher_Profile&) = delete;

    void load_profile(const std::string& target_email) override;
    void reset() override;
    std::string get_role() const override { return "Teacher"; }

    std::vector<std::string> get_courses_taught() const { return courses_taught; }
};
#endif