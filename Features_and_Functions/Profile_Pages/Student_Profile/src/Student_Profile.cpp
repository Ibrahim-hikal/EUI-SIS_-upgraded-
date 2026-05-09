#include "../header/Student_Profile.h"
// Readability pass: includes and declarations are kept visually grouped.
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

#define STUDENT_DB "Databases/Data_on_Each_Student.csv"
#define LOGIN_DB "Databases/Login_Credentials.csv"

using namespace std;

// ---> 1. BULLETPROOF MATCHER (Ignores spaces, \r, and caps) <---
static inline bool robust_match(string a, string b) {
    a.erase(remove_if(a.begin(), a.end(), [](unsigned char c){ return isspace(c) || c == '\r' || c == '\n'; }), a.end());
    b.erase(remove_if(b.begin(), b.end(), [](unsigned char c){ return isspace(c) || c == '\r' || c == '\n'; }), b.end());
    transform(a.begin(), a.end(), a.begin(), [](unsigned char c){ return tolower(c); });
    transform(b.begin(), b.end(), b.begin(), [](unsigned char c){ return tolower(c); });
    return a == b;
}

// ---> 2. EDGE CLEANER (Removes invisible CSV characters) <---
static inline string clean_edges(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n\"");
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n\"");
    return str.substr(first, (last - first + 1));
}

void Student_Profile::reset() {
    this->name = "";
    this->email = "";
    this->id = "";
    this->faculty = "";
    this->gpa = "";
    this->registered_courses.clear();
}

void Student_Profile::load_profile(const string &current_email) {
    reset();

    // ---> 3. LOAD LOGIN DATA (Name, ID, Email) <---
    ifstream login_file(LOGIN_DB);
    string line, cell;
    if (login_file.is_open()) {
        getline(login_file, line);
        while (getline(login_file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            vector<string> cols;
            while (getline(ss, cell, ',')) cols.push_back(clean_edges(cell));

            // Check BOTH Email (cols[0]) and ID (cols[4]) just in case!
            if (cols.size() >= 5 && (robust_match(cols[0], current_email) || robust_match(cols[4], current_email))) {
                this->email = cols[0]; // Lock in the official email
                this->name = cols[3];
                this->id = cols[4];
                break;
            }
        }
        login_file.close();
    }

    // ---> 4. LOAD ACADEMIC DATA (Faculty, GPA, Courses) <---
    ifstream student_file(STUDENT_DB);
    if (student_file.is_open()) {
        getline(student_file, line);
        while (getline(student_file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            vector<string> cols;
            while (getline(ss, cell, ',')) cols.push_back(clean_edges(cell));

            if (cols.size() >= 11 && robust_match(cols[0], this->id)) {
                this->faculty = cols[9];
                this->gpa = cols[10];
                string reg_str = cols[4];
                if (reg_str != "-1" && !reg_str.empty()) {
                    stringstream reg_ss(reg_str);
                    string token;
                    while (getline(reg_ss, token, '_')) {
                        if (!token.empty()) {
                            size_t slash = token.find('\\');
                            registered_courses.push_back((slash != string::npos) ? token.substr(0, slash) : token);
                        }
                    }
                }
                break;
            }
        }
        student_file.close();
    }
}