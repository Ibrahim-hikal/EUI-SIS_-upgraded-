#pragma once    //to fix multiple definition error

#include <string>
#include <vector>

using namespace std;

class Teacher_Profile {
private:
    string name;
    string email; // Acts as the unique ID
    vector<string> courses_taught;

    // Private constructor for Singleton
    Teacher_Profile() {}

    // Helper functions for CSV parsing
    string trim(const string& str);
    vector<string> split(const string& str, char delimiter);

public:
    // Delete copy constructor and assignment operator to enforce Singleton
    Teacher_Profile(const Teacher_Profile&) = delete;
    void operator=(const Teacher_Profile&) = delete;

    // Get the single instance
    static Teacher_Profile& get_instance() {
        static Teacher_Profile instance;
        return instance;
    }

    // Load data from the CSV based on the teacher's email
    void load_profile(const string& current_email);

    // Reset the profile (used during logout)
    void reset();

    // Getters
    string get_name() const { return name; }
    string get_email() const { return email; }
    vector<string> get_courses_taught() const { return courses_taught; }
};