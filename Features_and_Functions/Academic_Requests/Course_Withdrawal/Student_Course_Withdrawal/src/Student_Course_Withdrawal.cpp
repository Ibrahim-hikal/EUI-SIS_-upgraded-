#include "../header/Student_Course_Withdrawal.h"
#include <fstream>
#include <filesystem>
void StudentWithdrawalManager::submit(const std::string& id, const std::string& course, const std::string& reason) {
    std::filesystem::create_directories("Databases");
    std::ofstream file("Databases/Course_Withdrawals.csv", std::ios::app);
    if(file.is_open()) file << id << "," << course << "," << reason << ",Pending\n";
}