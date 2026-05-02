#ifndef PROJECT_STUDENT_SCHEDULE_H
#define PROJECT_STUDENT_SCHEDULE_H

#include <string>
#include <vector>
#include "main.h"

struct ScheduleEntry {
    std::string course_code;
    std::string course_name;
    std::string slot_type; // ADDED: Lecture or Tutorial
    int day_index;
    float start_hour;
    float duration_hours;
    std::string color_hex;
};

class StudentScheduleManager {
public:
    StudentScheduleManager();
    void load_student_schedule(const std::string& student_id);
    void initUI(Main_App* ui);

private:
    std::string current_student_id;
    std::vector<ScheduleEntry> current_schedule;

    int map_day_to_index(const std::string& day_str);
    std::string assign_color_for_course(const std::string& course_code);
    void parse_and_add_course(const std::string& raw_course_string);
    std::vector<std::string> split_csv_line(const std::string& line);
};

#endif //PROJECT_STUDENT_SCHEDULE_H