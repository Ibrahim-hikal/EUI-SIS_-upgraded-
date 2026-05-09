#include "../header/Assign_Courses.h"
// Readability pass: includes and declarations are kept visually grouped.
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>

#define TEACHER_DB "Databases/Data_on_each_teacher.csv"
#define COURSES_DB "Databases/Offered_Courses.csv"

using namespace std;

static inline bool robust_match(string a, string b) {
    a.erase(remove_if(a.begin(), a.end(), [](unsigned char c){ return isspace(c) || c == '\r' || c == '\n'; }), a.end());
    b.erase(remove_if(b.begin(), b.end(), [](unsigned char c){ return isspace(c) || c == '\r' || c == '\n'; }), b.end());
    transform(a.begin(), a.end(), a.begin(), [](unsigned char c){ return tolower(c); });
    transform(b.begin(), b.end(), b.begin(), [](unsigned char c){ return tolower(c); });
    return a == b;
}

static inline string clean_edges(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n\"");
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n\"");
    return str.substr(first, (last - first + 1));
}

string Assign_Course::get_teacher_name(const string& email) {
    ifstream file(TEACHER_DB);
    string line, name, e;
    if (file.is_open()) {
        getline(file, line);
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            getline(ss, name, ',');
            getline(ss, e, ',');
            if (robust_match(e, email)) return clean_edges(name);
        }
    }
    return "Not Found";
}

string Assign_Course::get_course_name(const string& code) {
    ifstream file(COURSES_DB);
    string line, c, p1, p2, name;
    if (file.is_open()) {
        getline(file, line);
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            getline(ss, c, ',');
            getline(ss, p1, ',');
            getline(ss, p2, ',');
            getline(ss, name, ',');
            if (robust_match(c, code)) return clean_edges(name);
        }
    }
    return "Not Found";
}

// ---> LIVE UI CHECKER <---
string Assign_Course::check_conflicts(const string& email, const string& code, const string& l1, const string& l2, const string& t1, const string& t2) {
    string t_name = get_teacher_name(email);
    if (t_name == "Not Found") return ""; // Let the UI handle missing teacher

    vector<string> active_slots;
    if (l1 != "None") active_slots.push_back(l1);
    if (l2 != "None") active_slots.push_back(l2);
    if (t1 != "None") active_slots.push_back(t1);
    if (t2 != "None") active_slots.push_back(t2);

    // 1. Check for Duplicate Times in the current form
    for (size_t i = 0; i < active_slots.size(); ++i) {
        for (size_t j = i + 1; j < active_slots.size(); ++j) {
            if (active_slots[i] == active_slots[j]) {
                return "Conflict: Duplicate time selected (" + active_slots[i] + ")";
            }
        }
    }

    // 2. Check Database Conflicts (GLOBAL OVERLAP CHECK)
    ifstream conflictFile(COURSES_DB);
    string line;
    if (conflictFile.is_open()) {
        getline(conflictFile, line);
        while (getline(conflictFile, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string cell; vector<string> cols;
            while(getline(ss, cell, ',')) cols.push_back(cell);

            // We check ALL courses now, not just the current teacher's courses
            if (cols.size() >= 7) {
                if (robust_match(cols[0], code)) continue; // Skip the course we are currently editing

                string existing_lecs = cols[5];
                string existing_tuts = cols[6];

                for (const string& slot : active_slots) {
                    if (existing_lecs.find(slot) != string::npos || existing_tuts.find(slot) != string::npos) {
                        if (robust_match(cols[4], t_name)) {
                            return "Teacher is already teaching " + clean_edges(cols[0]) + " on " + slot;
                        } else {
                            // Another teacher is using this slot!
                            return "Time Overlap! " + clean_edges(cols[0]) + " is already scheduled on " + slot;
                        }
                    }
                }
            }
        }
        conflictFile.close();
    }
    return ""; // Empty string means NO CONFLICTS!
}

string Assign_Course::assign_course(const string& email, const string& code, const string& l1, const string& l2, const string& t1, const string& t2) {
    try {
        string t_name = get_teacher_name(email);
        string c_name = get_course_name(code);

        if (t_name == "Not Found") return "Error: Teacher Email not found.";
        if (c_name == "Not Found") return "Error: Course Code not found.";

        string conflict = check_conflicts(email, code, l1, l2, t1, t2);
        if (!conflict.empty()) return conflict;

        string lec_str = "";
        if (l1 != "None") lec_str += l1;
        if (l2 != "None") { if (!lec_str.empty()) lec_str += " "; lec_str += l2; }

        string tut_str = "";
        if (t1 != "None") tut_str += t1;
        if (t2 != "None") { if (!tut_str.empty()) tut_str += " "; tut_str += t2; }

        // 3. Update Teacher Database (GHOST TEACHER CLEANUP)
        ifstream fileIn(TEACHER_DB);
        vector<string> lines;
        bool updated = false;
        string line;

        if (fileIn.is_open()) {
            while (getline(fileIn, line)) {
                if (line.empty()) continue;
                if (line.back() == '\r') line.pop_back();
                stringstream ss(line);
                string name, e, courses;
                getline(ss, name, ','); getline(ss, e, ','); getline(ss, courses);

                if (robust_match(e, email)) {
                    // NEW TEACHER: Add the course to their profile
                    if (courses == "\"\"" || clean_edges(courses).empty() || courses == "-1") {
                        courses = c_name;
                    } else if (courses.find(c_name) == string::npos) {
                        courses += "_" + c_name;
                    }
                    line = name + "," + e + "," + courses;
                    updated = true;
                } else {
                    // OLD TEACHERS: Sweep and remove the course so it doesn't duplicate
                    if (courses.find(c_name) != string::npos) {
                        string new_courses = "";
                        stringstream css(courses);
                        string crs;
                        while(getline(css, crs, '_')) {
                            if (!robust_match(crs, c_name) && !clean_edges(crs).empty()) {
                                if (!new_courses.empty()) new_courses += "_";
                                new_courses += crs;
                            }
                        }
                        if (new_courses.empty()) new_courses = "-1";
                        line = name + "," + e + "," + new_courses;
                        updated = true;
                    }
                }
                lines.push_back(line);
            }
            fileIn.close();
        }

        if (updated) {
            ofstream fileOut(TEACHER_DB, ios::trunc);
            for (const auto& l : lines) fileOut << l << "\n";
            fileOut.close();
        }

        // 4. Update Offered Courses Database
        ifstream fileInC(COURSES_DB);
        vector<string> c_lines;
        bool c_updated = false;

        if (fileInC.is_open()) {
            while (getline(fileInC, line)) {
                if (line.empty()) continue;
                if (line.back() == '\r') line.pop_back();
                stringstream ss(line); string cell; vector<string> cols;
                while (getline(ss, cell, ',')) cols.push_back(cell);

                if (cols.size() >= 8 && robust_match(cols[0], code)) {
                    cols[4] = t_name; cols[5] = lec_str; cols[6] = tut_str;
                    line = cols[0];
                    for(size_t i = 1; i < cols.size(); ++i) line += "," + cols[i];
                    c_updated = true;
                }
                c_lines.push_back(line);
            }
            fileInC.close();
        }

        if (c_updated) {
            ofstream fileOutC(COURSES_DB, ios::trunc);
            for (const auto& l : c_lines) fileOutC << l << "\n";
            fileOutC.close();
            return "Successfully assigned " + code + " to " + t_name + "!";
        }
        return "Error: Course " + code + " not found in Offered DB.";
    } catch (...) { return "Crash Prevented! Unknown memory error occurred."; }
}