#include "../header/Add_Teachers.h"
// Readability pass: includes and declarations are kept visually grouped.
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>

// We are now using your exact database names!
#define TEACHER_DB "Databases/Data_on_each_teacher.csv"
#define LOGIN_DB "Databases/Login_Credentials.csv"

using namespace std;

// This guarantees the new entry is ALWAYS on a new row!
void safe_append_to_db(const string& filename, const string& text) {
    bool add_newline = false;
    ifstream read_file(filename);
    if (read_file.is_open()) {
        read_file.seekg(0, ios::end);
        if (read_file.tellg() > 0) {
            read_file.seekg(-1, ios::end);
            char last_char;
            read_file.get(last_char);
            if (last_char != '\n') add_newline = true;
        }
        read_file.close();
    }
    ofstream file(filename, ios::app);
    if (file.is_open()) {
        if (add_newline) file << "\n";
        file << text;
        file.close();
    }
}

string to_lowercase(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return tolower(c); });
    return s;
}

string Add_Teacher::add_teacher(const string& first_name, const string& last_name, const string& password) {
    string email = to_lowercase(first_name) + "." + to_lowercase(last_name) + "@eui.edu.eg";
    string full_name = first_name + " " + last_name;

    // 1. Teacher DB Format: Name,Email,Courses_Taught
    // We add an empty string "" for the courses initially
    string teacher_row = full_name + "," + email + ",\"\"\n";
    safe_append_to_db(TEACHER_DB, teacher_row);

    // 2. Login DB Format: email,password,login type,username,ID(empty)
    string login_row = email + "," + password + ",TEACHER," + full_name + ",\n";
    safe_append_to_db(LOGIN_DB, login_row);

    return "Teacher Added!\nName: " + full_name + "\nEmail: " + email + "\nPassword: " + password;
}