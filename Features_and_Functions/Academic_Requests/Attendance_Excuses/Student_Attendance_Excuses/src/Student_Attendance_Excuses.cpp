#include "../header/Student_Attendance_Excuses.h"
// Readability pass: includes and declarations are kept visually grouped.
#include <fstream>
#include <filesystem>
void StudentExcuseManager::submit(const std::string& id, const std::string& course, const std::string& week, const std::string& reason) {
    std::filesystem::create_directories("Databases");
    std::ofstream file("Databases/Attendance_Excuses.csv", std::ios::app);
    if(file.is_open()) file << id << "," << course << "," << week << "," << reason << ",Pending\n";
}