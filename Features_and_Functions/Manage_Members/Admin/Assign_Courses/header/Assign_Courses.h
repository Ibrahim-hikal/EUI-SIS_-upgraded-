#ifndef ASSIGN_COURSES_H
#define ASSIGN_COURSES_H

// Readability note: declarations are grouped for easier scanning.

#include <string>

using namespace std;

class Assign_Course {
public:
    static string get_teacher_name(const string& email);
    static string get_course_name(const string& code);

    // NEW: Real-time conflict checker
    static string check_conflicts(const string& email, const string& code, const string& l1, const string& l2, const string& t1, const string& t2);

    static string assign_course(const string& email, const string& code, const string& l1, const string& l2, const string& t1, const string& t2);
};

#endif