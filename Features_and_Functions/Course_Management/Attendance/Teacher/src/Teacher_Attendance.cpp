#include "../header/Teacher_Attendance.h"
#include "main.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

// ---> THE ULTRA-ROBUST MATCHER <---
static inline bool is_exact_match(string a, string b) {
    a.erase(remove_if(a.begin(), a.end(), [](unsigned char c){ return isspace(c) || c == '\r' || c == '\n'; }), a.end());
    b.erase(remove_if(b.begin(), b.end(), [](unsigned char c){ return isspace(c) || c == '\r' || c == '\n'; }), b.end());
    transform(a.begin(), a.end(), a.begin(), [](unsigned char c){ return tolower(c); });
    transform(b.begin(), b.end(), b.begin(), [](unsigned char c){ return tolower(c); });
    return a == b;
}

static inline string trim_edges(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void TeacherAttendanceManager::initUI(Main_App* ui, const string& name) {
    this->teacher_name = trim_edges(name);

    auto courses_vec = get_available_courses();
    vector<CourseInfo> std_courses;
    for (int i = 0; i < courses_vec.size(); ++i) {
        CourseInfo info;
        info.course_code = courses_vec[i];
        info.course_name = slint::SharedString("");
        std_courses.push_back(info);
    }

    auto courses_model = make_shared<slint::VectorModel<CourseInfo>>(std_courses);
    ui->set_available_courses(courses_model);

    ui->on_load_students([this, ui](slint::SharedString course, int week) {
        string course_str(course.data());
        course_str = trim_edges(course_str);
        auto vec = this->get_students_for_week(course_str, week);
        vector<StudentAttendanceData> std_vec;
        for (int i = 0; i < vec.size(); ++i) std_vec.push_back(vec[i]);
        auto model = make_shared<slint::VectorModel<StudentAttendanceData>>(std_vec);
        ui->set_students_data(model);
    });

    if (!std_courses.empty()) ui->invoke_load_students(std_courses[0].course_code, 1);

    ui->on_save_attendance([this](shared_ptr<slint::Model<StudentAttendanceData>> data, slint::SharedString course, int week) {
        slint::SharedVector<StudentAttendanceData> vec;
        for (int i = 0; i < data->row_count(); ++i) {
            if (auto row = data->row_data(i)) vec.push_back(*row);
        }
        string course_str = trim_edges(string(course.data()));
        bool success = this->save_attendance(course_str, week, vec);
        if(success) cout << "Attendance saved!" << endl;
    });
}

vector<string> TeacherAttendanceManager::parse_csv_line(const string& line) const {
    vector<string> result; string current; bool in_quotes = false;
    for (char c : line) {
        if (c == '"') { in_quotes = !in_quotes; }
        else if (c == ',' && !in_quotes) { result.push_back(trim_edges(current)); current.clear(); }
        else { current += c; }
    }
    result.push_back(trim_edges(current)); return result;
}

slint::SharedVector<slint::SharedString> TeacherAttendanceManager::get_available_courses() const {
    slint::SharedVector<slint::SharedString> courses;
    ifstream file("Databases/Offered_Courses.csv");
    if (!file.is_open()) return courses;

    string line;
    getline(file, line);
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> cols = parse_csv_line(line);

        // USE THE ROBUST MATCHER!
        if (cols.size() > 4 && is_exact_match(cols[4], teacher_name)) {
            courses.push_back(slint::SharedString(cols[0]));
        }
    }
    return courses;
}

slint::SharedVector<StudentAttendanceData> TeacherAttendanceManager::get_students_for_week(const string& course_code, int week) const {
    slint::SharedVector<StudentAttendanceData> students;
    ifstream file("Databases/Courses/" + course_code + "/Attendance.csv");
    if (!file.is_open()) return students;
    string line; getline(file, line);
    int target_col = 3 + week;

    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> cols = parse_csv_line(line);
        if (cols.size() >= 2 && !cols[0].empty()) {
            StudentAttendanceData data;
            data.id = slint::SharedString(cols[0]); data.name = slint::SharedString(cols[1]);
            data.present = (cols.size() > target_col && cols[target_col] == "1");
            students.push_back(data);
        }
    }
    return students;
}

bool TeacherAttendanceManager::save_attendance(const string& course_code, int week, const slint::SharedVector<StudentAttendanceData>& records) const {
    string filepath = "Databases/Courses/" + course_code + "/Attendance.csv";
    ifstream infile(filepath);
    if (!infile.is_open()) return false;
    vector<vector<string>> csv_data;
    string line;
    while (getline(infile, line)) csv_data.push_back(parse_csv_line(line));
    infile.close();

    int target_col = 3 + week;
    for (size_t i = 1; i < csv_data.size(); ++i) {
        if (csv_data[i].empty()) continue;
        string id = trim_edges(csv_data[i][0]);
        for (size_t j = 0; j < records.size(); ++j) {
            if (trim_edges(string(records[j].id)) == id) {
                while (csv_data[i].size() <= target_col) csv_data[i].push_back("");
                csv_data[i][target_col] = records[j].present ? "1" : "0";
                break;
            }
        }
    }
    ofstream outfile(filepath);
    for (const auto& row : csv_data) {
        for (size_t i = 0; i < row.size(); ++i) outfile << row[i] << (i < row.size() - 1 ? "," : "");
        outfile << "\n";
    }
    return true;
}