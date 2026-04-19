#include <iostream>
#include <fstream>
#include <sstream>
#include "../header/Accept_and_Refuse_Courses.h"
#include "main.h" // Include Slint generated header to cast void* to Main_App*

using namespace std;

// --- Constructor ---
AdminCourseManager::AdminCourseManager(const string& basePath) : dbBasePath(basePath), currentStudentIdx(-1) {}

// --- Private Utility Methods ---

string AdminCourseManager::trim(const string& s) {
    size_t first = s.find_first_not_of(" \t\r\n");
    if (string::npos == first) return s;
    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, (last - first + 1));
}

vector<string> AdminCourseManager::parseCSVLine(string line) {
    vector<string> result;
    string cell;
    bool inQuotes = false;
    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        if (c == '\"') inQuotes = !inQuotes;
        else if (c == ',' && !inQuotes) {
            result.push_back(trim(cell));
            cell.clear();
        } else cell += c;
    }
    result.push_back(trim(cell));
    return result;
}

void AdminCourseManager::loadCourseNames(const string& filename) {
    courseNames.clear();
    ifstream file(filename);
    if (!file.is_open()) return;
    string line;
    getline(file, line);
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> cols = parseCSVLine(line);
        if (cols.size() >= 4) courseNames[cols[0]] = cols[3];
    }
}

void AdminCourseManager::parseStudentData(const string& filename) {
    students.clear();
    ifstream file(filename);
    if (!file.is_open()) return;
    string line;
    getline(file, line);
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> columns = parseCSVLine(line);
        if (columns.size() < 11) continue;
        Student s;
        s.id = columns[0]; s.name = columns[1]; s.passedCourses = columns[2];
        s.failedCourses = columns[3]; s.registeredCourses = columns[4];
        s.excuseRequested = columns[6]; s.totalExcuses = columns[7];
        s.withdrawalRequests = columns[8]; s.faculty = columns[9]; s.gpa = columns[10];

        stringstream ss(columns[5]);
        string segment;
        while (getline(ss, segment, '_')) {
            if (segment.empty()) continue;
            size_t bPos = segment.find("\\\\");
            size_t sPos = segment.find("//");
            if (bPos != string::npos && sPos != string::npos) {
                Course c;
                c.code = segment.substr(0, bPos);
                c.name = courseNames.count(c.code) ? courseNames.at(c.code) : "Unknown";
                string fLec = segment.substr(bPos + 2, sPos - (bPos + 2));
                string fTut = segment.substr(sPos + 2);
                size_t lS = fLec.find(' ');
                if (lS != string::npos) { c.lectureDay = fLec.substr(0, lS); c.lectureTime = fLec.substr(lS + 1); }
                size_t tS = fTut.find(' ');
                if (tS != string::npos) { c.tutorialDay = fTut.substr(0, tS); c.tutorialTime = fTut.substr(tS + 1); }
                s.requestedCourses.push_back(c);
            }
        }
        students.push_back(s);
    }
}

void AdminCourseManager::saveAllToCSV(const string& filename) {
    ofstream file(filename);
    file << "ID,Name,Passed Courses,Failed Courses,Registered Courses,Requested Courses,Excuse Requested?,Total Excuses Accepted,Withdrawal Requests,Faculty,GPA\n";
    for (const auto& s : students) {
        string reqStr = "";
        for (const auto& c : s.requestedCourses) {
            reqStr += "_" + c.code + "\\\\" + c.lectureDay + " " + c.lectureTime + "//" + c.tutorialDay + " " + c.tutorialTime;
        }
        file << s.id << "," << s.name << "," << s.passedCourses << "," << s.failedCourses << ","
             << s.registeredCourses << "," << reqStr << "," << s.excuseRequested << ","
             << s.totalExcuses << "," << s.withdrawalRequests << "," << s.faculty << "," << s.gpa << "\n";
    }
}

// --- Internal UI Handlers ---

void AdminCourseManager::resetUI(void* ui_ptr) {
    auto ui = static_cast<Main_App*>(ui_ptr);
    ui->set_Submit_Button_Pressed(false);
    ui->set_c1_code(""); ui->set_c1_name(""); ui->set_c1_l_day(""); ui->set_c1_l_slot(""); ui->set_c1_t_day(""); ui->set_c1_t_slot("");
    ui->set_c2_code(""); ui->set_c2_name(""); ui->set_c2_l_day(""); ui->set_c2_l_slot(""); ui->set_c2_t_day(""); ui->set_c2_t_slot("");
    ui->set_c3_code(""); ui->set_c3_name(""); ui->set_c3_l_day(""); ui->set_c3_l_slot(""); ui->set_c3_t_day(""); ui->set_c3_t_slot("");
    ui->set_c4_code(""); ui->set_c4_name(""); ui->set_c4_l_day(""); ui->set_c4_l_slot(""); ui->set_c4_t_day(""); ui->set_c4_t_slot("");
    ui->set_c5_code(""); ui->set_c5_name(""); ui->set_c5_l_day(""); ui->set_c5_l_slot(""); ui->set_c5_t_day(""); ui->set_c5_t_slot("");

    ui->set_course1_accept_button_pressed(false); ui->set_course1_decline_button_pressed(false);
    ui->set_course2_accept_button_pressed(false); ui->set_course2_decline_button_pressed(false);
    ui->set_course3_accept_button_pressed(false); ui->set_course3_decline_button_pressed(false);
    ui->set_course4_accept_button_pressed(false); ui->set_course4_decline_button_pressed(false);
    ui->set_course5_accept_button_pressed(false); ui->set_course5_decline_button_pressed(false);
}

void AdminCourseManager::loadStudentToUI(void* ui_ptr) {
    auto ui = static_cast<Main_App*>(ui_ptr);
    if (currentStudentIdx < 0 || currentStudentIdx >= (int)students.size()) {
        resetUI(ui);
        ui->set_student_id("No Students Pending");
        ui->set_student_name("");
        ui->set_faculty("");
        return;
    }

    resetUI(ui);
    ui->set_show_warning(false);
    const Student& s = students[currentStudentIdx];
    ui->set_student_id(s.id.c_str());
    ui->set_student_name(s.name.c_str());
    ui->set_faculty(s.faculty.c_str());

    if (s.requestedCourses.size() >= 1) {
        ui->set_c1_code(s.requestedCourses[0].code.c_str()); ui->set_c1_name(s.requestedCourses[0].name.c_str());
        ui->set_c1_l_day(s.requestedCourses[0].lectureDay.c_str()); ui->set_c1_l_slot(s.requestedCourses[0].lectureTime.c_str());
        ui->set_c1_t_day(s.requestedCourses[0].tutorialDay.c_str()); ui->set_c1_t_slot(s.requestedCourses[0].tutorialTime.c_str());
    }
    if (s.requestedCourses.size() >= 2) {
        ui->set_c2_code(s.requestedCourses[1].code.c_str()); ui->set_c2_name(s.requestedCourses[1].name.c_str());
        ui->set_c2_l_day(s.requestedCourses[1].lectureDay.c_str()); ui->set_c2_l_slot(s.requestedCourses[1].lectureTime.c_str());
        ui->set_c2_t_day(s.requestedCourses[1].tutorialDay.c_str()); ui->set_c2_t_slot(s.requestedCourses[1].tutorialTime.c_str());
    }
    if (s.requestedCourses.size() >= 3) {
        ui->set_c3_code(s.requestedCourses[2].code.c_str()); ui->set_c3_name(s.requestedCourses[2].name.c_str());
        ui->set_c3_l_day(s.requestedCourses[2].lectureDay.c_str()); ui->set_c3_l_slot(s.requestedCourses[2].lectureTime.c_str());
        ui->set_c3_t_day(s.requestedCourses[2].tutorialDay.c_str()); ui->set_c3_t_slot(s.requestedCourses[2].tutorialTime.c_str());
    }
    if (s.requestedCourses.size() >= 4) {
        ui->set_c4_code(s.requestedCourses[3].code.c_str()); ui->set_c4_name(s.requestedCourses[3].name.c_str());
        ui->set_c4_l_day(s.requestedCourses[3].lectureDay.c_str()); ui->set_c4_l_slot(s.requestedCourses[3].lectureTime.c_str());
        ui->set_c4_t_day(s.requestedCourses[3].tutorialDay.c_str()); ui->set_c4_t_slot(s.requestedCourses[3].tutorialTime.c_str());
    }
    if (s.requestedCourses.size() >= 5) {
        ui->set_c5_code(s.requestedCourses[4].code.c_str()); ui->set_c5_name(s.requestedCourses[4].name.c_str());
        ui->set_c5_l_day(s.requestedCourses[4].lectureDay.c_str()); ui->set_c5_l_slot(s.requestedCourses[4].lectureTime.c_str());
        ui->set_c5_t_day(s.requestedCourses[4].tutorialDay.c_str()); ui->set_c5_t_slot(s.requestedCourses[4].tutorialTime.c_str());
    }
}

// --- Public Interface ---

void AdminCourseManager::initUI(void* ui_ptr) {
    loadCourseNames(dbBasePath + "Offered_Courses.csv");
    parseStudentData(dbBasePath + "Data_on_Each_Student.csv");

    currentStudentIdx = -1;
    for (int i = 0; i < (int)students.size(); ++i) {
        if (!students[i].requestedCourses.empty()) {
            currentStudentIdx = i;
            break;
        }
    }

    if (currentStudentIdx == -1) {
        resetUI(ui_ptr);
        auto ui = static_cast<Main_App*>(ui_ptr);
        ui->set_student_id("");
    } else {
        loadStudentToUI(ui_ptr);
    }
}

void AdminCourseManager::nextStudent(void* ui_ptr) {
    int start_search = (currentStudentIdx == -1) ? 0 : currentStudentIdx + 1;
    for (int i = start_search; i < (int)students.size(); ++i) {
        if (!students[i].requestedCourses.empty()) {
            currentStudentIdx = i;
            loadStudentToUI(ui_ptr);
            return;
        }
    }
    cout << "No more students with requested courses forwards." << endl;
}

void AdminCourseManager::prevStudent(void* ui_ptr) {
    int prev_idx = currentStudentIdx - 1;
    while (prev_idx >= 0) {
        if (!students[prev_idx].requestedCourses.empty()) {
            currentStudentIdx = prev_idx;
            loadStudentToUI(ui_ptr);
            return;
        }
        prev_idx--;
    }
}

void AdminCourseManager::submitDecisions(void* ui_ptr) {
    auto ui = static_cast<Main_App*>(ui_ptr);
    if (currentStudentIdx < 0 || currentStudentIdx >= (int)students.size()) return;

    Student& s = students[currentStudentIdx];
    vector<Course> still_requested;

    auto formatForCSV = [](const Course& c) {
        return "_" + c.code + "\\\\" + c.lectureDay + " " + c.lectureTime +
               "//" + c.tutorialDay + " " + c.tutorialTime;
    };

    auto processRow = [&](int idx, bool accepted, bool declined) {
        if (idx >= (int)s.requestedCourses.size()) return;
        if (accepted) {
            s.registeredCourses += formatForCSV(s.requestedCourses[idx]);
        } else if (!declined) {
            still_requested.push_back(s.requestedCourses[idx]);
        }
    };

    processRow(0, ui->get_course1_accept_button_pressed(), ui->get_course1_decline_button_pressed());
    processRow(1, ui->get_course2_accept_button_pressed(), ui->get_course2_decline_button_pressed());
    processRow(2, ui->get_course3_accept_button_pressed(), ui->get_course3_decline_button_pressed());
    processRow(3, ui->get_course4_accept_button_pressed(), ui->get_course4_decline_button_pressed());
    processRow(4, ui->get_course5_accept_button_pressed(), ui->get_course5_decline_button_pressed());

    s.requestedCourses = still_requested;
    saveAllToCSV(dbBasePath + "Data_on_Each_Student.csv");

    ui->set_Submit_Button_Pressed(true);
    ui->set_show_warning(false);
}