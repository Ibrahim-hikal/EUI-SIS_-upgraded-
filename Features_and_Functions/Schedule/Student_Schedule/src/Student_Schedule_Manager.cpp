#include "../header/Student_Schedule_Manager.h"
// Readability pass: includes and declarations are kept visually grouped.
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <functional> // ADDED: For std::hash

StudentScheduleManager::StudentScheduleManager() {}

std::vector<std::string> StudentScheduleManager::split_csv_line(const std::string& line) {
    std::vector<std::string> result;
    std::string current;
    bool in_quotes = false;
    for (char c : line) {
        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (c == ',' && !in_quotes) {
            result.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    result.push_back(current);
    return result;
}

int StudentScheduleManager::map_day_to_index(const std::string& day_str) {
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

std::string StudentScheduleManager::assign_color_for_course(const std::string& course_code) {
    // A palette of 9 distinct pastel colors
    std::vector<std::string> palette = {
        "#e8f5e9", // Green
        "#e3f2fd", // Blue
        "#fff3e0", // Orange
        "#fce4ec", // Pink
        "#f3e5f5", // Purple
        "#e0f7fa", // Cyan
        "#f0f4c3", // Lime
        "#ffe0b2", // Deep Orange
        "#ffcdd2"  // Red
    };

    // Hash the course code string to consistently pick the same color for the same course
    size_t hash_val = std::hash<std::string>{}(course_code);
    return palette[hash_val % palette.size()];
}

void StudentScheduleManager::parse_and_add_course(const std::string& raw_course_string) {
    std::cout << "\n--- DEBUG: Parsing Courses ---" << std::endl;
    std::cout << "Raw string from CSV: [" << raw_course_string << "]" << std::endl;

    std::stringstream ss(raw_course_string);
    std::string course_block;

    while (std::getline(ss, course_block, '_')) {
        if (course_block.empty()) continue;

        std::cout << "  -> Processing course block: [" << course_block << "]" << std::endl;

        size_t backslash_pos = course_block.find('\\');
        if (backslash_pos == std::string::npos) continue;

        size_t times_start = backslash_pos;
        while (times_start < course_block.length() && course_block[times_start] == '\\') {
            times_start++;
        }

        std::string code = course_block.substr(0, backslash_pos);
        std::string all_times = course_block.substr(times_start);

        std::string delimiter = "//";
        size_t pos = 0;
        std::vector<std::string> time_slots;
        while ((pos = all_times.find(delimiter)) != std::string::npos) {
            time_slots.push_back(all_times.substr(0, pos));
            all_times.erase(0, pos + delimiter.length());
        }
        if (!all_times.empty()) {
            time_slots.push_back(all_times);
        }

        std::regex time_regex(R"(([A-Za-z]+)\s+(\d{1,2}):(\d{2})\s*-\s*(\d{1,2}):(\d{2}))");

        int slot_index = 0; // Track if it's the first slot (Lecture) or subsequent (Tutorial)

        for (const std::string& ts : time_slots) {
            std::smatch match;
            if (std::regex_search(ts, match, time_regex) && match.size() >= 6) {
                std::string day_str = match[1].str();

                float start_h = std::stof(match[2].str()) + (std::stof(match[3].str()) / 60.0f);
                float end_h = std::stof(match[4].str()) + (std::stof(match[5].str()) / 60.0f);

                ScheduleEntry entry;
                entry.course_code = code;
                entry.course_name = "";
                entry.slot_type = (slot_index == 0) ? "Lecture" : "Tutorial"; // Assign type based on position
                entry.day_index = map_day_to_index(day_str);
                entry.start_hour = start_h;
                entry.duration_hours = end_h - start_h;
                entry.color_hex = assign_color_for_course(entry.course_code);

                current_schedule.push_back(entry);
            }
            slot_index++; // Increment for the next loop iteration
        }
    }
}

void StudentScheduleManager::load_student_schedule(const std::string& student_id) {
    std::cout << "\n--- DEBUG: Loading Schedule for ID: " << student_id << " ---" << std::endl;
    current_student_id = student_id;
    current_schedule.clear();

    std::ifstream file("Databases/Data_on_Each_Student.csv");
    if (!file.is_open()) {
        std::cerr << "CRITICAL ERROR: Failed to open Data_on_Each_Student.csv." << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line);
    std::vector<std::string> headers = split_csv_line(line);

    int reg_col_idx = -1;
    for (int i = 0; i < headers.size(); ++i) {
        if (headers[i].find("Registered Courses") != std::string::npos ||
            headers[i].find("Schedule") != std::string::npos) {
            reg_col_idx = i;
            break;
        }
    }

    if (reg_col_idx == -1) return;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::vector<std::string> cols = split_csv_line(line);
        if (!cols.empty() && cols[0] == student_id) {
            if (reg_col_idx < cols.size()) {
                parse_and_add_course(cols[reg_col_idx]);
            }
            break;
        }
    }
}

void StudentScheduleManager::initUI(Main_App* ui) {
    std::vector<CourseSlot> slint_slots;
    for (const auto& entry : current_schedule) {
        CourseSlot slot;
        slot.course_code = slint::SharedString(entry.course_code);
        slot.course_name = slint::SharedString(entry.course_name);
        slot.slot_type = slint::SharedString(entry.slot_type); // Pass type to UI
        slot.day_index = entry.day_index;
        slot.start_hour = entry.start_hour;
        slot.duration_hours = entry.duration_hours;

        uint32_t hex_val = std::stoul(entry.color_hex.substr(1), nullptr, 16);
        uint8_t r = (hex_val >> 16) & 0xFF;
        uint8_t g = (hex_val >> 8) & 0xFF;
        uint8_t b = hex_val & 0xFF;
        slot.bg_color = slint::Color::from_rgb_uint8(r, g, b);

        slint_slots.push_back(slot);
    }
    auto schedule_model = std::make_shared<slint::VectorModel<CourseSlot>>(slint_slots);
    ui->set_student_schedule_data(schedule_model);
}