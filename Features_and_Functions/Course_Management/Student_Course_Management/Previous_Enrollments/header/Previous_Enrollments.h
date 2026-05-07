#pragma once
#include <string>
#include <vector>
#include "main.h"

class PreviousEnrollments {
public:
    PreviousEnrollments(const std::string& csv_file);
    void load_student_data(Main_App* ui, const std::string& student_id);

private:
    std::string csv_file_path;
    std::vector<std::string> getCourses(const std::string& course_str);
};