#include "../header/Assign_Courses.h"
#include <fstream>
#include <sstream>
#include <vector>

#define TEACHER_DB "Databases/Data_on_each_teacher.csv"
#define COURSES_DB "Databases/Offered_Courses.csv"

using namespace std;

string Assign_Course::get_teacher_name(const string& email) {
    ifstream file(TEACHER_DB);
    string line, name, e;
    if (file.is_open()) {
        getline(file, line);
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            getline(ss, name, ',');
            getline(ss, e, ',');
            if (!e.empty() && e.back() == '\r') e.pop_back();
            if (e == email) return name;
        }
    }
    return "Not Found";
}

string Assign_Course::get_course_name(const string& code) {
    ifstream file(COURSES_DB);
    string line, c, p1, p2, name;
    if (file.is_open()) {
        getline(file, line);
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            getline(ss, c, ',');
            getline(ss, p1, ',');
            getline(ss, p2, ',');
            getline(ss, name, ',');
            if (!c.empty() && c.back() == '\r') c.pop_back();
            if (c == code) return name;
        }
    }
    return "Not Found";
}

string Assign_Course::assign_course(const string& email, const string& code, const string& l1, const string& l2, const string& t1, const string& t2) {
    string t_name = get_teacher_name(email);
    string c_name = get_course_name(code);

    if (t_name == "Not Found") return "Error: Teacher Email not found.";
    if (c_name == "Not Found") return "Error: Course Code not found.";

    // 1. Format the Time Slots
    string lec_str = "";
    if (l1 != "None") lec_str += l1;
    if (l2 != "None") {
        if (!lec_str.empty()) lec_str += " ";
        lec_str += l2;
    }

    string tut_str = "";
    if (t1 != "None") tut_str += t1;
    if (t2 != "None") {
        if (!tut_str.empty()) tut_str += " ";
        tut_str += t2;
    }

    // 2. Update Teacher Database
    ifstream fileIn(TEACHER_DB);
    vector<string> lines;
    bool updated = false;

    if (fileIn.is_open()) {
        string line;
        while (getline(fileIn, line)) {
            if (line.empty()) continue;
            if (line.back() == '\r') line.pop_back();

            stringstream ss(line);
            string name, e, courses;
            getline(ss, name, ',');
            getline(ss, e, ',');
            getline(ss, courses);

            if (e == email) {
                if (courses == "\"\"" || courses.empty()) {
                    courses = c_name;
                } else if (courses.find(c_name) == string::npos) {
                    courses += "_" + c_name;
                }
                line = name + "," + e + "," + courses;
                updated = true;
            }
            lines.push_back(line);
        }
        fileIn.close();
    }

    if (updated) {
        ofstream fileOut(TEACHER_DB, ios::trunc);
        for (const auto& l : lines) fileOut << l << "\n";
        fileOut.close();
    }

    // 3. Update Offered Courses Database
    ifstream fileInC(COURSES_DB);
    vector<string> c_lines;
    bool c_updated = false;

    if (fileInC.is_open()) {
        string line;
        while (getline(fileInC, line)) {
            if (line.empty()) continue;
            if (line.back() == '\r') line.pop_back();

            stringstream ss(line);
            string cell;
            vector<string> cols;
            while (getline(ss, cell, ',')) cols.push_back(cell);

            // Format: Code,Pre1,Pre2,Name,Instructor,Lec,Tut,Credits
            if (cols.size() >= 8 && cols[0] == code) {
                cols[4] = t_name; // Set Teacher
                cols[5] = lec_str; // Set Lectures
                cols[6] = tut_str; // Set Tutorials

                line = cols[0];
                for(size_t i = 1; i < cols.size(); ++i) {
                    line += "," + cols[i];
                }
                c_updated = true;
            }
            c_lines.push_back(line);
        }
        fileInC.close();
    }

    if (c_updated) {
        ofstream fileOutC(COURSES_DB, ios::trunc);
        for (const auto& l : c_lines) fileOutC << l << "\n";
        fileOutC.close();
        return "Successfully assigned " + code + " to " + t_name + "!";
    }

    return "Error: Course " + code + " not found in Offered DB.";
}