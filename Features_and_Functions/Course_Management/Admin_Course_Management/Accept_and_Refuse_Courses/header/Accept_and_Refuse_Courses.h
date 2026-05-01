#ifndef ACCEPT_AND_REFUSE_COURSES_H
#define ACCEPT_AND_REFUSE_COURSES_H

#include <string>
#include <vector>
#include <map>

// IMPORT THE NEW STUDENT FILE
#include "../../../Shared/Student.h" // <-- Make sure the relative path to this file is correct

using namespace std;

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
    map<string, vector<int>> sessionDecisions;
    void saveCurrentDecisions(void* ui_ptr);

public:
    explicit AdminCourseManager(const string& basePath);

    // Public UI Hooks
    void initUI(void* ui_ptr);
    void nextStudent(void* ui_ptr);
    void prevStudent(void* ui_ptr);
    void submitDecisions(void* ui_ptr);
};

#endif