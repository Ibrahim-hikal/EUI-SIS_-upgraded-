#ifndef ADD_TEACHER_H
#define ADD_TEACHER_H

#include <string>

using namespace std;

class Add_Teacher {
public:
    // Now accepts first name, last name, and password, and returns a success message
    static string add_teacher(
        const string& first_name,
        const string& last_name,
        const string& password
    );
};

#endif