#include "Student.h"
#include <fstream>
#include <sstream>
#include <iostream>

    using namespace std;
// ==========================================
// STATIC DATA STORAGE
// ==========================================
vector<Course>& Student::get_mutable_courses() {
    static vector<Course> eligible;
    return eligible;
}

// ==========================================
// CSV PARSING & LOADING
// ==========================================
void Student::load_offered_courses() {
    auto& courses = get_mutable_courses();

    // Guard Clause: If we already loaded the data, stop here.
    if (!courses.empty()) return;

    ifstream file("Databases/Offered_Courses.csv");
    if (!file.is_open()) {
        cerr << "CRITICAL: Could not open Offered_Courses.csv" << endl;
        return;
    }

    string line, cell;
    getline(file, line); // Skip CSV header

    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        vector<string> cols;
        while (getline(ss, cell, ',')) {
            cols.push_back(cell);
        }

        // Ensure we have enough columns to avoid out-of-bounds crashes
        if (cols.size() >= 4) {
            Course c;
            c.code = cols[0];
            c.preReq1 = cols[1];
            c.preReq2 = cols[2];
            c.name = cols[3];
            courses.push_back(c);
        }
    }
    file.close();
}

// ==========================================
// COURSE FILTERING LOGIC
// ==========================================

// Helper 1: Check if the student passed it or is taking it now
bool Student::is_already_taken(const Course& c) const {
    bool passed = (passedCourses.find(c.code) != string::npos);
    bool taking_now = (registeredCourses.find(c.code) != string::npos);
    return passed || taking_now;
}

// Helper 2: Check if they failed it in the past
bool Student::is_previously_failed(const Course& c) const {
    return failedCourses.find(c.code) != string::npos;
}

// Helper 3: Check if both prerequisites are met (or don't exist)
bool Student::meets_prerequisites(const Course& c) const {
    auto is_met = [this](const string& pre) {
        return pre == "-1" || passedCourses.find(pre) != string::npos;
    };
    return is_met(c.preReq1) && is_met(c.preReq2);
}

// The main filter: Notice how easy this is to read now!
vector<Course> Student::get_eligible_courses() const {
    vector<Course> eligible;

    for (const auto& c : get_mutable_courses()) {
        // 1. If they already took it and passed, skip it.
        if (is_already_taken(c)) {
            continue;
        }

        // 2. Allow if they meet prerequisites OR if they are retaking a failed class
        if (meets_prerequisites(c) || is_previously_failed(c)) {
            eligible.push_back(c);
        }
    }

    return eligible;
}

// ==========================================
// REGISTRATION
// ==========================================
bool Student::register_course(const string& course_code) {
    // Check against the student's specific eligible list
    for (const auto& c : get_eligible_courses()) {
        if (c.code == course_code) {
            requestedCourses.push_back(c);
            return true;
        }
    }
    return false; // Course not found or not eligible
}

void Student::load_data() {
    std::ifstream file("Databases/Data_on_Each_Student.csv");
    if (!file.is_open()) {
        std::cerr << "CRITICAL: Could not open Data_on_Each_Student.csv" << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line); // Skip header

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // 1. Safe CSV Parser (ignores commas inside quotes)
        std::vector<std::string> cols;
        std::string current;
        bool in_quotes = false;

        for (char c : line) {
            if (c == '"') {
                in_quotes = !in_quotes;
            } else if (c == ',' && !in_quotes) {
                cols.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        }
        cols.push_back(current);

        // 2. Find this specific student and load their history
        if (!cols.empty() && cols[0] == this->id) {

            // NOTE: Double check these column numbers match your CSV layout!
            // Based on your Registered_Courses file, Registered is column 4.
            if (cols.size() > 2) this->passedCourses = cols[2];
            if (cols.size() > 3) this->failedCourses = cols[3];
            if (cols.size() > 4) this->registeredCourses = cols[4];

            std::cout << "SUCCESS: Loaded data for student " << this->id << std::endl;
            break; // Stop searching once we find them
        }
    }
    file.close();
}