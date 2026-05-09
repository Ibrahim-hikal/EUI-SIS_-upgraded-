#include "../header/Add_Students.h"
// Readability pass: includes and declarations are kept visually grouped.
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <ctime>

#define STUDENT_DB "Databases/Data_on_Each_Student.csv"
#define LOGIN_DB "Databases/Login_Credentials.csv"

using namespace std;

void safe_append_student(const string& filename, const string& text) {
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

string Add_Student::generate_student_id() {
    // 1. Get dynamic current year (YY)
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    string yy = to_string((now->tm_year + 1900) % 100);

    ifstream file(STUDENT_DB);
    string line;
    int max_suffix = 0; // Starts at 0, first student will be 001

    if (file.is_open()) {
        getline(file, line); // Skip header
        while (getline(file, line)) {
            if (line.empty()) continue;
            size_t pos = line.find(',');
            if (pos != string::npos) {
                string id_str = line.substr(0, pos);

                // We only want to look at IDs from THIS year that match the 101 pattern
                string target_prefix = yy + "-101";
                if (id_str.find(target_prefix) == 0 && id_str.length() >= 9) {
                    try {
                        // Extract only the last 3 digits (the XXX part)
                        int suffix = stoi(id_str.substr(6));
                        if (suffix > max_suffix) max_suffix = suffix;
                    } catch (...) {}
                }
            }
        }
    }

    // 2. Format the new ID as YY-101XXX
    max_suffix++;
    string suffix_str = to_string(max_suffix);

    // Add leading zeros if it's less than 3 digits (e.g. turns '1' into '001')
    while(suffix_str.length() < 3) {
        suffix_str = "0" + suffix_str;
    }

    return yy + "-101" + suffix_str;
}

string Add_Student::add_student(const string& full_name, const string& faculty, const string& password) {
    string id = generate_student_id();

    string student_row = id + "," + full_name + ",-1,-1,-1,-1,,0,," + faculty + ",0.0\n";
    safe_append_student(STUDENT_DB, student_row);

    string email = id + "@students.eui.edu.eg";
    string login_row = email + "," + password + ",STUDENT," + full_name + "," + id + "\n";
    safe_append_student(LOGIN_DB, login_row);

    return "Student Added!\nID: " + id + "\nPassword: " + password;
}