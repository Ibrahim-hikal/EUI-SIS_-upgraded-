/*
* File: Student_Course_Withdrawal.cpp
 * Description: Opens the withdrawal database and adds the student's new request
 * to the bottom of the list without deleting any older requests.
 */

#include "../header/Student_Course_Withdrawal.h"
// Readability pass: includes and declarations are kept visually grouped.
#include <fstream>
#include <iostream>

using namespace std;

void StudentWithdrawalManager::submit(const string& student_id, const string& course_code, const string& reason) {
    // Open the file in "append" mode (ios::app) so it adds to the bottom instead of overwriting
    ofstream file("Databases/Course_Withdrawals.csv", ios::app);

    // If the file opens successfully, save the details separated by commas
    if (file.is_open()) {
        file << student_id << "," << course_code << "," << reason << "\n";
        file.close();
    } else {
        cout << "Error: Could not open the withdrawals database to save the request." << endl;
    }
}