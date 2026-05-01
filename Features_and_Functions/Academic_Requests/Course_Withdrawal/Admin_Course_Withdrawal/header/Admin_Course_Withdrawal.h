#pragma once
#include <string>
#include <vector>

// Pure C++ Struct - NO SLINT INCLUDES NEEDED HERE
struct CppWithdrawalRequest {
    std::string student_id;
    std::string course_code;
    std::string reason;
};

class AdminWithdrawalManager {
public:
    static std::vector<CppWithdrawalRequest> fetch();
    static void process(const std::string& target_id, const std::string& target_course, bool approved);
};