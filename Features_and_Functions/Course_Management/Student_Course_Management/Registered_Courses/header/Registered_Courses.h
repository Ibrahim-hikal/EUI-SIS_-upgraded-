#ifndef PROJECT_REGISTERED_COURSES_H
#define PROJECT_REGISTERED_COURSES_H

#include <string>
#include <vector>
#include <slint.h>

class CourseInfo; // From Slint

class RegisteredCoursesManager {
private:
    std::string student_id;
    std::vector<std::string> parse_csv_line(const std::string& line) const;
    std::string calculate_total_attendance(const std::string& course_code) const;

public:
    explicit RegisteredCoursesManager(std::string id);
    slint::SharedVector<CourseInfo> get_registered_courses_with_attendance() const;
};

#endif //PROJECT_REGISTERED_COURSES_H