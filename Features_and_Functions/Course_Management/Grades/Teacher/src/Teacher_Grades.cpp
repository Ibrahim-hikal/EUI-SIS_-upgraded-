//
// Created by ahmed on 4/25/2026.
//

#include "../header/Teacher_Grades.h"
// Readability pass: includes and declarations are kept visually grouped.
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;

void TeacherGradesManager::initUI(Main_App* ui, const string& name) {
    this->teacher_name = name;

    ui->on_load_grades([this, ui](slint::SharedString course) {
        string course_str(course.data());
        course_str.erase(std::remove(course_str.begin(), course_str.end(), '\r'), course_str.end());
        course_str.erase(std::remove(course_str.begin(), course_str.end(), '\n'), course_str.end());

        auto vec = this->get_grades_for_course(course_str);
        vector<StudentGradeData> std_vec;
        for (int i = 0; i < vec.size(); ++i) std_vec.push_back(vec[i]);

        auto model = std::make_shared<slint::VectorModel<StudentGradeData>>(std_vec);
        ui->set_grades_data(model);
    });

    ui->on_save_grades([this](std::shared_ptr<slint::Model<StudentGradeData>> data, slint::SharedString course) {
        slint::SharedVector<StudentGradeData> vec;
        for (int i = 0; i < data->row_count(); ++i) {
            if (auto row = data->row_data(i)) vec.push_back(*row);
        }

        string course_str(course.data());
        course_str.erase(std::remove(course_str.begin(), course_str.end(), '\r'), course_str.end());
        course_str.erase(std::remove(course_str.begin(), course_str.end(), '\n'), course_str.end());

        if (this->save_grades(course_str, vec)) cout << "Grades saved successfully!" << endl;
        else cout << "Failed to save grades." << endl;
    });
}

vector<string> TeacherGradesManager::parse_csv_line(const string& line) const {
    vector<string> result; string current; bool in_quotes = false;
    for (char c : line) {
        if (c == '\r') continue;
        if (c == '"') in_quotes = !in_quotes;
        else if (c == ',' && !in_quotes) { result.push_back(current); current.clear(); }
        else current += c;
    }
    result.push_back(current); return result;
}

slint::SharedVector<StudentGradeData> TeacherGradesManager::get_grades_for_course(const string& course_code) const {
    slint::SharedVector<StudentGradeData> students;
    ifstream file("Databases/Courses/" + course_code + "/Grades.csv");
    if (!file.is_open()) return students;

    string line; getline(file, line);
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> cols = parse_csv_line(line);
        if (cols.size() >= 1) {
            StudentGradeData data;
            data.id = slint::SharedString(cols[0]);
            data.quiz1 = slint::SharedString(cols.size() > 1 ? cols[1] : "");
            data.quiz2 = slint::SharedString(cols.size() > 2 ? cols[2] : "");
            data.ass1 = slint::SharedString(cols.size() > 3 ? cols[3] : "");
            data.ass2 = slint::SharedString(cols.size() > 4 ? cols[4] : "");
            data.midterm = slint::SharedString(cols.size() > 5 ? cols[5] : "");
            data.project = slint::SharedString(cols.size() > 6 ? cols[6] : "");
            data.final_mark = slint::SharedString(cols.size() > 7 ? cols[7] : "");
            data.total = slint::SharedString(cols.size() > 8 ? cols[8] : "");
            data.letter = slint::SharedString(cols.size() > 9 ? cols[9] : "");
            data.gpa = slint::SharedString(cols.size() > 10 ? cols[10] : "");
            students.push_back(data);
        }
    }
    return students;
}

bool TeacherGradesManager::save_grades(const string& course_code, const slint::SharedVector<StudentGradeData>& records) const {
    string filepath = "Databases/Courses/" + course_code + "/Grades.csv";
    ifstream infile(filepath);
    if (!infile.is_open()) return false;

    vector<vector<string>> csv_data;
    string line;
    while (getline(infile, line)) csv_data.push_back(parse_csv_line(line));
    infile.close();

    for (size_t i = 1; i < csv_data.size(); ++i) {
        if (csv_data[i].empty()) continue;
        string id = csv_data[i][0];

        for (size_t j = 0; j < records.size(); ++j) {
            if (string(records[j].id) == id) {
                while (csv_data[i].size() <= 10) csv_data[i].push_back("");
                csv_data[i][1] = string(records[j].quiz1);
                csv_data[i][2] = string(records[j].quiz2);
                csv_data[i][3] = string(records[j].ass1);
                csv_data[i][4] = string(records[j].ass2);
                csv_data[i][5] = string(records[j].midterm);
                csv_data[i][6] = string(records[j].project);
                csv_data[i][7] = string(records[j].final_mark);
                csv_data[i][8] = string(records[j].total);
                csv_data[i][9] = string(records[j].letter);
                csv_data[i][10] = string(records[j].gpa);
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