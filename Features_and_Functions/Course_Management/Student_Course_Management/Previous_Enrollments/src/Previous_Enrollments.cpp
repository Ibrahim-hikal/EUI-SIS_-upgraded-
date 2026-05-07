#include "../header/Previous_Enrollments.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <main.h>

// Logic to load data from CSV and send it to the Slint UI properties
void load_enrollment_data(auto ui_handle, std::string student_id) {
    std::ifstream file("Databases/Data_on_Each_Student.csv");
    std::string line;

    if (!file.is_open()) return;

    // Skip the header row
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string id, name, passed_str, failed_str;

        // Column mapping: 0:ID, 1:Name, 2:Passed, 3:Failed
        std::getline(ss, id, ',');
        if (id == student_id) {
            std::getline(ss, name, ',');
            std::getline(ss, passed_str, ',');
            std::getline(ss, failed_str, ',');

            // Lambda function to split course strings (handles single or multiple courses)
            auto parse_courses = [](std::string raw) {
                std::vector<CourseInfo> list;
                if (raw == "-1" || raw.empty()) return list;

                std::stringstream rss(raw);
                std::string code;
                // Splitting by '^' or ',' if multiple courses exist
                while (std::getline(rss, code, '^')) {
                    list.push_back({
                        slint::SharedString(code),           // .code
                        slint::SharedString("Course " + code) // .name
                    });
                }
                return list;
            };

            // Convert vectors to Slint Models
            auto passed_model = std::make_shared<slint::VectorModel<CourseInfo>>(parse_courses(passed_str));
            auto failed_model = std::make_shared<slint::VectorModel<CourseInfo>>(parse_courses(failed_str));

            // Push the models to the UI properties
            ui_handle->set_passed_courses(passed_model);
            ui_handle->set_failed_courses(failed_model);
            break;
        }
    }
    file.close();
}