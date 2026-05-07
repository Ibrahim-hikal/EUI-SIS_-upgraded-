#include "../header/Previous_Enrollments.h"
#include <fstream>
#include <sstream>
#include <iostream>

PreviousEnrollments::PreviousEnrollments(const std::string& csv_file) : csv_file_path("Databases/Data_on_Each_Student.csv") {}

std::vector<std::string> PreviousEnrollments::getCourses(const std::string& course_str) {
    if (course_str == "-1" || course_str.empty()) return {};
    std::vector<std::string> courses;
    courses.push_back(course_str);
    return courses;
}

void PreviousEnrollments::load_student_data(Main_App* ui, const std::string& student_id) {
    std::ifstream file(csv_file_path);
    std::string line;
    std::vector<std::string> passed, failed;

    if (std::getline(file, line)) {
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string id, name, passed_str, failed_str;

            std::getline(ss, id, ','); std::getline(ss, name, ',');
            std::getline(ss, passed_str, ','); std::getline(ss, failed_str, ',');

            if (id == student_id) {
                passed = getCourses(passed_str);
                failed = getCourses(failed_str);
                break;
            }
        }
    }

    auto passed_model = std::make_shared<slint::VectorModel<CourseInfo>>();
    for (const auto& c : passed) {
        CourseInfo info; info.course_code = slint::SharedString(c);
        passed_model->push_back(info);
    }

    auto failed_model = std::make_shared<slint::VectorModel<CourseInfo>>();
    for (const auto& c : failed) {
        CourseInfo info; info.course_code = slint::SharedString(c);
        failed_model->push_back(info);
    }

    ui->set_passed_courses(passed_model);
    ui->set_failed_courses(failed_model);
}