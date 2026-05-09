/*
 * File: Login_page.cpp
 * Description: Handles the actual process of reading the user database,
 * cleaning up the text, and verifying if the entered email/ID and password are correct.
 */

#include "../header/Login_Page.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

#define LOGIN_DB "Databases/Login_Credentials.csv"

using namespace std;

/*
 * Helper Tool: Cleans up a piece of text using basic C++ logic.
 * It removes spaces/line breaks and manually converts capital letters
 * to lowercase using ASCII math.
 */
static string clean_text(string text) {
    string result = "";

    // Loop through every single character in the word
    for (int i = 0; i < text.length(); i++) {
        char c = text[i];

        // 1. Skip over any spaces, tabs, or invisible "Enter" key breaks
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
            continue; // Skip to the next letter
        }

        // 2. Convert Uppercase to Lowercase (ASCII math: 'A' is 65, 'a' is 97, so we add 32 to convert)
        if (c >= 'A' && c <= 'Z') {
            c = c + 32;
        }

        // 3. Attach the clean letter to our final result
        result += c;
    }

    return result;
}

/*
 * Main Tool: Compares two strings by cleaning them both first.
 */
static inline bool robust_match(string a, string b) {
    return clean_text(a) == clean_text(b);
}

// Helper Tool: Removes invisible spaces and quotes from database cells
static inline string clean_edges(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n\"");
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n\"");
    return str.substr(first, (last - first + 1));
}

// Converts the user role number to the database text
static string role_to_string(User_Role role) {
    if (role == User_Role::STUDENT) return "STUDENT";
    if (role == User_Role::TEACHER) return "TEACHER";
    if (role == User_Role::ADMIN) return "ADMIN";
    return "";
}

/*
 * Scans the database to find the user's ID or Email.
 * If found, checks the password and ensures they selected the correct portal.
 */
Login_Status Login_Manager::login(const string& email_or_id, const string& password, User_Role role) {
    ifstream file(LOGIN_DB);

    if (!file.is_open()) {
        return Login_Status::USER_NOT_FOUND;
    }

    string line, cell;
    getline(file, line); // Skip the header row

    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        vector<string> cols;

        while (getline(ss, cell, ',')) {
            cols.push_back(clean_edges(cell));
        }

        if (cols.size() >= 3) {

            // ---> THE FIX: Check BOTH the Email (Col 0) AND the ID (Col 4) <---
            bool is_match = robust_match(cols[0], email_or_id);
            if (cols.size() >= 5 && robust_match(cols[4], email_or_id)) {
                is_match = true;
            }

            if (is_match) {

                // Passwords are case-sensitive, so we check them exactly as typed
                if (cols[1] != password) {
                    return Login_Status::PASSWORD_INCORRECT;
                }

                // Make sure they clicked the correct portal
                string expected_role = role_to_string(role);
                if (!robust_match(cols[2], expected_role)) {
                    return Login_Status::ROLE_MISMATCH;
                }

                return Login_Status::SUCCESS;
            }
        }
    }

    return Login_Status::USER_NOT_FOUND;
}