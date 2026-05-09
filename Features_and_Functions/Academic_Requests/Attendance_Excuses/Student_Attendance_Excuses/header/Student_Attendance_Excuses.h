#pragma once
// Readability note: declarations are grouped for easier scanning.

#include <string>
class StudentExcuseManager {
public:
    static void submit(const std::string& id, const std::string& course, const std::string& week, const std::string& reason);
};