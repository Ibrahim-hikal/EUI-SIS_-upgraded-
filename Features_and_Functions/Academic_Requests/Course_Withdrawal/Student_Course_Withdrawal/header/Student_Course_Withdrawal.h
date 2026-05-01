#pragma once
#include <string>

class StudentWithdrawalManager {
public:
    static void submit(const std::string& id, const std::string& course, const std::string& reason);
};