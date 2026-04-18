#include "../header/Accept_and_Refuse_Courses.h"
#include <iostream>

using namespace std;

// Helper to remove extra spaces/newlines from CSV data
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
        if (c == '"') inQuotes = !inQuotes;
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
    string line;
    if (!file.is_open()) {
        cerr << "Error: Could not open Offered_Courses.csv" << endl;
        return names;
    }
    getline(file, line); // Skip header
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> cols = parseCSVLine(line);
        if (cols.size() >= 4) {
            // Code is index 0, Name is index 3 in your CSV
            names[cols[0]] = cols[3];
        }
    }
    return names;
}

vector<Student> parseStudentData(string filename, const map<string, string>& courseNames) {
    vector<Student> students;
    ifstream file(filename);
    string line;
    if (!file.is_open()) return students;

    getline(file, line); // Skip header row

    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> columns = parseCSVLine(line);
        if (columns.size() < 10) continue;

        Student s;
        s.id = columns[0];
        s.name = columns[1];
        s.faculty = columns[9];

        stringstream ss(columns[5]); // Requested Courses
        string segment;
        while (getline(ss, segment, '_')) {
            if (segment.empty()) continue;
            size_t backslashPos = segment.find("\\\\");
            size_t slashPos = segment.find("//");

            if (backslashPos != string::npos && slashPos != string::npos) {
                Course c;
                c.code = segment.substr(0, backslashPos);

                // LOOKUP: Get name from map based on code
                if (courseNames.count(c.code)) {
                    c.name = courseNames.at(c.code);
                } else {
                    c.name = "Unknown Course";
                }

                string fullLec = segment.substr(backslashPos + 2, slashPos - (backslashPos + 2));
                string fullTut = segment.substr(slashPos + 2);

                size_t lecSpace = fullLec.find(' ');
                if (lecSpace != string::npos) {
                    c.lectureDay = fullLec.substr(0, lecSpace);
                    c.lectureTime = fullLec.substr(lecSpace + 1);
                }
                size_t tutSpace = fullTut.find(' ');
                if (tutSpace != string::npos) {
                    c.tutorialDay = fullTut.substr(0, tutSpace);
                    c.tutorialTime = fullTut.substr(tutSpace + 1);
                }
                s.requestedCourses.push_back(c);
            }
        }
        students.push_back(s);
    }
    return students;
}