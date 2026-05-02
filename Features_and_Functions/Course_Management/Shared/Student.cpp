#include "Student.h"
#include <fstream>
#include <sstream>
#include <iostream>

    using namespace std;
// ==========================================
// STATIC DATA STORAGE
// ==========================================
vector<Course>& Student::get_mutable_courses() {
    static vector<Course> eligible;
    return eligible;
}

// ==========================================
// CSV PARSING & LOADING
// ==========================================
void Student::load_offered_courses() {
    auto& courses = get_mutable_courses();

    // Guard Clause: If we already loaded the data, stop here.
    if (!courses.empty()) return;

    ifstream file("Databases/Offered_Courses.csv");
    if (!file.is_open()) {
        cerr << "CRITICAL: Could not open Offered_Courses.csv" << endl;
        return;
    }

    string line, cell;
    getline(file, line); // Skip CSV header

    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        vector<string> cols;
        while (getline(ss, cell, ',')) {
            cols.push_back(cell);
        }

        // Ensure we have enough columns to avoid out-of-bounds crashes
        if (cols.size() >= 4) {
            Course c;
            c.code = cols[0];
            c.preReq1 = cols[1];
            c.preReq2 = cols[2];
            c.name = cols[3];
            courses.push_back(c);
        }
    }
    file.close();
}

// ==========================================
// COURSE FILTERING LOGIC
// ==========================================

// Helper 1: Check if the student passed it or is taking it now
bool Student::is_already_taken(const Course& c) const {
    bool passed = (passedCourses.find(c.code) != string::npos);
    bool taking_now = (registeredCourses.find(c.code) != string::npos);
    return passed || taking_now;
}

// Helper 2: Check if they failed it in the past
bool Student::is_previously_failed(const Course& c) const {
    return failedCourses.find(c.code) != string::npos;
}

// Helper 3: Check if both prerequisites are met (or don't exist)
bool Student::meets_prerequisites(const Course& c) const {
    auto is_met = [this](const string& pre) {
        return pre == "-1" || passedCourses.find(pre) != string::npos;
    };
    return is_met(c.preReq1) && is_met(c.preReq2);
}

// The main filter: Notice how easy this is to read now!
vector<Course> Student::get_eligible_courses() const {
    vector<Course> eligible;

    for (const auto& c : get_mutable_courses()) {
        // 1. If they already took it and passed, skip it.
        if (is_already_taken(c)) {
            continue;
        }

        // 2. Allow if they meet prerequisites OR if they are retaking a failed class
        if (meets_prerequisites(c) || is_previously_failed(c)) {
            eligible.push_back(c);
        }
    }

    return eligible;
}

// ==========================================
// REGISTRATION
// ==========================================
bool Student::has_time_conflict(const string& l_day, const string& l_time, const string& t_day, const string& t_time) const {
    string new_lec = l_day + " " + l_time;
    string new_tut = t_day + " " + t_time;

    // 1. CRITICAL FIX: A course cannot have its lecture and tutorial at the exact same time!
    if (new_lec == new_tut) return true;

    // 2. Check Pending Requested Courses
    for (const auto& c : requestedCourses) {
        string exist_lec = c.lectureDay + " " + c.lectureTime;
        string exist_tut = c.tutorialDay + " " + c.tutorialTime;
        if (new_lec == exist_lec || new_lec == exist_tut ||
            new_tut == exist_lec || new_tut == exist_tut) {
            return true;
            }
    }

    // 3. Check Already Registered Courses
    if (registeredCourses.find(new_lec) != string::npos) return true;
    if (registeredCourses.find(new_tut) != string::npos) return true;

    return false;
}

bool Student::register_course(const string& course_code, const string& l_day, const string& l_time, const string& t_day, const string& t_time) {
    // Before adding the course, we check for a time clash!
    if (has_time_conflict(l_day, l_time, t_day, t_time)) {
        return false;
    }

    for (const auto& c : get_eligible_courses()) {
        if (c.code == course_code) {
            Course new_request = c;
            // Inject the custom selected times chosen by the student in the UI
            new_request.lectureDay = l_day;
            new_request.lectureTime = l_time;
            new_request.tutorialDay = t_day;
            new_request.tutorialTime = t_time;

            requestedCourses.push_back(new_request);
            return true;
        }
    }
    return false;
}

void Student::load_data() {
    std::ifstream file("Databases/Data_on_Each_Student.csv");
    if (!file.is_open()) {
        std::cerr << "CRITICAL: Could not open Data_on_Each_Student.csv" << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line); // Skip header

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // 1. Safe CSV Parser (ignores commas inside quotes)
        std::vector<std::string> cols;
        std::string current;
        bool in_quotes = false;

        for (char c : line) {
            if (c == '"') {
                in_quotes = !in_quotes;
            } else if (c == ',' && !in_quotes) {
                cols.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        }
        cols.push_back(current);

        // 2. Find this specific student and load their history
        if (!cols.empty() && cols[0] == this->id) {

            // NOTE: Double check these column numbers match your CSV layout!
            // Based on your Registered_Courses file, Registered is column 4.
            if (cols.size() > 2) this->passedCourses = cols[2];
            if (cols.size() > 3) this->failedCourses = cols[3];
            if (cols.size() > 4) this->registeredCourses = cols[4];

            if (cols.size() > 5 && !cols[5].empty()) {
                string reqStr = cols[5];
                size_t pos = 0;
                while ((pos = reqStr.find('_', pos)) != string::npos) {
                    size_t next_pos = reqStr.find('_', pos + 1);
                    string segment = (next_pos == string::npos) ? reqStr.substr(pos + 1) : reqStr.substr(pos + 1, next_pos - pos - 1);

                    size_t bPos = segment.find("\\\\");
                    size_t sPos = segment.find("//");

                    if (bPos != string::npos && sPos != string::npos) {
                        Course c;
                        c.code = segment.substr(0, bPos);

                        // Pull the name from the master list so the UI displays it correctly
                        for (const auto& mc : get_mutable_courses()) {
                            if (mc.code == c.code) { c.name = mc.name; break; }
                        }

                        string fLec = segment.substr(bPos + 2, sPos - (bPos + 2));
                        string fTut = segment.substr(sPos + 2);

                        size_t lS = fLec.find(' ');
                        if (lS != string::npos) { c.lectureDay = fLec.substr(0, lS); c.lectureTime = fLec.substr(lS + 1); }

                        size_t tS = fTut.find(' ');
                        if (tS != string::npos) { c.tutorialDay = fTut.substr(0, tS); c.tutorialTime = fTut.substr(tS + 1); }

                        this->requestedCourses.push_back(c);
                    }
                    pos++;
                }
            }

            std::cout << "SUCCESS: Loaded data for student " << this->id << std::endl;
            break; // Stop searching once we find them
        }
    }
    file.close();
}
// ==========================================
// FILE I/O
// ==========================================
void Student::save_requests_to_csv() {
    std::ifstream fileIn("Databases/Data_on_Each_Student.csv");
    if (!fileIn.is_open()) return;
    std::vector<std::string> lines;
    std::string line;

    while (std::getline(fileIn, line)) {
        if (line.empty()) continue;
        bool has_cr = (line.back() == '\r');
        if (has_cr) line.pop_back();

        // Custom split to ignore commas inside quotes
        std::vector<std::string> cols;
        std::string current;
        bool in_quotes = false;
        for (char c : line) {
            if (c == '"') in_quotes = !in_quotes;
            else if (c == ',' && !in_quotes) { cols.push_back(current); current.clear(); }
            else current += c;
        }
        cols.push_back(current);

        // Update this specific student's record
        if (!cols.empty() && cols[0] == this->id) {
            std::string reqStr = "";
            for (const auto& c : this->requestedCourses) {
                reqStr += "_" + c.code + "\\\\" + c.lectureDay + " " + c.lectureTime + "//" + c.tutorialDay + " " + c.tutorialTime;
            }
            while (cols.size() <= 5) cols.push_back(""); // Ensure column 5 exists
            cols[5] = reqStr; // Replace requested courses column

            line = "";
            for (size_t i = 0; i < cols.size(); ++i) {
                line += cols[i] + (i < cols.size() - 1 ? "," : "");
            }
        }
        lines.push_back(line + (has_cr ? "\r" : ""));
    }
    fileIn.close();

    // Overwrite the database
    std::ofstream fileOut("Databases/Data_on_Each_Student.csv");
    for (const auto& l : lines) fileOut << l << "\n";
}