#ifndef ADD_STUDENT_H
#define ADD_STUDENT_H

#include <string>

using namespace std;

class Add_Student {
public:
    static void add_student(
        const string& id,
        const string& name,
        const string& faculty,
        const string& gpa
    );
};

#endif