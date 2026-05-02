#ifndef PROJECT_TEACHER_SCHEDULE_MANAGER_H
#define PROJECT_TEACHER_SCHEDULE_MANAGER_H

#include <string>
#include <vector>
#include "main.h"


struct TeacherScheduleEntry {
    std::string course_code;
    std::string slot_type; // Lecture or Tutorial
    int day_index;
    float start_hour;
    float duration_hours;
    std::string color_hex;
};

class TeacherScheduleManager {
public:
    TeacherScheduleManager();
    void load_teacher_schedule(const std::string& teacher_name);
    void initUI(Main_App* ui);

private:
    std::string current_teacher_name;
    std::vector<TeacherScheduleEntry> current_schedule;

    int map_day_to_index(const std::string& day_str);
    std::string assign_color_for_course(const std::string& course_code);
    std::vector<std::string> split_csv_line(const std::string& line);
    void parse_slots(const std::string& code, const std::string& slots_string, const std::string& slot_type);
};
#endif //PROJECT_TEACHER_SCHEDULE_MANAGER_H