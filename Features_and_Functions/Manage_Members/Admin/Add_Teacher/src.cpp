#include "header.h"

#include <fstream>
#include <iostream>

#define TEACHER_DB "Databases/Teachers_Data.csv"

using namespace std;

void Add_Teacher::add_teacher(
    const string& name,
    const string& email
) {
    ofstream file(TEACHER_DB, ios::app);

    if (!file.is_open()) {
        cout << "Error opening teacher database\n";
        return;
    }

    file << name << ","
         << email << "\n";

    file.close();

    cout << "Teacher added successfully\n";
}