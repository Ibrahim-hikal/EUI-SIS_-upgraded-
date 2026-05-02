#include "../header/Admin_Course_Withdrawal.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <iostream>

// =========================================================
// INTERNAL HELPER (Hidden from the rest of the app)
// =========================================================
void update_student_enrollment_on_disk(const std::string& student_id, const std::string& course_code) {
    std::ifstream fileIn("Databases/Data_on_Each_Student.csv");
    if (!fileIn.is_open()) return;

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(fileIn, line)) {
        std::stringstream ss(line);
        std::string cell;
        std::vector<std::string> cols;

        // Split CSV columns[cite: 3]
        while (std::getline(ss, cell, ',')) cols.push_back(cell);

        if (!cols.empty() && cols[0] == student_id) {
            std::string reg = cols[4]; // Column 4 is Registered Courses[cite: 3]

            // Courses are formatted as: _CODE\\Day Time//Day Time
            // We search for the specific course block starting with '_'
            size_t start = reg.find("_" + course_code);
            if (start != std::string::npos) {
                // Find where the next course begins or the string ends
                size_t next_underscore = reg.find("_", start + 1);

                if (next_underscore == std::string::npos) {
                    reg.erase(start); // Remove until end of string
                } else {
                    reg.erase(start, next_underscore - start); // Remove specific block
                }
            }

            // Reconstruct the full CSV line[cite: 3]
            line = "";
            for (size_t i = 0; i < cols.size(); ++i) {
                line += (i == 4 ? reg : cols[i]) + (i < cols.size() - 1 ? "," : "");
            }
        }
        lines.push_back(line);
    }
    fileIn.close();

    // Overwrite the database with updated data[cite: 3]
    std::ofstream fileOut("Databases/Data_on_Each_Student.csv");
    for (const auto& l : lines) fileOut << l << "\n";
}

// =========================================================
// PUBLIC INTERFACE
// =========================================================

std::vector<CppWithdrawalRequest> AdminWithdrawalManager::fetch() {
    std::vector<CppWithdrawalRequest> reqs;
    std::ifstream file("Databases/Course_Withdrawals.csv");
    if (!file.is_open()) return reqs;

    std::string line;
    while (std::getline(file, line)) {
        if(line.empty()) continue;
        // Strip carriage returns for Windows compatibility
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        std::stringstream ss(line);
        std::string id, course, reason, status;
        std::getline(ss, id, ',');
        std::getline(ss, course, ',');
        std::getline(ss, reason, ',');
        std::getline(ss, status, ',');

        if (status == "Pending") {
            reqs.push_back({id, course, reason});
        }
    }
    return reqs;
}

void AdminWithdrawalManager::process(const std::string& target_id, const std::string& target_course, bool approved) {
    // 1. Update the Request Log
    std::ifstream fileIn("Databases/Course_Withdrawals.csv");
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(fileIn, line)) {
        if(line.empty()) continue;
        if(line.back() == '\r') line.pop_back();

        std::stringstream ss(line);
        std::string id, course, reason, status;
        std::getline(ss, id, ',');
        std::getline(ss, course, ',');
        std::getline(ss, reason, ',');
        std::getline(ss, status, ',');

        if (id == target_id && course == target_course && status == "Pending") {
            status = approved ? "Approved" : "Rejected";
            line = id + "," + course + "," + reason + "," + status;
        }
        lines.push_back(line);
    }
    fileIn.close();

    std::ofstream fileOut("Databases/Course_Withdrawals.csv");
    for (const auto& l : lines) fileOut << l << "\n";

    // 2. If approved, update the actual student record[cite: 3]
    if (approved) {
        update_student_enrollment_on_disk(target_id, target_course);
        std::cout << "DEBUG: Student " << target_id << " removed from " << target_course << " registration." << std::endl;
    }
}