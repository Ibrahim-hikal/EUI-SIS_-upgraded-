#include "../header/Admin_Course_Withdrawal.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <iostream>

// =========================================================
// NEW HELPER: Remove from Teacher's Attendance & Grades
// =========================================================
void remove_student_from_teacher_files(const string& student_id, const string& course_code) {
    string attPath = "Databases/Courses/" + course_code + "/Attendance.csv";
    string gradesPath = "Databases/Courses/" + course_code + "/Grades.csv";

    // Reusable lambda to filter out the student from a given CSV
    auto remove_from_csv = [&](const string& path) {
        ifstream fileIn(path);
        if (!fileIn.is_open()) return;

        vector<string> lines;
        string line;
        while (getline(fileIn, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string id_cell;
            getline(ss, id_cell, ',');

            // Strip hidden characters just in case
            id_cell.erase(remove_if(id_cell.begin(), id_cell.end(), ::isspace), id_cell.end());
            string clean_target = student_id;
            clean_target.erase(remove_if(clean_target.begin(), clean_target.end(), ::isspace), clean_target.end());

            // If it's NOT the withdrawn student, keep the row
            if (id_cell != clean_target) {
                lines.push_back(line);
            }
        }
        fileIn.close();

        // Overwrite the file without the student
        ofstream fileOut(path);
        for (const auto& l : lines) {
            fileOut << l << "\n";
        }
    };

    // Execute the removal for both files
    remove_from_csv(attPath);
    remove_from_csv(gradesPath);
}

// =========================================================
// INTERNAL HELPER (Hidden from the rest of the app)
// =========================================================
void update_student_enrollment_on_disk(const string& student_id, const string& course_code) {
    ifstream fileIn("Databases/Data_on_Each_Student.csv");
    if (!fileIn.is_open()) return;

    vector<string> lines;
    string line;
    while (getline(fileIn, line)) {
        stringstream ss(line);
        string cell;
        vector<string> cols;

        // Split CSV columns
        while (getline(ss, cell, ',')) cols.push_back(cell);

        if (!cols.empty() && cols[0] == student_id) {
            string reg = cols[4]; // Column 4 is Registered Courses

            // Courses are formatted as: _CODE\\Day Time//Day Time
            // We search for the specific course block starting with '_'
            size_t start = reg.find("_" + course_code);
            if (start != string::npos) {
                // Find where the next course begins or the string ends
                size_t next_underscore = reg.find("_", start + 1);

                if (next_underscore == string::npos) {
                    reg.erase(start); // Remove until end of string
                } else {
                    reg.erase(start, next_underscore - start); // Remove specific block
                }
            }

            // Reconstruct the full CSV line
            line = "";
            for (size_t i = 0; i < cols.size(); ++i) {
                line += (i == 4 ? reg : cols[i]) + (i < cols.size() - 1 ? "," : "");
            }
        }
        lines.push_back(line);
    }
    fileIn.close();

    // Overwrite the database with updated data
    ofstream fileOut("Databases/Data_on_Each_Student.csv");
    for (const auto& l : lines) fileOut << l << "\n";
}

// =========================================================
// PUBLIC INTERFACE
// =========================================================

vector<CppWithdrawalRequest> AdminWithdrawalManager::fetch() {
    vector<CppWithdrawalRequest> reqs;
    ifstream file("Databases/Course_Withdrawals.csv");
    if (!file.is_open()) return reqs;

    string line;
    while (getline(file, line)) {
        if(line.empty()) continue;
        // Strip carriage returns for Windows compatibility
        line.erase(remove(line.begin(), line.end(), '\r'), line.end());

        stringstream ss(line);
        string id, course, reason, status;
        getline(ss, id, ',');
        getline(ss, course, ',');
        getline(ss, reason, ',');
        getline(ss, status, ',');

        if (status == "Pending") {
            reqs.push_back({id, course, reason});
        }
    }
    return reqs;
}

void AdminWithdrawalManager::process(const string& target_id, const string& target_course, bool approved) {
    // 1. Update the Request Log
    ifstream fileIn("Databases/Course_Withdrawals.csv");
    vector<string> lines;
    string line;
    while (getline(fileIn, line)) {
        if(line.empty()) continue;
        if(line.back() == '\r') line.pop_back();

        stringstream ss(line);
        string id, course, reason, status;
        getline(ss, id, ',');
        getline(ss, course, ',');
        getline(ss, reason, ',');
        getline(ss, status, ',');

        if (id == target_id && course == target_course && status == "Pending") {
            status = approved ? "Approved" : "Rejected";
            line = id + "," + course + "," + reason + "," + status;
        }
        lines.push_back(line);
    }
    fileIn.close();

    ofstream fileOut("Databases/Course_Withdrawals.csv");
    for (const auto& l : lines) fileOut << l << "\n";

    // 2. If approved, update the actual student record AND the teacher files
    if (approved) {
        update_student_enrollment_on_disk(target_id, target_course);

        // ---> THIS IS THE NEW FIX THAT UPDATES THE TEACHER <---
        remove_student_from_teacher_files(target_id, target_course);

        cout << "DEBUG: Student " << target_id << " removed from " << target_course << " registration." << endl;
    }
}