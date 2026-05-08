#include "../header/Add_Students.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

#define STUDENT_DB "Databases/Data_on_Each_Student.csv"
#define LOGIN_DB "Databases/Login_Credentials.csv" // Make sure this matches your login file name!

using namespace std;

string Add_Student::generate_student_id() {
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    tm* local = localtime(&t);
    int year = (local->tm_year + 1900) % 100;

    stringstream prefix_ss;
    prefix_ss << setfill('0') << setw(2) << year << "-101";
    string prefix = prefix_ss.str();

    int max_suffix = 0;

    ifstream file(STUDENT_DB);
    if (file.is_open()) {
        string line;
        getline(file, line);
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string id;
            getline(ss, id, ',');

            if (id.find(prefix) == 0) {
                string suffix_str = id.substr(prefix.length());
                try {
                    int suffix = stoi(suffix_str);
                    if (suffix > max_suffix) {
                        max_suffix = suffix;
                    }
                } catch (...) {}
            }
        }
        file.close();
    }

    max_suffix++;
    stringstream final_id;
    final_id << prefix << setfill('0') << setw(3) << max_suffix;

    return final_id.str();
}

// ---> NOW ACCEPTS THE PASSWORD PARAMETER <---
string Add_Student::add_student(const string& name, const string& faculty, const string& password) {
    string new_id = generate_student_id();
    string email = new_id + "@student.eui.edu.eg";

    // Save to Student Database
    ofstream file(STUDENT_DB, ios::app);
    if (file.is_open()) {
        file << new_id << ","
             << name << ","
             << ","
             << ","
             << ","
             << ","
             << "," << "0," << ","
             << faculty << "," << "\n";
        file.close();
    }

    // Save to Login Database (Assuming format: ID, Email, Password, Role)
    ofstream loginFile(LOGIN_DB, ios::app);
    if (loginFile.is_open()) {
        loginFile << email << "," << password << "," << "STUDENT" << "," << name << "," << new_id << "\n";
        loginFile.close();
    }

    // Return success string
    return "Student Created Successfully!\nID: " + new_id + "\nEmail: " + email + "\nPassword: " + password;
}