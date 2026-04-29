#include "../header/Teacher_Attendance.h"
#include "main.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

void TeacherAttendanceManager::initUI(Main_App* ui, const string& name) {
    this->teacher_name = name;

    // 1. Fetch available courses and pass to UI
    auto courses_vec = get_available_courses();
    std::vector<CourseInfo> std_courses; // <-- CHANGED: Now holds CourseInfo

    for (int i = 0; i < courses_vec.size(); ++i) {
        CourseInfo info;
        info.course_code = courses_vec[i];
        info.course_name = slint::SharedString(""); // Empty string since your CSV just gives the code
        std_courses.push_back(info);
    }

    // <-- CHANGED: Model is now CourseInfo
    auto courses_model = std::make_shared<slint::VectorModel<CourseInfo>>(std_courses);
    ui->set_available_courses(courses_model);

    // 2. Register the load students callback FIRST
    ui->on_load_students([this, ui](slint::SharedString course, int week) {
        std::string course_str(course.data());
        // Clean any hidden carriage returns from CSV parsing
        course_str.erase(std::remove(course_str.begin(), course_str.end(), '\r'), course_str.end());
        course_str.erase(std::remove(course_str.begin(), course_str.end(), '\n'), course_str.end());

        auto vec = this->get_students_for_week(course_str, week);
        std::vector<StudentAttendanceData> std_vec;
        for (int i = 0; i < vec.size(); ++i) {
            std_vec.push_back(vec[i]);
        }
        auto model = std::make_shared<slint::VectorModel<StudentAttendanceData>>(std_vec);
        ui->set_students_data(model);
    });

    // 3. Auto-load the first course's students AFTER the callback is registered
    if (!std_courses.empty()) {
        // <-- CHANGED: We now access .course_code from the struct
        ui->invoke_load_students(std_courses[0].course_code, 1);
    }

    // 4. Register the save attendance callback
    ui->on_save_attendance([this](std::shared_ptr<slint::Model<StudentAttendanceData>> data, slint::SharedString course, int week) {
        slint::SharedVector<StudentAttendanceData> vec;
        for (int i = 0; i < data->row_count(); ++i) {
            if (auto row = data->row_data(i)) {
                vec.push_back(*row);
            }
        }

        std::string course_str(course.data());
        course_str.erase(std::remove(course_str.begin(), course_str.end(), '\r'), course_str.end());
        course_str.erase(std::remove(course_str.begin(), course_str.end(), '\n'), course_str.end());

        bool success = this->save_attendance(course_str, week, vec);
        if(success) {
            cout << "Attendance for " << course_str << " Week " << week << " saved successfully!" << endl;
        } else {
            cout << "Failed to save attendance." << endl;
        }
    });
}

vector<string> TeacherAttendanceManager::parse_csv_line(const string& line) const {
    vector<string> result;
    string current;
    bool in_quotes = false;
    for (char c : line) {
        if (c == '\r') continue; // Fix: Strip \r to prevent path corruption
        if (c == '"') { in_quotes = !in_quotes; }
        else if (c == ',' && !in_quotes) { result.push_back(current); current.clear(); }
        else { current += c; }
    }
    result.push_back(current);
    return result;
}

slint::SharedVector<slint::SharedString> TeacherAttendanceManager::get_available_courses() const {
    slint::SharedVector<slint::SharedString> courses;
    ifstream file("Offered_Courses.csv");
    if (!file.is_open()) return courses;

    string line;
    getline(file, line); // Skip header
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> cols = parse_csv_line(line);
        if (cols.size() > 4 && cols[4] == teacher_name) {
            courses.push_back(slint::SharedString(cols[0]));
        }
    }
    return courses;
}

slint::SharedVector<StudentAttendanceData> TeacherAttendanceManager::get_students_for_week(const string& course_code, int week) const {
    slint::SharedVector<StudentAttendanceData> students;
    string filepath = "Databases/Courses/" + course_code + "/Attendance.csv";
    ifstream file(filepath);
    if (!file.is_open()) return students;

    string line;
    getline(file, line);
    int target_col = 3 + week;

    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> cols = parse_csv_line(line);
        if (cols.size() >= 2 && !cols[0].empty()) {
            StudentAttendanceData data;
            data.id = slint::SharedString(cols[0]);
            data.name = slint::SharedString(cols[1]);
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
        string id = csv_data[i][0];

        for (size_t j = 0; j < records.size(); ++j) {
            if (string(records[j].id) == id) {
                while (csv_data[i].size() <= target_col) csv_data[i].push_back("");
                csv_data[i][target_col] = records[j].present ? "1" : "0";
                break;
            }
        }
    }

    ofstream outfile(filepath);
    for (const auto& row : csv_data) {
        for (size_t i = 0; i < row.size(); ++i) {
            outfile << row[i] << (i < row.size() - 1 ? "," : "");
        }
        outfile << "\n";
    }
    return true;
}