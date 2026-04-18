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
    string name;
    string lectureDay;
    string tutorialDay;
    string lectureTime;
    string tutorialTime;
};

class Student {
public:
    string id;
    string name;
    string passedCourses;
    string failedCourses;
    string registeredCourses;
    vector<Course> requestedCourses;
    string excuseRequested;
    string totalExcuses;
    string withdrawalRequests;
    string faculty;
    string gpa;
};

vector<string> parseCSVLine(string line);
map<string, string> loadCourseNames(string filename);
vector<Student> parseStudentData(string filename, const map<string, string>& courseNames);
void updateStudentCSV(string filename, const vector<Student>& students);
void saveAllToCSV(string filename, const vector<Student>& students);

#endif