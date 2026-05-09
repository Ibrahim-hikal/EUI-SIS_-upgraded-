/*
 * FILE: Admin_Profile.cpp
 * DESCRIPTION: Implementation of the Admin_Profile class - handles administrator operations
 *
 * PURPOSE: Provides implementation for:
 *          - Loading admin profile data from database
 *          - Resetting admin data on logout
 *          - End-of-semester processing (grade calculation, GPA updates, database cleanup)
 *
 * KEY RESPONSIBILITIES:
 *   - Load admin profile from Databases/Data_on_each_admin.csv
 *   - Process semester-end operations including grade calculations and GPA updates
 *   - Clear student registrations and requests for new semester
 *   - Handle database file locking issues (e.g., Excel open)
 *
 * DATABASE FILES USED:
 *   - Databases/Data_on_each_admin.csv: Admin profile data
 *   - Databases/Data_on_Each_Student.csv: Student records (modified during semester end)
 *   - Databases/Course_Withdrawals.csv: Cleared during semester end
 *   - Databases/Attendance_Excuses.csv: Cleared during semester end
 *   - Databases/Courses/<CourseCode>/Grades.csv: Individual course grade files (read-only)
 *
 * DEPENDENCIES: Admin_Profile.h (class definition)
 */

#include "../header/Admin_Profile.h"
// Readability pass: includes and declarations are kept visually grouped.
#include <fstream>        // For file I/O operations
#include <sstream>        // For string stream parsing (CSV processing)
#include <vector>         // For storing CSV rows and columns
#include <iostream>       // For console output and error reporting
#include <filesystem>     // For file system operations (checking course directories)
#include <iomanip>        // For precise GPA formatting (fixed, setprecision)

using namespace std;

// ==========================================
// BASIC PROFILE MANAGEMENT
// ==========================================

/*
 * FUNCTION: Admin_Profile::reset()
 * PURPOSE: Clears all admin profile data to empty/default values
 *
 * USAGE: Called when logging out or switching users to prevent data leakage
 *        between different admin sessions
 *
 * EFFECTS:
 *   - name: Set to empty string
 *   - email: Set to empty string
 *   - position: Set to empty string
 */
void Admin_Profile::reset() {
    this->name = "";
    this->email = "";
    this->position = "";
}

/*
 * FUNCTION: Admin_Profile::load_profile()
 * PURPOSE: Loads admin profile data from the database using email as key
 *
 * PROCESS:
 *   1. Reset all data to empty state
 *   2. Set email to the provided value
 *   3. Open Databases/Data_on_each_admin.csv
 *   4. Skip header row
 *   5. Search for row where column 1 (email) matches current_email
 *   6. Extract name from column 0, position from column 2
 *   7. Close file
 *
 * CSV FORMAT: name,email,position
 *   Example: "Dr. Ahmed Hassan,ahmed@eui.edu.eg,Dean"
 *
 * PARAMETERS:
 *   - current_email: Email address to search for in database
 *
 * ERROR HANDLING: Gracefully handles missing file or malformed CSV
 *                 If file not found, admin data remains empty
 */
void Admin_Profile::load_profile(const string &current_email) {
    reset();
    this->email = current_email;

    ifstream file("Databases/Data_on_each_admin.csv");
    string line, cell;
    if (file.is_open()) {
        getline(file, line);  // Skip header row
        while (getline(file, line)) {
            if (line.empty()) continue;
            if (line.back() == '\r') line.pop_back();  // Handle Windows line endings

            stringstream ss(line);
            vector<string> cols;
            while (getline(ss, cell, ',')) cols.push_back(cell);
            if (cols.size() >= 3 && cols[1] == current_email) {
                this->name = cols[0];      // Column 0: Admin name
                this->position = cols[2];  // Column 2: Admin position
                break;  // Found the admin, no need to continue searching
            }
        }
        file.close();
    }
}

// ---> CHANGED FROM VOID TO STRING <---
string Admin_Profile::end_semester() {
    // ==========================================
    // PHASE 1: OPEN STUDENT DATABASE FOR READING
    // ==========================================
    string student_db = "Databases/Data_on_Each_Student.csv";
    ifstream file_in(student_db);
    if (!file_in.is_open()) {
        return "ERROR: Could not open Databases/Data_on_Each_Student.csv";
    }

    // ==========================================
    // PHASE 2: READ AND PREPARE STUDENT DATA
    // ==========================================
    vector<string> lines;  // Will store all processed lines
    string header;
    getline(file_in, header);
    if (!header.empty() && header.back() == '\r') header.pop_back();  // Handle Windows line endings
    lines.push_back(header);  // Keep header as first line

    string line;
    // Process each student record
    while (getline(file_in, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();  // Handle Windows line endings

        // Parse CSV line into columns
        stringstream ss(line);
        string cell;
        vector<string> cols;
        while (getline(ss, cell, ',')) cols.push_back(cell);

        // Skip malformed lines (need at least 11 columns for full processing)
        if (cols.size() < 11) {
            lines.push_back(line);
            continue;
        }

        // Extract key student data
        string id = cols[0];           // Student ID
        string passed_str = cols[2];   // Previously passed courses
        string failed_str = cols[3];   // Previously failed courses
        string reg_str = cols[4];      // Currently registered courses

        // ==========================================
        // PHASE 3: PROCESS REGISTERED COURSES (IF ANY)
        // ==========================================
        // Only process students who have registered courses this semester
        if (reg_str != "-1" && !reg_str.empty()) {
            // Parse registered courses from encoded string format
            // Format: "CSE021\\Saturday 9:00-11:00//Sunday 2:00-4:00_PHM111\\..."
            vector<string> current_reg;
            stringstream reg_ss(reg_str);
            string token;
            while (getline(reg_ss, token, '_')) {  // Split by underscore
                if (token.empty()) continue;
                size_t slash = token.find('\\');   // Find course code separator
                // Extract just the course code (before backslash)
                current_reg.push_back((slash != string::npos) ? token.substr(0, slash) : token);
            }

            // ==========================================
            // PHASE 4: CALCULATE GPA FROM COURSE HISTORY
            // ==========================================
            double total_points = 0.0;  // Sum of grade points
            int total_count = 0;        // Total courses counted

            // Lambda function to count courses and add grade points
            // Used for both passed and failed course history
            auto count_history = [&](string s, double pts) {
                if (s == "-1" || s.empty()) return;  // No history to count
                stringstream ss_h(s);
                string h;
                while (getline(ss_h, h, '_')) {  // Split course history by underscore
                    if (!h.empty()) {
                        total_points += pts;  // Add the grade points for this course
                        total_count++;        // Count this course
                    }
                }
            };

            // Count previously passed courses (3.0 points each for GPA calculation)
            count_history(passed_str, 3.0);
            // Count previously failed courses (0.0 points each)
            count_history(failed_str, 0.0);

            // Prepare strings for new passed/failed courses
            string new_passed = (passed_str == "-1") ? "" : passed_str;
            string new_failed = (failed_str == "-1") ? "" : failed_str;

            // ==========================================
            // PHASE 5: PROCESS GRADES FOR EACH REGISTERED COURSE
            // ==========================================
            for (const string& course_code : current_reg) {
                // Find the grades file for this course
                string grades_path = "";
                try {
                    // Search through course directories for Grades.csv file
                    if (std::filesystem::exists("Databases/Courses")) {
                        for (const auto& entry : std::filesystem::recursive_directory_iterator("Databases/Courses")) {
                            // Look for Grades.csv in a directory named after the course code
                            if (entry.is_regular_file() &&
                                entry.path().filename() == "Grades.csv" &&
                                entry.path().parent_path().filename() == course_code) {
                                grades_path = entry.path().string();
                                break;  // Found it, stop searching
                            }
                        }
                    }
                } catch (...) {}  // Ignore filesystem errors

                // ==========================================
                // PHASE 6: LOOKUP STUDENT'S GRADE FOR THIS COURSE
                // ==========================================
                double mark = -1.0;  // Default: no grade found
                if (!grades_path.empty()) {
                    ifstream g_file(grades_path);
                    string g_line;
                    getline(g_file, g_line);  // Skip header
                    while (getline(g_file, g_line)) {
                        if (g_line.empty()) continue;
                        if (g_line.back() == '\r') g_line.pop_back();

                        stringstream g_ss(g_line);
                        string g_cell;
                        vector<string> g_cols;
                        while (getline(g_ss, g_cell, ',')) g_cols.push_back(g_cell);

                        // Find row for this student (column 0 = student ID)
                        if (!g_cols.empty() && g_cols[0] == id) {
                            // Column 8 contains the final grade
                            if (g_cols.size() >= 9 && !g_cols[8].empty()) {
                                try {
                                    mark = stod(g_cols[8]);  // Convert string to double
                                } catch(...) {
                                    mark = -1.0;  // Invalid grade format
                                }
                            }
                            break;  // Found student's grade, stop searching
                        }
                    }
                }

                // ==========================================
                // PHASE 7: PROCESS GRADE AND UPDATE STUDENT RECORD
                // ==========================================
                total_count++;  // Count this course toward GPA

                if (mark >= 60.0) {
                    // STUDENT PASSED THE COURSE
                    // Add course to passed list
                    if (!new_passed.empty()) new_passed += "_";
                    new_passed += course_code;

                    // Convert numerical grade to GPA points (4.0 scale)
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
                    // STUDENT FAILED THE COURSE
                    // Add course to failed list (no GPA points)
                    if (!new_failed.empty()) new_failed += "_";
                    new_failed += course_code;
                    // total_points unchanged (0.0 for failed courses)
                }
            }

            // ==========================================
            // PHASE 8: UPDATE STUDENT RECORD WITH NEW DATA
            // ==========================================
            // Update passed courses column
            cols[2] = new_passed.empty() ? "-1" : new_passed;
            // Update failed courses column
            cols[3] = new_failed.empty() ? "-1" : new_failed;

            // Calculate and update final GPA
            double final_gpa = (total_count > 0) ? (total_points / total_count) : 0.0;
            stringstream gpa_ss;
            gpa_ss << fixed << setprecision(2) << final_gpa;  // Format as X.XX
            cols[10] = gpa_ss.str();  // Column 10 = GPA
        }

        // ==========================================
        // PHASE 9: CLEAR REGISTRATIONS AND REQUESTS FOR NEW SEMESTER
        // ==========================================
        cols[4] = "-1"; // Clear Registered courses
        cols[5] = "-1"; // Clear Requested courses
        cols[6] = "";   // Clear Excuse Requests
        cols[7] = "0";  // Clear Total Excuses Accepted
        cols[8] = "";   // Clear Withdrawal Requests

        // Reconstruct the CSV line with updated data
        string updated_line = "";
        for (size_t i = 0; i < cols.size(); ++i) {
            updated_line += cols[i] + (i == cols.size() - 1 ? "" : ",");
        }
        lines.push_back(updated_line);
    }
    file_in.close();

    // ==========================================
    // PHASE 10: WRITE UPDATED STUDENT DATA BACK TO FILE
    // ==========================================
    // Check for file locking (e.g., Excel has file open)
    ofstream file_out(student_db, ios::trunc);
    if (!file_out.is_open()) {
        return "ERROR: Database locked! Close your CSV files in Excel and try again.";
    }

    // Write all lines back to file
    for (const auto& l : lines) file_out << l << "\n";
    file_out.close();

    // ==========================================
    // PHASE 11: CLEAR REQUEST DATABASES FOR NEW SEMESTER
    // ==========================================
    try {
        // Clear course withdrawal requests
        ofstream wOut("Databases/Course_Withdrawals.csv", ios::trunc);
        if (wOut.is_open()) wOut.close();

        // Clear attendance excuse requests
        ofstream eOut("Databases/Attendance_Excuses.csv", ios::trunc);
        if (eOut.is_open()) eOut.close();
    } catch (...) {}  // Ignore errors if files can't be cleared

    // ==========================================
    // PHASE 12: RETURN SUCCESS MESSAGE
    // ==========================================
    return "Semester Ended Successfully! Databases cleared for the new term.";
}