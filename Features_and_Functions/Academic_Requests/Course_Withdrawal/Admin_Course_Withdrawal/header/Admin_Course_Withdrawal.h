#pragma once
#include <string>
#include <vector>
#include <iostream>
    using namespace std;
struct CppWithdrawalRequest {
    string student_id;
    string course_code;
    string reason;
};

class AdminWithdrawalManager {
public:
    static vector<CppWithdrawalRequest> fetch();
    // This will now handle BOTH updating the request status AND removing the course enrollment
    static void process(const string& target_id, const string& target_course, bool approved);
};