//
// Created by ahmed on 4/25/2026.
//

#ifndef PROJECT_TEACHER_GRADES_H
#define PROJECT_TEACHER_GRADES_H

// Readability note: declarations are grouped for easier scanning.
#include <string>
#include <vector>
#include "main.h"

class TeacherGradesManager {
public:
    void initUI(Main_App* ui, const std::string& name);

private:
    std::string teacher_name;
    std::vector<std::string> parse_csv_line(const std::string& line) const;
    slint::SharedVector<StudentGradeData> get_grades_for_course(const std::string& course_code) const;
    bool save_grades(const std::string& course_code, const slint::SharedVector<StudentGradeData>& records) const;
};
#endif //PROJECT_TEACHER_GRADES_H