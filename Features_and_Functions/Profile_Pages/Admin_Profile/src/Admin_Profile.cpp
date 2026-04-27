#include "../header/Admin_Profile.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

using namespace std;

void Admin_Profile::load_profile(const string &current_email) {
    // 1. Clear old data and set the email immediately from the parameter
    reset();
    this->email = current_email;

    // 2. Open the specific Admin database
    // Note: Use "../Databases/..." if CLion doesn't see the folder
    ifstream file("Databases/Data_on_each_admin.csv");

    string line, cell;
    if (file.is_open()) {
        getline(file, line); // Skip header row

        while (getline(file, line)) {
            stringstream ss(line);
            vector<string> cols;
            while (getline(ss, cell, ',')) cols.push_back(cell);

            // Data_on_each_admin.csv structure: Name(0), Email(1), Position(2)
            if (cols.size() >= 3 && cols[1] == current_email) {
                this->name = cols[0];
                this->position = cols[2];
                break; // Found the admin, stop searching
            }
        }
        file.close();

        // 3. Debug output to confirm it worked
        cout << "\n[ADMIN DATA LOADED]" << endl;
        cout << "Name: " << this->name << endl;
        cout << "Position: " << this->position << endl;

    } else {
        cerr << "FILE ERROR: Could not open Databases/Data_on_each_admin.csv" << endl;
        this->name = "DATABASE ERROR";
        this->position = "FILE NOT FOUND";
    }
}

void Admin_Profile::reset() {
    this->name = "";
    this->email = "";
    this->position = "";
}