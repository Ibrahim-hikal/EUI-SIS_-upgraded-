#pragma once
#include <string>
#include <vector>

// Pure C++ Struct - NO SLINT INCLUDES NEEDED HERE
struct CppExcuseRequest {
    std::string student_id;
    std::string course_code;
    std::string week;
    std::string reason;
};

class AdminExcuseManager {
public:
    static std::vector<CppExcuseRequest> fetch();
    static void process(const std::string& target_id, const std::string& target_course, const std::string& target_week, bool approved);
};