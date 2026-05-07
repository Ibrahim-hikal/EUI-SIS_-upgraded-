#ifndef ADD_TEACHER_H
#define ADD_TEACHER_H

#include <string>

using namespace std;

class Add_Teacher {
public:
    static void add_teacher(
        const string& name,
        const string& email
    );
};

#endif