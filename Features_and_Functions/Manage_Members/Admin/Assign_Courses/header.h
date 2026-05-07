#ifndef ASSIGN_COURSES_H
#define ASSIGN_COURSES_H

#include <string>

using namespace std;

class Assign_Courses {
public:
    static void assign_course(
        const string& teacher_email,
        const string& course_name
    );
};

#endif