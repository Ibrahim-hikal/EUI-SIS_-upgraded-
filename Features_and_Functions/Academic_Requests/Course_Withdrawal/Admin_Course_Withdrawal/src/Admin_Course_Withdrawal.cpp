#include "../header/Admin_Course_Withdrawal.h"
#include <fstream>
#include <sstream>
#include <algorithm>

std::vector<CppWithdrawalRequest> AdminWithdrawalManager::fetch() {
    std::vector<CppWithdrawalRequest> reqs;
    std::ifstream file("Databases/Course_Withdrawals.csv");
    if (!file.is_open()) return reqs;

    std::string line;
    while (std::getline(file, line)) {
        if(line.empty()) continue;
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        std::stringstream ss(line);
        std::string id, course, reason, status;
        std::getline(ss, id, ','); std::getline(ss, course, ',');
        std::getline(ss, reason, ','); std::getline(ss, status, ',');

        if (status == "Pending") {
            reqs.push_back({id, course, reason});
        }
    }
    return reqs;
}

void AdminWithdrawalManager::process(const std::string& target_id, const std::string& target_course, bool approved) {
    std::ifstream fileIn("Databases/Course_Withdrawals.csv");
    std::vector<std::string> lines; std::string line;

    while (std::getline(fileIn, line)) {
        if(line.empty()) continue;
        bool has_cr = (line.back() == '\r'); if(has_cr) line.pop_back();

        std::stringstream ss(line);
        std::string id, course, reason, status;
        std::getline(ss, id, ','); std::getline(ss, course, ',');
        std::getline(ss, reason, ','); std::getline(ss, status, ',');

        if (id == target_id && course == target_course && status == "Pending") {
            status = approved ? "Approved" : "Rejected";
            line = id + "," + course + "," + reason + "," + status;
        }
        lines.push_back(line);
    }
    std::ofstream fileOut("Databases/Course_Withdrawals.csv");
    for (const auto& l : lines) fileOut << l << "\n";
}