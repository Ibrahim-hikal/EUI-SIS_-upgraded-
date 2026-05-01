#ifndef STUDENT_H
#define STUDENT_H

#include <string>
#include <vector>
#include <iostream>

using namespace std;

// ==========================================
// COURSE STRUCTURE
// ==========================================
class Course {
public:
    string code;
    string name;
    string preReq1;
    string preReq2;
    string lectureDay;
    string tutorialDay;
    string lectureTime;
    string tutorialTime;
};

// ==========================================
// STUDENT CLASS
// ==========================================
class Student {
public:
    // --- Public Properties ---
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

    // --- Constructors ---
    Student() = default;
    explicit Student(string student_id) : id(move(student_id)) {}

    // --- Core Methods ---
    static void load_offered_courses();
    void load_data();

    // Evaluates the student's history to build a list of allowed courses
    vector<Course> get_eligible_courses() const;

    // Attempts to add a course to the student's request list
    bool register_course(const string& course_code);

private:
    // --- Internal Storage ---
    // Holds the master list of all university courses in memory
    static vector<Course>& get_mutable_courses();

    // --- Private Helper Methods for Clean Logic ---
    // These abstract away the messy string searches
    bool is_already_taken(const Course& c) const;
    bool is_previously_failed(const Course& c) const;
    bool meets_prerequisites(const Course& c) const;
};

#endif // STUDENT_H