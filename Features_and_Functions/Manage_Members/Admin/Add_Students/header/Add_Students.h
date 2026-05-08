#ifndef ADD_STUDENT_H
#define ADD_STUDENT_H

#include <string>

class Add_Student {
public:
    static std::string generate_student_id();

    // Now accepts the password directly from the UI
    static std::string add_student(const std::string& name, const std::string& faculty, const std::string& password);
};

#endif