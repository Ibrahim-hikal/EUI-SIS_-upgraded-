#include "../header/Teacher_Profile.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;


// Helper to clean up strings
string trim_teacher(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void Teacher_Profile::load_profile(const string& target_email) {
    // Reset data
    this->email = target_email;
    this->name = "Unknown Instructor";
    this->courses_taught.clear();

    // 1. Get the Teacher's real name from Login_Credentials.csv
    ifstream login_file("Databases/Login_Credentials.csv");
    string line, cell;
    if (login_file.is_open()) {
        getline(login_file, line); // Skip header
        while (getline(login_file, line)) {
            stringstream ss(line);
            vector<string> cols;
            while (getline(ss, cell, ',')) cols.push_back(trim_teacher(cell));

            // If the email matches, grab the Username (Column 3 is index 3)
            if (cols.size() >= 4 && cols[0] == target_email) {
                this->name = cols[3];
                break;
            }
        }
        login_file.close();
    }

    // 2. Look up their courses in Offered_Courses.csv using their Name!
    ifstream courses_file("Databases/Offered_Courses.csv");
    if (courses_file.is_open()) {
        getline(courses_file, line); // Skip header
        while (getline(courses_file, line)) {
            stringstream ss(line);
            vector<string> cols;
            // Parse CSV accounting for commas
            while (getline(ss, cell, ',')) cols.push_back(trim_teacher(cell));

            // Offered_Courses Columns: Code(0), Pre1(1), Pre2(2), Name(3), Instructor(4)
            if (cols.size() >= 5) {
                if (cols[4] == this->name) { // If the instructor matches this teacher
                    // Format it as "CODE - Name"
                    string display_course = cols[0] + " - " + cols[3];
                    this->courses_taught.push_back(display_course);
                    cout << "Added Course: " << display_course << endl;
                }
            }
        }
        courses_file.close();
    } else {
        cerr << "Error: Could not open Databases/Offered_Courses.csv" << endl;
    }
}

void Teacher_Profile::reset() {
    name = "";
    email = "";
    courses_taught.clear();
}