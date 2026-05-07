#include "../header/Add_Students.h"

#include <fstream>
#include <iostream>

#define STUDENT_DB "Databases/Data_on_Each_Student.csv"

using namespace std;

void Add_Student::add_student(
    const string& id,
    const string& name,
    const string& faculty,
    const string& gpa
) {
    ofstream file(STUDENT_DB, ios::app);

    if (!file.is_open()) {
        cout << "Error opening database\n";
        return;
    }

    file << id << ","
         << name << ","
         << faculty << ","
         << gpa << "\n";

    file.close();

    cout << "Student added successfully\n";
}