#ifndef STUDENT_PROFILE_H
#define STUDENT_PROFILE_H

#include <string>
#include <vector>

using namespace std;

// THE ABSOLUTE PATH FIX (Use forward slashes!)
#define STUDENT_DB "Databases/Data_on_Each_Student.csv"

class Student_Profile {
private:
    string id;
    string name;
    string faculty;
    string gpa;

    Student_Profile() = default;

public:
    static Student_Profile& get_instance();

    // Disable copy/assignment for Singleton
    Student_Profile(const Student_Profile&) = delete;
    void operator=(const Student_Profile&) = delete;

    void load_profile(const string& target_id);

    // Getters
    string get_id() const { return id; }
    string get_name() const { return name; }
    string get_faculty() const { return faculty; }
    string get_gpa() const { return gpa; }

    // Dynamic Email Logic
    string get_email() const {
        if (id.empty() || id == "Loading...") return "Loading...";
        return id + "@students.eui.edu.eg";
    }
};

#endif