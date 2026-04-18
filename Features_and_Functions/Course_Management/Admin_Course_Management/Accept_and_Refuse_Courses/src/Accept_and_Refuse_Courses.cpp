#include <iostream>
#include <vector>
#include <map>
#include "accept_and_refuse_courses.h"
#include "../header/Accept_and_Refuse_Courses.h"

using namespace std;



void updateStudentCSV(string filename, const vector<Student>& students) {
    ofstream file(filename);
    if (!file.is_open()) return;

    // Write header
    file << "ID,Name,Passed Courses,Failed Courses,Registered Courses,Requested Courses,Excuse Requested?,Total Excuses Accepted,Withdrawal Requests,Faculty,GPA\n";

    for (const auto& s : students) {
        file << s.id << "," << s.name << ",,,,"; // Keeping existing gaps for simplicity

        // Logic to format Registered and Requested strings goes here based on the modifications made during the 'Submit' callback.
    }
}

string trim(const string& s) {
    size_t first = s.find_first_not_of(" \t\r\n");
    if (string::npos == first) return s;
    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, (last - first + 1));
}

vector<string> parseCSVLine(string line) {
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

map<string, string> loadCourseNames(string filename) {
    map<string, string> names;
    ifstream file(filename);
    if (!file.is_open()) return names;
    string line;
    getline(file, line); // header
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> cols = parseCSVLine(line);
        if (cols.size() >= 4) names[cols[0]] = cols[3];
    }
    return names;
}

vector<Student> parseStudentData(string filename, const map<string, string>& courseNames) {
    vector<Student> students;
    ifstream file(filename);
    if (!file.is_open()) return students;
    string line;
    getline(file, line); // header
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> columns = parseCSVLine(line);
        if (columns.size() < 11) continue;
        Student s;
        s.id = columns[0]; s.name = columns[1]; s.passedCourses = columns[2];
        s.failedCourses = columns[3]; s.registeredCourses = columns[4];
        s.excuseRequested = columns[6]; s.totalExcuses = columns[7];
        s.withdrawalRequests = columns[8]; s.faculty = columns[9]; s.gpa = columns[10];

        stringstream ss(columns[5]); // Requested Courses column
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
    return students;
}

void saveAllToCSV(string filename, const vector<Student>& students) {
    ofstream file(filename);
    // Write header exactly as requested
    file << "ID,Name,Passed Courses,Failed Courses,Registered Courses,Requested Courses,Excuse Requested?,Total Excuses Accepted,Withdrawal Requests,Faculty,GPA\n";

    for (const auto& s : students) {
        string reqStr = "";
        for (const auto& c : s.requestedCourses) {
            reqStr += "_" + c.code + "\\\\" + c.lectureDay + " " + c.lectureTime + "//" + c.tutorialDay + " " + c.tutorialTime;
        }

        // Ensure s.registeredCourses (Column 5) is included
        file << s.id << "," << s.name << "," << s.passedCourses << "," << s.failedCourses << ","
             << s.registeredCourses << "," << reqStr << "," << s.excuseRequested << ","
             << s.totalExcuses << "," << s.withdrawalRequests << "," << s.faculty << "," << s.gpa << "\n";
    }
}

