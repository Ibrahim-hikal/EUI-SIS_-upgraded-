#include "../header/Admin_Profile.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <filesystem>
#include <iomanip>

using namespace std;

void Admin_Profile::reset() {
    this->name = "";
    this->email = "";
    this->position = "";
}

void Admin_Profile::load_profile(const string &current_email) {
    reset();
    this->email = current_email;

    ifstream file("Databases/Data_on_each_admin.csv");
    string line, cell;
    if (file.is_open()) {
        getline(file, line);
        while (getline(file, line)) {
            if (line.empty()) continue;
            if (line.back() == '\r') line.pop_back();

            stringstream ss(line);
            vector<string> cols;
            while (getline(ss, cell, ',')) cols.push_back(cell);
            if (cols.size() >= 3 && cols[1] == current_email) {
                this->name = cols[0];
                this->position = cols[2];
                break;
            }
        }
        file.close();
    }
}

// ---> CHANGED FROM VOID TO STRING <---
string Admin_Profile::end_semester() {
    string student_db = "Databases/Data_on_Each_Student.csv";
    ifstream file_in(student_db);
    if (!file_in.is_open()) {
        return "ERROR: Could not open Databases/Data_on_Each_Student.csv";
    }

    vector<string> lines;
    string header;
    getline(file_in, header);
    if (!header.empty() && header.back() == '\r') header.pop_back();
    lines.push_back(header);

    string line;
    while (getline(file_in, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();

        stringstream ss(line);
        string cell;
        vector<string> cols;
        while (getline(ss, cell, ',')) cols.push_back(cell);

        if (cols.size() < 11) { lines.push_back(line); continue; }

        string id = cols[0];
        string passed_str = cols[2];
        string failed_str = cols[3];
        string reg_str = cols[4];

        if (reg_str != "-1" && !reg_str.empty()) {
            vector<string> current_reg;
            stringstream reg_ss(reg_str);
            string token;
            while (getline(reg_ss, token, '_')) {
                if (token.empty()) continue;
                size_t slash = token.find('\\');
                current_reg.push_back((slash != string::npos) ? token.substr(0, slash) : token);
            }

            double total_points = 0.0;
            int total_count = 0;

            auto count_history = [&](string s, double pts) {
                if (s == "-1" || s.empty()) return;
                stringstream ss_h(s);
                string h;
                while (getline(ss_h, h, '_')) {
                    if (!h.empty()) { total_points += pts; total_count++; }
                }
            };
            count_history(passed_str, 3.0);
            count_history(failed_str, 0.0);

            string new_passed = (passed_str == "-1") ? "" : passed_str;
            string new_failed = (failed_str == "-1") ? "" : failed_str;

            for (const string& course_code : current_reg) {
                string grades_path = "";
                try {
                    if (std::filesystem::exists("Databases/Courses")) {
                        for (const auto& entry : std::filesystem::recursive_directory_iterator("Databases/Courses")) {
                            if (entry.is_regular_file() && entry.path().filename() == "Grades.csv" && entry.path().parent_path().filename() == course_code) {
                                grades_path = entry.path().string();
                                break;
                            }
                        }
                    }
                } catch (...) {}

                double mark = -1.0;
                if (!grades_path.empty()) {
                    ifstream g_file(grades_path);
                    string g_line;
                    getline(g_file, g_line);
                    while (getline(g_file, g_line)) {
                        if (g_line.empty()) continue;
                        if (g_line.back() == '\r') g_line.pop_back();

                        stringstream g_ss(g_line);
                        string g_cell;
                        vector<string> g_cols;
                        while (getline(g_ss, g_cell, ',')) g_cols.push_back(g_cell);
                        if (!g_cols.empty() && g_cols[0] == id) {
                            if (g_cols.size() >= 9 && !g_cols[8].empty()) {
                                try { mark = stod(g_cols[8]); } catch(...) { mark = -1.0; }
                            }
                            break;
                        }
                    }
                }

                total_count++;
                if (mark >= 60.0) {
                    if (!new_passed.empty()) new_passed += "_";
                    new_passed += course_code;
                    if (mark >= 97) total_points += 4.0;
                    else if (mark >= 93) total_points += 4.0;
                    else if (mark >= 89) total_points += 3.7;
                    else if (mark >= 84) total_points += 3.3;
                    else if (mark >= 80) total_points += 3.0;
                    else if (mark >= 76) total_points += 2.7;
                    else if (mark >= 73) total_points += 2.3;
                    else if (mark >= 70) total_points += 2.0;
                    else if (mark >= 67) total_points += 1.7;
                    else if (mark >= 64) total_points += 1.3;
                    else if (mark >= 60) total_points += 1.0;
                } else {
                    if (!new_failed.empty()) new_failed += "_";
                    new_failed += course_code;
                }
            }

            cols[2] = new_passed.empty() ? "-1" : new_passed;
            cols[3] = new_failed.empty() ? "-1" : new_failed;

            double final_gpa = (total_count > 0) ? (total_points / total_count) : 0.0;
            stringstream gpa_ss;
            gpa_ss << fixed << setprecision(2) << final_gpa;
            cols[10] = gpa_ss.str();
        }

        cols[4] = "-1"; // Clear Registered
        cols[5] = "-1"; // Clear Requested
        cols[6] = "";   // Clear Excuse Requests
        cols[7] = "0";  // Clear Total Excuses Accepted
        cols[8] = "";   // Clear Withdrawal Requests

        string updated_line = "";
        for (size_t i = 0; i < cols.size(); ++i) {
            updated_line += cols[i] + (i == cols.size() - 1 ? "" : ",");
        }
        lines.push_back(updated_line);
    }
    file_in.close();

    // ---> EXCEL LOCK CHECKER <---
    ofstream file_out(student_db, ios::trunc);
    if (!file_out.is_open()) return "ERROR: Database locked! Close your CSV files in Excel and try again.";

    for (const auto& l : lines) file_out << l << "\n";
    file_out.close();

    try {
        ofstream wOut("Databases/Course_Withdrawals.csv", ios::trunc);
        if (wOut.is_open()) wOut.close();

        ofstream eOut("Databases/Attendance_Excuses.csv", ios::trunc);
        if (eOut.is_open()) eOut.close();
    } catch (...) {}

    return "Semester Ended Successfully! Databases cleared for the new term.";
}