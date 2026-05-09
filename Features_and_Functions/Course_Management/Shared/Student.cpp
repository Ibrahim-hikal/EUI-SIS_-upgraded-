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

    if (!courses.empty()) return;

    ifstream file("Databases/Offered_Courses.csv");
    if (!file.is_open()) {
        cerr << "CRITICAL: Could not open Offered_Courses.csv" << endl;
        return;
    }

    string line, cell;
    getline(file, line);

    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        vector<string> cols;
        while (getline(ss, cell, ',')) {
            cols.push_back(cell);
        }

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

bool Student::is_already_taken(const Course& c) const {
    bool passed = (passedCourses.find(c.code) != string::npos);
    bool taking_now = (registeredCourses.find(c.code) != string::npos);
    return passed || taking_now;
}

bool Student::is_previously_failed(const Course& c) const {
    return failedCourses.find(c.code) != string::npos;
}

bool Student::meets_prerequisites(const Course& c) const {
    auto is_met = [this](const string& pre) {
        return pre == "-1" || passedCourses.find(pre) != string::npos;
    };
    return is_met(c.preReq1) && is_met(c.preReq2);
}

vector<Course> Student::get_eligible_courses() const {
    vector<Course> eligible;

    for (const auto& c : get_mutable_courses()) {
        if (is_already_taken(c)) continue;
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

    if (new_lec == new_tut) return true;

    for (const auto& c : requestedCourses) {
        string exist_lec = c.lectureDay + " " + c.lectureTime;
        string exist_tut = c.tutorialDay + " " + c.tutorialTime;
        if (new_lec == exist_lec || new_lec == exist_tut ||
            new_tut == exist_lec || new_tut == exist_tut) {
            return true;
        }
    }

    if (registeredCourses.find(new_lec) != string::npos) return true;
    if (registeredCourses.find(new_tut) != string::npos) return true;

    return false;
}

bool Student::register_course(const string& course_code, const string& l_day, const string& l_time, const string& t_day, const string& t_time) {
    if (has_time_conflict(l_day, l_time, t_day, t_time)) return false;

    for (const auto& c : get_eligible_courses()) {
        if (c.code == course_code) {
            Course new_request = c;
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
    ifstream file("Databases/Data_on_Each_Student.csv");
    if (!file.is_open()) {
        cerr << "CRITICAL: Could not open Data_on_Each_Student.csv" << endl;
        return;
    }

    string line;
    getline(file, line);

    while (getline(file, line)) {
        if (line.empty()) continue;

        vector<string> cols;
        string current;
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

        if (!cols.empty() && cols[0] == this->id) {
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

                        for (const auto& mc : get_mutable_courses()) {
                            // USING OPERATOR OVERLOAD == HERE
                            if (mc == c) { c.name = mc.name; break; }
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

            cout << "SUCCESS: Loaded data for student " << this->id << endl;
            for(const auto& req : this->requestedCourses) {
                // USING OPERATOR OVERLOAD << HERE
                cout << "  -> Loaded Request: " << req << endl;
            }
            break;
        }
    }
    file.close();
}

// ==========================================
// FILE I/O
// ==========================================
void Student::save_requests_to_csv() {
    ifstream fileIn("Databases/Data_on_Each_Student.csv");
    if (!fileIn.is_open()) return;
    vector<string> lines;
    string line;

    while (getline(fileIn, line)) {
        if (line.empty()) continue;
        bool has_cr = (line.back() == '\r');
        if (has_cr) line.pop_back();

        vector<string> cols;
        string current;
        bool in_quotes = false;
        for (char c : line) {
            if (c == '"') in_quotes = !in_quotes;
            else if (c == ',' && !in_quotes) { cols.push_back(current); current.clear(); }
            else current += c;
        }
        cols.push_back(current);

        if (!cols.empty() && cols[0] == this->id) {
            string reqStr = "";
            for (const auto& c : this->requestedCourses) {
                reqStr += "_" + c.code + "\\\\" + c.lectureDay + " " + c.lectureTime + "//" + c.tutorialDay + " " + c.tutorialTime;
            }
            while (cols.size() <= 5) cols.push_back("");
            cols[5] = reqStr;

            line = "";
            for (size_t i = 0; i < cols.size(); ++i) {
                line += cols[i] + (i < cols.size() - 1 ? "," : "");
            }
        }
        lines.push_back(line + (has_cr ? "\r" : ""));
    }
    fileIn.close();

    ofstream fileOut("Databases/Data_on_Each_Student.csv");
    for (const auto& l : lines) fileOut << l << "\n";
}