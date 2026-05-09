#ifndef STUDENT_PROFILE_H
#define STUDENT_PROFILE_H

#include <string>
#include <vector>
#include "../../IProfile.h" // Links the Abstract Base Class

// ---> 6. INHERITANCE ("Student is a Profile") <---
class Student_Profile : public IProfile {
private:
    // Core student profile fields loaded from persistence.
    std::string id;
    std::string faculty;
    std::string gpa;
    std::vector<std::string> registered_courses;

    // Singleton-only construction.
    Student_Profile() {}

public:
    // ---> 7. DESIGN PATTERN 1: SINGLETON <---
    static Student_Profile& get_instance() {
        static Student_Profile instance;
        return instance;
    }

    Student_Profile(const Student_Profile&) = delete;
    Student_Profile& operator=(const Student_Profile&) = delete;

    void load_profile(const std::string& current_email) override;
    void reset() override;
    std::string get_role() const override { return "Student"; }

    std::string get_id() const { return id; }
    std::string get_faculty() const { return faculty; }
    std::string get_gpa() const { return gpa; }
    std::vector<std::string> get_registered_courses() const { return registered_courses; }
};
#endif