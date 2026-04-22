#ifndef PROJECT_TEACHER_ATTENDANCE_H
#define PROJECT_TEACHER_ATTENDANCE_H

#include <string>
#include <vector>
#include <slint.h>
#include "main.h" // Required to access Main_App UI methods

struct StudentAttendanceData;

class TeacherAttendanceManager {
private:
    std::string teacher_name;
    std::vector<std::string> parse_csv_line(const std::string& line) const;

public:
    TeacherAttendanceManager() = default; // Default constructor

    // New method to encapsulate all UI logic
    void initUI(Main_App* ui, const std::string& name);

    slint::SharedVector<slint::SharedString> get_available_courses() const;
    slint::SharedVector<StudentAttendanceData> get_students_for_week(const std::string& course_code, int week) const;
    bool save_attendance(const std::string& course_code, int week, const slint::SharedVector<StudentAttendanceData>& records) const;
};
#endif //PROJECT_TEACHER_ATTENDANCE_H