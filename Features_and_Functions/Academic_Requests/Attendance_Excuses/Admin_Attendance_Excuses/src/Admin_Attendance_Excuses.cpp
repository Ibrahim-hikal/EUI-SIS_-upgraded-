#include "../header/Admin_Attendance_Excuses.h"
// Readability pass: includes and declarations are kept visually grouped.
#include <fstream>
#include <sstream>
#include <algorithm>

std::vector<CppExcuseRequest> AdminExcuseManager::fetch() {
    std::vector<CppExcuseRequest> reqs;
    std::ifstream file("Databases/Attendance_Excuses.csv");
    if (!file.is_open()) return reqs;

    std::string line;
    while (std::getline(file, line)) {
        if(line.empty()) continue;
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        std::stringstream ss(line);
        std::string id, course, week, reason, status;
        std::getline(ss, id, ','); std::getline(ss, course, ',');
        std::getline(ss, week, ','); std::getline(ss, reason, ',');
        std::getline(ss, status, ',');

        if (status == "Pending") {
            reqs.push_back({id, course, week, reason});
        }
    }
    return reqs;
}

void AdminExcuseManager::process(const std::string& target_id, const std::string& target_course, const std::string& target_week, bool approved) {
    std::ifstream fileIn("Databases/Attendance_Excuses.csv");
    std::vector<std::string> lines; std::string line;

    while (std::getline(fileIn, line)) {
        if(line.empty()) continue;
        bool has_cr = (line.back() == '\r'); if(has_cr) line.pop_back();

        std::stringstream ss(line);
        std::string id, course, week, reason, status;
        std::getline(ss, id, ','); std::getline(ss, course, ',');
        std::getline(ss, week, ','); std::getline(ss, reason, ',');
        std::getline(ss, status, ',');

        if (id == target_id && course == target_course && week == target_week && status == "Pending") {
            status = approved ? "Approved" : "Rejected";
            line = id + "," + course + "," + week + "," + reason + "," + status;
        }
        lines.push_back(line);
    }
    std::ofstream fileOut("Databases/Attendance_Excuses.csv");
    for (const auto& l : lines) fileOut << l << "\n";
}