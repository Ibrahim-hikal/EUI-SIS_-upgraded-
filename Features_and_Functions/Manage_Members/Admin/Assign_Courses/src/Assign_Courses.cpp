#include "../header/Assign_Courses.h"

#include <fstream>
#include <iostream>

#define COURSE_DB "Databases/Teacher_Courses.csv"

using namespace std;

void Assign_Courses::assign_course(
    const string& teacher_email,
    const string& course_name
) {
    ofstream file(COURSE_DB, ios::app);

    if (!file.is_open()) {
        cout << "Error opening course database\n";
        return;
    }

    file << teacher_email << ","
         << course_name << "\n";

    file.close();

    cout << "Course assigned successfully\n";
}