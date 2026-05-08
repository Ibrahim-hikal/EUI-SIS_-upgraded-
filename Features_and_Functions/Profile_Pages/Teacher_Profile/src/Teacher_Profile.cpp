#include "../header/Teacher_Profile.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

// ---> THE SIMPLE CLEANER <---
// Strips invisible \r and spaces from the ends of strings
void clean_str(string& str) {
    while(!str.empty() && (str.back() == '\r' || str.back() == '\n' || str.back() == ' ')) str.pop_back();
    while(!str.empty() && str.front() == ' ') str.erase(0, 1);
}

void Teacher_Profile::load_profile(const string& target_email) {
    this->email = target_email;
    this->name = "";
    this->courses_taught.clear();

    // 1. Get the Teacher's real name
    ifstream login_file("Databases/Login_Credentials.csv");
    string line, cell;
    if (login_file.is_open()) {
        getline(login_file, line);
        while (getline(login_file, line)) {
            stringstream ss(line);
            vector<string> cols;
            // Clean every cell as we read it
            while (getline(ss, cell, ',')) { clean_str(cell); cols.push_back(cell); }

            if (cols.size() >= 4 && cols[0] == target_email) {
                this->name = cols[3];
                break;
            }
        }
        login_file.close();
    }

    // 2. Look up courses in Offered_Courses
    ifstream courses_file("Databases/Offered_Courses.csv");
    if (courses_file.is_open()) {
        getline(courses_file, line);
        while (getline(courses_file, line)) {
            stringstream ss(line);
            vector<string> cols;
            // Clean every cell as we read it
            while (getline(ss, cell, ',')) { clean_str(cell); cols.push_back(cell); }

            // Perfect string matching because the \r is gone!
            if (cols.size() >= 5 && cols[4] == this->name) {
                string display_course = cols[0] + " - " + cols[3];
                this->courses_taught.push_back(display_course);
            }
        }
        courses_file.close();
    }
}

void Teacher_Profile::reset() {
    name = ""; email = ""; courses_taught.clear();
}