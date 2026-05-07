//------------Previous Enrollments .h File-------------

#ifndef PROJECT_PREVIOUS_ENROLLMENTS_H
#define PROJECT_PREVIOUS_ENROLLMENTS_H

#include <string>

class Main_App;

class EnrollmentManager {
public:

    static void loadStudentData(const std::string& targetID, Main_App* ui);
};

#endif //PROJECT_PREVIOUS_ENROLLMENTS_H