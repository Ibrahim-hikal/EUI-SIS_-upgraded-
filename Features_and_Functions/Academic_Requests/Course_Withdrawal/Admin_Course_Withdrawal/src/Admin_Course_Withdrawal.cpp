/*
 * File: Admin_Course_Withdrawal.cpp
 * Description: Does the actual work of reading the pending requests and modifying
 * the student database if the administrator approves a class drop.
 */

#include "../header/Admin_Course_Withdrawal.h"
// Readability pass: includes and declarations are kept visually grouped.
#include <fstream>
#include <sstream>

using namespace std;

// Grabs all unhandled requests from the database
vector<WithdrawalData> AdminWithdrawalManager::fetch() {
    vector<WithdrawalData> requests; // Updated to use the new name
    ifstream file("Databases/Course_Withdrawals.csv");
    string line, cell;

    if (file.is_open()) {
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            vector<string> cols;

            while (getline(ss, cell, ',')) {
                cols.push_back(cell);
            }

            // If the row has an ID, a Course, and a Reason, add it to our list
            if (cols.size() >= 3) {
                requests.push_back({cols[0], cols[1], cols[2]});
            }
        }
        file.close();
    }
    return requests;
}

// Applies the admin's decision
void AdminWithdrawalManager::process(const string& student_id, const string& course_code, bool approved) {

    // STEP 1: Remove the request from the pending list
    vector<string> remaining_requests;
    ifstream file_in("Databases/Course_Withdrawals.csv");
    string line;

    if (file_in.is_open()) {
        while (getline(file_in, line)) {
            if (line.empty()) continue;
            // Only keep requests that do NOT match the one we are currently processing
            if (line.find(student_id + "," + course_code) == string::npos) {
                remaining_requests.push_back(line);
            }
        }
        file_in.close();
    }

    // Overwrite the file with the remaining requests
    ofstream file_out("Databases/Course_Withdrawals.csv", ios::trunc);
    for (const string& r : remaining_requests) {
        file_out << r << "\n";
    }
    file_out.close();

    // STEP 2: If the admin said NO, we stop here. If they said YES, we update the student.
    if (!approved) return;

    ifstream student_in("Databases/Data_on_Each_Student.csv");
    vector<string> student_lines;

    if (student_in.is_open()) {
        while (getline(student_in, line)) {
            if (line.empty()) continue;

            stringstream ss(line);
            string cell;
            vector<string> cols;
            while (getline(ss, cell, ',')) cols.push_back(cell);

            // Find the specific student in the database
            if (cols.size() >= 5 && cols[0] == student_id) {
                string courses = cols[4];
                string updated_courses = "";

                // Break their course list apart and rebuild it without the dropped class
                stringstream css(courses);
                string crs;
                while (getline(css, crs, '_')) {
                    if (crs.empty()) continue;

                    // Keep the course if it is NOT the one being dropped
                    if (crs.find(course_code) == string::npos) {
                        if (!updated_courses.empty()) updated_courses += "_";
                        updated_courses += crs;
                    }
                }

                // If they dropped their only class, mark it as empty (-1)
                if (updated_courses.empty()) updated_courses = "-1";
                cols[4] = updated_courses;

                // Rebuild the student's database row with the new course list
                line = "";
                for (size_t i = 0; i < cols.size(); ++i) {
                    line += cols[i] + (i == cols.size() - 1 ? "" : ",");
                }
            }
            student_lines.push_back(line);
        }
        student_in.close();
    }

    // Save the updated student records back to the database
    ofstream student_out("Databases/Data_on_Each_Student.csv", ios::trunc);
    for (const string& l : student_lines) {
        student_out << l << "\n";
    }
    student_out.close();
}