#ifndef ACCEPT_AND_REFUSE_COURSES_H
#define ACCEPT_AND_REFUSE_COURSES_H

#include <string>
#include <vector>
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

// Encapsulated Manager Class
class AdminCourseManager {
private:
    vector<Student> students;
    map<string, string> courseNames;
    int currentStudentIdx;
    string dbBasePath;

    // Internal Utility Methods
    string trim(const string& s);
    vector<string> parseCSVLine(string line);
    void loadCourseNames(const string& filename);
    void parseStudentData(const string& filename);
    void saveAllToCSV(const string& filename);

    // Internal UI State Handlers
    void resetUI(void* ui_ptr);
    void loadStudentToUI(void* ui_ptr);

public:
    // Constructor
    AdminCourseManager(const string& basePath);

    // Public UI Hooks (void* prevents circular dependencies with main.h)
    void initUI(void* ui_ptr);
    void nextStudent(void* ui_ptr);
    void prevStudent(void* ui_ptr);
    void submitDecisions(void* ui_ptr);
};

#endif