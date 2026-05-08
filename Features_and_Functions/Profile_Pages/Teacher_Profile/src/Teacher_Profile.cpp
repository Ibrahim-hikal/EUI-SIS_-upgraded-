#include "../header/Teacher_Profile.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

// ---> THE ULTRA-ROBUST MATCHER <---
// Strips all spaces, invisible characters, and ignores uppercase/lowercase
static inline bool is_exact_match(string a, string b) {
    a.erase(remove_if(a.begin(), a.end(), [](unsigned char c){ return isspace(c) || c == '\r' || c == '\n'; }), a.end());
    b.erase(remove_if(b.begin(), b.end(), [](unsigned char c){ return isspace(c) || c == '\r' || c == '\n'; }), b.end());
    transform(a.begin(), a.end(), a.begin(), [](unsigned char c){ return tolower(c); });
    transform(b.begin(), b.end(), b.begin(), [](unsigned char c){ return tolower(c); });
    return a == b;
}

// Just trims the edges for displaying nicely on the UI
static inline string trim_edges(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
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
            while (getline(ss, cell, ',')) { cols.push_back(trim_edges(cell)); }

            // USE THE ROBUST MATCHER!
            if (cols.size() >= 4 && is_exact_match(cols[0], target_email)) {
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
            while (getline(ss, cell, ',')) { cols.push_back(trim_edges(cell)); }

            // USE THE ROBUST MATCHER!
            if (cols.size() >= 5 && is_exact_match(cols[4], this->name)) {
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