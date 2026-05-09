#include "../header/Teacher_Schedule_Manager.h"
// Readability pass: includes and declarations are kept visually grouped.
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <functional>

using namespace std;

// ---> THE ULTRA-ROBUST MATCHER <---
static inline bool is_exact_match(string a, string b) {
    a.erase(remove_if(a.begin(), a.end(), [](unsigned char c){ return isspace(c) || c == '\r' || c == '\n'; }), a.end());
    b.erase(remove_if(b.begin(), b.end(), [](unsigned char c){ return isspace(c) || c == '\r' || c == '\n'; }), b.end());
    transform(a.begin(), a.end(), a.begin(), [](unsigned char c){ return tolower(c); });
    transform(b.begin(), b.end(), b.begin(), [](unsigned char c){ return tolower(c); });
    return a == b;
}

static inline string trim_edges(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

TeacherScheduleManager::TeacherScheduleManager() {}

std::vector<std::string> TeacherScheduleManager::split_csv_line(const std::string& line) {
    std::vector<std::string> result; std::string current; bool in_quotes = false;
    for (char c : line) {
        if (c == '"') { in_quotes = !in_quotes; }
        else if (c == ',' && !in_quotes) { result.push_back(trim_edges(current)); current.clear(); }
        else { current += c; }
    }
    result.push_back(trim_edges(current)); return result;
}

int TeacherScheduleManager::map_day_to_index(const std::string& day_str) {
    std::string d = day_str;
    if (d.find("Sat") != std::string::npos) return 0;
    if (d.find("Sun") != std::string::npos) return 1;
    if (d.find("Mon") != std::string::npos) return 2;
    if (d.find("Tue") != std::string::npos) return 3;
    if (d.find("Wed") != std::string::npos) return 4;
    if (d.find("Thu") != std::string::npos) return 5;
    if (d.find("Fri") != std::string::npos) return 6;
    return -1;
}

std::string TeacherScheduleManager::assign_color_for_course(const std::string& course_code) {
    std::vector<std::string> palette = { "#e8f5e9", "#e3f2fd", "#fff3e0", "#fce4ec", "#f3e5f5", "#e0f7fa", "#f0f4c3", "#ffe0b2", "#ffcdd2" };
    size_t hash_val = std::hash<std::string>{}(course_code);
    return palette[hash_val % palette.size()];
}

void TeacherScheduleManager::parse_slots(const std::string& code, const std::string& slots_string, const std::string& slot_type) {
    std::regex time_regex(R"(([A-Za-z]+)\s+(\d{1,2}):(\d{2})\s*-\s*(\d{1,2}):(\d{2}))");
    auto words_begin = std::sregex_iterator(slots_string.begin(), slots_string.end(), time_regex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string day_str = match[1].str();
        float start_h = std::stof(match[2].str()) + (std::stof(match[3].str()) / 60.0f);
        float end_h = std::stof(match[4].str()) + (std::stof(match[5].str()) / 60.0f);

        TeacherScheduleEntry entry;
        entry.course_code = code; entry.slot_type = slot_type; entry.day_index = map_day_to_index(day_str);
        entry.start_hour = start_h; entry.duration_hours = end_h - start_h; entry.color_hex = assign_color_for_course(code);
        current_schedule.push_back(entry);
    }
}

void TeacherScheduleManager::load_teacher_schedule(const std::string& teacher_name) {
    current_teacher_name = trim_edges(teacher_name);
    current_schedule.clear();

    std::ifstream file("Databases/Offered_Courses.csv");
    if (!file.is_open()) return;

    std::string line; std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::vector<std::string> cols = split_csv_line(line);

        if (cols.size() >= 7) {
            std::string code = cols[0];
            std::string instructor = cols[4];

            // USE THE ROBUST MATCHER!
            if (is_exact_match(instructor, current_teacher_name)) {
                parse_slots(code, cols[5], "Lecture");
                parse_slots(code, cols[6], "Tutorial");
            }
        }
    }
}

void TeacherScheduleManager::initUI(Main_App* ui) {
    std::vector<CourseSlot> slint_slots;
    for (const auto& entry : current_schedule) {
        CourseSlot slot;
        slot.course_code = slint::SharedString(entry.course_code); slot.course_name = slint::SharedString("");
        slot.slot_type = slint::SharedString(entry.slot_type); slot.day_index = entry.day_index;
        slot.start_hour = entry.start_hour; slot.duration_hours = entry.duration_hours;
        uint32_t hex_val = std::stoul(entry.color_hex.substr(1), nullptr, 16);
        slot.bg_color = slint::Color::from_rgb_uint8((hex_val >> 16) & 0xFF, (hex_val >> 8) & 0xFF, hex_val & 0xFF);
        slint_slots.push_back(slot);
    }
    auto schedule_model = std::make_shared<slint::VectorModel<CourseSlot>>(slint_slots);
    ui->set_teacher_schedule_data(schedule_model);
}