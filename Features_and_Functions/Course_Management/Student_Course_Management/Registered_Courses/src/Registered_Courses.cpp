#include "../header/Registered_Courses.h"
#include "main.h"
#include <fstream>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

RegisteredCoursesManager::RegisteredCoursesManager(string id) : student_id(move(id)) {}

vector<string> RegisteredCoursesManager::parse_csv_line(const string& line) const {
    vector<string> result; string current; bool in_quotes = false;
    for (char c : line) {
        if (c == '"') in_quotes = !in_quotes;
        else if (c == ',' && !in_quotes) { result.push_back(current); current.clear(); }
        else current += c;
    }
    result.push_back(current); return result;
}

// Calculates attendance by checking Week 1 through 13 columns
string RegisteredCoursesManager::calculate_total_attendance(const string& course_code) const {
    ifstream file("Databases/Courses/" + course_code + "/Attendance.csv");
    if (!file.is_open()) return "No Data";

    string line;
    getline(file, line); // header
    while (getline(file, line)) {
        vector<string> cols = parse_csv_line(line);
        if (!cols.empty() && cols[0] == student_id) {
            int present_count = 0;
            int total_recorded_weeks = 0;

            for (int w = 1; w <= 13; ++w) {
                int col_idx = 3 + w;
                if (cols.size() > col_idx && !cols[col_idx].empty()) {
                    total_recorded_weeks++;
                    if (cols[col_idx] == "1") present_count++;
                }
            }
            return "Present: " + to_string(present_count) + " / " + to_string(total_recorded_weeks) + " weeks";
        }
    }
    return "0 / 0 weeks";
}

string RegisteredCoursesManager::calculate_total_grade(const string& course_code) const {
    ifstream file("Databases/Courses/" + course_code + "/Grades.csv");
    if (!file.is_open()) return "No Data";

    string line;
    getline(file, line);
    while (getline(file, line)) {
        vector<string> cols = parse_csv_line(line);
        if (cols.size() >= 10 && cols[0] == student_id) {
            return cols[8] + " (" + cols[9] + ")";
        }
    }
    return "No Data";
}

slint::SharedVector<CourseInfo> RegisteredCoursesManager::get_registered_courses_with_attendance() const {
    slint::SharedVector<CourseInfo> result_list;

    ifstream student_file("Databases/Data_on_Each_Student.csv");
    if (!student_file.is_open()) return result_list;

    string line; getline(student_file, line);
    string registered_str = "";
    while (getline(student_file, line)) {
        vector<string> cols = parse_csv_line(line);
        if (cols.size() >= 5 && cols[0] == student_id) {
            registered_str = cols[4];
            break;
        }
    }
    student_file.close();

    // Parse the encoded string: _CSE021\Tuesday 09:00-11:00//Wednesday...
    vector<string> enrolled_codes;
    size_t pos = 0;
    while ((pos = registered_str.find('_', pos)) != string::npos) {
        size_t end_pos = registered_str.find('\\', pos);
        if (end_pos != string::npos) enrolled_codes.push_back(registered_str.substr(pos + 1, end_pos - pos - 1));
        pos++;
    }

    // Map against Offered Courses to get names and instructors
    ifstream courses_file("Databases/Offered_Courses.csv");
    vector<vector<string>> offered_courses;
    if (courses_file.is_open()) {
        getline(courses_file, line);
        while (getline(courses_file, line)) offered_courses.push_back(parse_csv_line(line));
    }

    for (const string& code : enrolled_codes) {
        CourseInfo info;
        info.course_code = slint::SharedString(code);
        info.course_name = slint::SharedString("Unknown Name");
        info.instructor_name = slint::SharedString("Unknown Instructor");

        for (const auto& oc : offered_courses) {
            if (oc[0] == code) {
                info.course_name = slint::SharedString(oc[3]);
                info.instructor_name = slint::SharedString(oc[4]);
                break;
            }
        }

        // Inject the attendance calculation
        info.attendance_summary = slint::SharedString(calculate_total_attendance(code));
        info.grade_summary = slint::SharedString(calculate_total_grade(code));
        result_list.push_back(info);
    }
    return result_list;
}