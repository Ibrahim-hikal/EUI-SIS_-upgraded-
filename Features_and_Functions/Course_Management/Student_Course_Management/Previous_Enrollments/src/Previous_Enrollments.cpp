#include "../header/Previous_Enrollments.h"
// Readability pass: includes and declarations are kept visually grouped.
#include "main.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

// Helper to strip hidden whitespace and carriage returns that break matching
static string trim_str(string s) {
    if (s.empty()) return s;
    s.erase(0, s.find_first_not_of(" \t\r\n\""));
    s.erase(s.find_last_not_of(" \t\r\n\"") + 1);
    return s;
}

PreviousEnrollmentsManager::PreviousEnrollmentsManager(string id) : student_id(move(id)) {}

vector<string> PreviousEnrollmentsManager::parse_csv_line(const string& line) const {
    vector<string> result; string current; bool in_quotes = false;
    for (char c : line) {
        if (c == '"') in_quotes = !in_quotes;
        else if (c == ',' && !in_quotes) { result.push_back(current); current.clear(); }
        else current += c;
    }
    result.push_back(current); return result;
}

vector<string> PreviousEnrollmentsManager::split_courses(const string& course_str) const {
    vector<string> result;
    if (course_str == "-1" || course_str.empty() || trim_str(course_str) == "") return result;

    string current;
    for (char c : course_str) {
        // Splits by anything that isn't a letter or number (handles spaces, commas, underscores, slashes, etc.)
        if (isalnum(c)) {
            current += c;
        } else {
            if (!current.empty() && current != "-1") {
                result.push_back(current);
                current.clear();
            }
        }
    }
    if (!current.empty() && current != "-1") result.push_back(current);
    return result;
}

void PreviousEnrollmentsManager::calculate_total_grade(const string& course_code, string& out_details, bool& out_has_grades) const {
    out_has_grades = false;
    out_details = "";

    ifstream file("Databases/Courses/" + course_code + "/Grades.csv");
    if (!file.is_open()) return;

    string line;
    getline(file, line); // Skip header

    int max_weights[] = {0, 5, 5, 5, 5, 25, 15, 40};
    string labels[] = {"", "Quiz 1", "Quiz 2", "Assignment 1", "Assignment 2", "Midterm", "Project", "Final"};

    while (getline(file, line)) {
        vector<string> cols = parse_csv_line(line);
        if (cols.size() >= 10 && trim_str(cols[0]) == student_id) {
            out_has_grades = true;
            string details = "";
            for (int i = 1; i <= 7; ++i) {
                if (cols.size() > i && !trim_str(cols[i]).empty()) {
                    if (!details.empty()) details += "\n";
                    details += labels[i] + ": " + trim_str(cols[i]) + "/" + to_string(max_weights[i]);
                }
            }
            out_details = details.empty() ? "No grades entered." : details;
            return;
        }
    }
}

void PreviousEnrollmentsManager::load_student_data(Main_App* ui) {
    ifstream student_file("Databases/Data_on_Each_Student.csv");
    string passed_str = "", failed_str = "";

    if (student_file.is_open()) {
        string line;
        if (getline(student_file, line)) { // Read header to dynamically find columns
            vector<string> headers = parse_csv_line(line);
            int passed_idx = 2; // Fallbacks
            int failed_idx = 3;

            for (size_t i = 0; i < headers.size(); ++i) {
                string h = trim_str(headers[i]);
                if (h == "Passed Courses") passed_idx = i;
                else if (h == "Failed Courses") failed_idx = i;
            }

            while (getline(student_file, line)) {
                vector<string> cols = parse_csv_line(line);
                if (cols.size() > max(passed_idx, failed_idx) && trim_str(cols[0]) == student_id) {
                    passed_str = cols[passed_idx];
                    failed_str = cols[failed_idx];
                    break;
                }
            }
        }
        student_file.close();
    }

    vector<string> passed_codes = split_courses(passed_str);
    vector<string> failed_codes = split_courses(failed_str);

    // Map against Offered Courses to get names and instructors
    ifstream courses_file("Databases/Offered_Courses.csv");
    vector<vector<string>> offered_courses;
    if (courses_file.is_open()) {
        string line; getline(courses_file, line); // Skip header
        while (getline(courses_file, line)) {
            offered_courses.push_back(parse_csv_line(line));
        }
        courses_file.close();
    }

    auto process_courses = [&](const vector<string>& codes, bool is_failed) {
        std::vector<CourseInfo> std_vec;
        for (const string& code : codes) {
            CourseInfo info;
            info.course_code = slint::SharedString(code);
            info.course_name = slint::SharedString("Unknown Name");
            info.instructor_name = slint::SharedString("Unknown Instructor");
            info.is_failed = is_failed;

            for (const auto& oc : offered_courses) {
                if (oc.size() > 4 && trim_str(oc[0]) == code) {
                    info.course_name = slint::SharedString(trim_str(oc[3]));
                    info.instructor_name = slint::SharedString(trim_str(oc[4]));
                    break;
                }
            }

            string details;
            bool has_grades;
            calculate_total_grade(code, details, has_grades);

            info.detailed_grades = slint::SharedString(details);
            info.has_final_grade = has_grades;

            std_vec.push_back(info);
        }
        return std::make_shared<slint::VectorModel<CourseInfo>>(std_vec);
    };

    ui->set_passed_courses(process_courses(passed_codes, false));
    ui->set_failed_courses(process_courses(failed_codes, true));
}