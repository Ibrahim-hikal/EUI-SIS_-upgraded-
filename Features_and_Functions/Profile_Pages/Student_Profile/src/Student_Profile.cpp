#include "../header/Student_Profile.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

Student_Profile& Student_Profile::get_instance() {
    static Student_Profile instance;
    return instance;
}

// Robust parser to handle CSV fields that contain quotes and extra commas
vector<string> parse_csv_line(const string& line) {
    vector<string> result;
    string current;
    bool in_quotes = false;

    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (c == ',' && !in_quotes) {
            result.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    result.push_back(current);
    return result;
}

void Student_Profile::load_profile(const string& target_id) {
    ifstream file(STUDENT_DB);

    if (!file.is_open()) {
        // Push error text to the UI so you know it failed
        this->name = "DATABASE ERROR";
        this->id = "FILE NOT FOUND";
        this->faculty = "N/A";
        this->gpa = "N/A";
        return;
    }

    cout << "SUCCESS: File opened successfully!" << endl;

    string line;
    getline(file, line); // Skip the Header row

    bool found = false;

    while (getline(file, line)) {
        if (line.empty()) continue;

        vector<string> cols = parse_csv_line(line);

        // Take a quick peek at the first ID in the row for debugging
        if (!cols.empty()) {
            cout << "   -> Scanning CSV row, found ID: [" << cols[0] << "]" << endl;
        }

        // Based on your DB format: ID(0), Name(1), ... Faculty(9), GPA(10)
        if (cols.size() >= 10 && cols[0] == target_id) {
            this->id = cols[0];
            this->name = cols[1];
            this->faculty = cols[9];
            this->gpa = cols[10];

            cout << "MATCH FOUND! Loaded data for: " << this->name << endl;
            found = true;
            break;
        }
    }
    file.close();

    if (!found) {
        cout << "ERROR: File was read, but ID [" << target_id << "] was not found inside." << endl;
    }
    cout << "============================================\n" << endl;
}