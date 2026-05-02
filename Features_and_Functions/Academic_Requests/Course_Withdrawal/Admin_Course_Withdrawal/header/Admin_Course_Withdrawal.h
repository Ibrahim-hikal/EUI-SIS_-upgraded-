#pragma once
#include <string>
#include <vector>

struct CppWithdrawalRequest {
    std::string student_id;
    std::string course_code;
    std::string reason;
};

class AdminWithdrawalManager {
public:
    static std::vector<CppWithdrawalRequest> fetch();
    // This will now handle BOTH updating the request status AND removing the course enrollment
    static void process(const std::string& target_id, const std::string& target_course, bool approved);
};