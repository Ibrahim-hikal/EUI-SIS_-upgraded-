#ifndef ACCEPT_AND_REFUSE_COURSES_H
#define ACCEPT_AND_REFUSE_COURSES_H

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>

using namespace std;

class Course {
public:
    string code;
    string name; // The "Subject Name" from Offered_Courses.csv
    string lectureDay;
    string tutorialDay;
    string lectureTime;
    string tutorialTime;
};

class Student {
public:
    string id;
    string name;
    string faculty;
    vector<Course> requestedCourses;
};

// Function declarations
vector<string> parseCSVLine(string line);
map<string, string> loadCourseNames(string filename);
vector<Student> parseStudentData(string filename, const map<string, string>& courseNames);

#endif