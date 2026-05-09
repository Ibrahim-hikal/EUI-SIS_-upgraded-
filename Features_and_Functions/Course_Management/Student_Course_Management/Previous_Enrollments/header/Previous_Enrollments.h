#pragma once
// Readability note: data-facing declarations are grouped together.

#include <string>
#include <vector>
#include <slint.h>

class Main_App; // Forward declare to pass the UI pointer

class PreviousEnrollmentsManager {
private:
    std::string student_id;
    std::vector<std::string> parse_csv_line(const std::string& line) const;
    std::vector<std::string> split_courses(const std::string& course_str) const;
    void calculate_total_grade(const std::string& course_code, std::string& out_details, bool& out_has_grades) const;
public:
    explicit PreviousEnrollmentsManager(std::string id);
    void load_student_data(Main_App* ui);
};