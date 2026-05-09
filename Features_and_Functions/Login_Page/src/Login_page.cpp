/*
 * FILE: Login_page.cpp
 * DESCRIPTION: Implementation of the user authentication system
 *
 * PURPOSE: Handles the actual process of:
 *          - Reading the user database from CSV files
 *          - Cleaning up and normalizing text for comparison
 *          - Verifying if entered email/ID and password are correct
 *          - Checking if user selected the correct portal
 *
 * DATABASE: Databases/Login_Credentials.csv
 *           CSV format: email, password, role, [other fields...], id
 *
 * DEPENDENCIES: Login_Page.h (header/interface definition)
 */

#include "../header/Login_Page.h"
// Readability pass: includes and declarations are kept visually grouped.
#include <fstream>       // For file input/output operations
#include <sstream>       // For string stream parsing (splitting CSV lines)
#include <vector>        // For storing CSV columns
#include <algorithm>     // For algorithm utilities (if needed)

// Define the path to the login database file
#define LOGIN_DB "Databases/Login_Credentials.csv"

using namespace std;

// ==========================================
// HELPER FUNCTIONS - TEXT NORMALIZATION
// ==========================================

/*
 * FUNCTION: clean_text()
 * PURPOSE: Normalizes a string for case-insensitive comparison
 *
 * OPERATIONS:
 *   1. Removes all whitespace (spaces, tabs, newlines, carriage returns)
 *   2. Converts uppercase letters to lowercase using ASCII math
 *      ASCII values: 'A'=65, 'a'=97, so uppercase - 32 = lowercase
 *
 * USAGE: Enables email comparisons to be case-insensitive (user convenience)
 *        Example: "John@EUI.edu.eg" -> "john@eui.edu.eg"
 *
 * PARAMETERS:
 *   - text: The input string to clean
 *
 * RETURNS: A normalized string (lowercase, no whitespace)
 *
 * SECURITY NOTE: Passwords are NOT cleaned, so "MyPass" != "mypass"
 *                This preserves password case-sensitivity for security
 */
static string clean_text(string text) {
    string result = "";

    // Loop through every single character in the word
    for (int i = 0; i < text.length(); i++) {
        char c = text[i];

        // 1. Skip over any spaces, tabs, or invisible "Enter" key breaks
        // These characters: ' ' (space), '\n' (newline), '\r' (carriage return), '\t' (tab)
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
            continue; // Skip to the next letter
        }

        // 2. Convert Uppercase to Lowercase
        // ASCII math: 'A' is 65, 'a' is 97, so we add 32 to convert uppercase to lowercase
        if (c >= 'A' && c <= 'Z') {
            c = c + 32;  // Convert: A->a, B->b, ..., Z->z
        }

        // 3. Attach the clean letter to our final result
        result += c;
    }

    return result;
}

/*
 * FUNCTION: robust_match()
 * PURPOSE: Compares two strings with case-insensitive matching
 *
 * PROCESS: Cleans both strings first, then compares the results
 *
 * USAGE: Email addresses and IDs are compared this way for user convenience
 *
 * EXAMPLES:
 *   robust_match("john@eui.edu.eg", "JOHN@EUI.EDU.EG") -> true
 *   robust_match("john@eui.edu.eg", "john@eui.edu.eg") -> true
 *   robust_match("john@eui.edu.eg", "jane@eui.edu.eg") -> false
 *
 * PARAMETERS:
 *   - a: First string to compare
 *   - b: Second string to compare
 *
 * RETURNS: true if normalized versions are identical, false otherwise
 */
static inline bool robust_match(string a, string b) {
    return clean_text(a) == clean_text(b);
}

/*
 * FUNCTION: clean_edges()
 * PURPOSE: Removes leading/trailing whitespace and quotes from CSV fields
 *
 * WHY NEEDED: CSV files sometimes have formatting issues:
 *   - Extra spaces around values: "  email@example.com  "
 *   - Quoted values: "\"email@example.com\""
 *   - Carriage returns: "email@example.com\r"
 *
 * USAGE: Cleans each CSV field after splitting the line
 *
 * PROCESS:
 *   1. Find first non-whitespace/quote character
 *   2. Find last non-whitespace/quote character
 *   3. Extract substring between them
 *
 * PARAMETERS:
 *   - str: The CSV field to clean
 *
 * RETURNS: Cleaned string, or empty string if entire field is whitespace/quotes
 */
static inline string clean_edges(const string& str) {
    // Find the first character that is NOT whitespace or quote
    size_t first = str.find_first_not_of(" \t\r\n\"");

    // If no such character found, entire string is whitespace/quotes
    if (string::npos == first) return "";

    // Find the last character that is NOT whitespace or quote
    size_t last = str.find_last_not_of(" \t\r\n\"");

    // Extract and return the substring between first and last good characters
    return str.substr(first, (last - first + 1));
}

/*
 * FUNCTION: role_to_string()
 * PURPOSE: Converts an enumerated user role to its database text representation
 *
 * USAGE: When comparing database roles to the user's selected portal
 *
 * CONVERSION TABLE:
 *   User_Role::STUDENT -> "STUDENT"
 *   User_Role::TEACHER -> "TEACHER"
 *   User_Role::ADMIN   -> "ADMIN"
 *
 * PARAMETERS:
 *   - role: The enumerated role to convert
 *
 * RETURNS: The role as a string, or empty string if role not recognized
 */
static string role_to_string(User_Role role) {
    if (role == User_Role::STUDENT) return "STUDENT";
    if (role == User_Role::TEACHER) return "TEACHER";
    if (role == User_Role::ADMIN) return "ADMIN";
    return "";  // Unknown role
}

// ==========================================
// MAIN AUTHENTICATION FUNCTION
// ==========================================

/*
 * FUNCTION: Login_Manager::login()
 * PURPOSE: Authenticate a user by checking credentials against the database
 *
 * PROCESS FLOW:
 *   1. Attempt to open the login database CSV file
 *   2. Read and skip the header row (contains column names)
 *   3. For each subsequent row (each user):
 *      a. Parse the CSV line into individual fields
 *      b. Extract: email (col 0), password (col 1), role (col 2), id (col 4 if exists)
 *      c. Check if email OR id matches the input (case-insensitive)
 *      d. If match found, verify password (case-sensitive!)
 *      e. Verify role matches the portal user is trying to access
 *      f. Return SUCCESS if all checks pass
 *   4. Return USER_NOT_FOUND if loop completes without finding user
 *
 * ERROR HANDLING:
 *   - File not found: Returns USER_NOT_FOUND (same as user not in database)
 *   - Malformed CSV line: Safely skips and continues
 *
 * SECURITY CONSIDERATIONS:
 *   - Passwords are case-sensitive (IMPORTANT: preserves password case in database)
 *   - Emails/IDs are case-insensitive (improves user experience)
 *   - Invalid lines in CSV are skipped gracefully
 *   - No logs of failed attempts (to prevent username enumeration attacks)
 *
 * PARAMETERS:
 *   - email_or_id: User's email OR ID (function tries both)
 *   - password: User's password (case-sensitive check)
 *   - role: Portal type being accessed (STUDENT/TEACHER/ADMIN)
 *
 * RETURNS: Login_Status enumeration value indicating result:
 *          - SUCCESS: Credentials valid and role matches
 *          - USER_NOT_FOUND: Email/ID not in database
 *          - PASSWORD_INCORRECT: Email/ID found but password doesn't match
 *          - ROLE_MISMATCH: Credentials valid but user selected wrong portal
 */
Login_Status Login_Manager::login(const string& email_or_id, const string& password, User_Role role) {
    // Attempt to open the login database file
    ifstream file(LOGIN_DB);

    // If file cannot be opened (doesn't exist or permission denied)
    if (!file.is_open()) {
        return Login_Status::USER_NOT_FOUND;  // Treat as "user not found"
    }

    string line, cell;

    // Skip the header row (first line contains column names like "email,password,role,...")
    getline(file, line);

    // Read each subsequent line as a user record
    while (getline(file, line)) {
        // Skip empty lines that might appear in the CSV file
        if (line.empty()) continue;

        // Parse the CSV line into individual columns
        stringstream ss(line);      // Create a stream from the line
        vector<string> cols;        // Store each CSV field

        // Split the line by commas and collect each field
        while (getline(ss, cell, ',')) {
            cols.push_back(clean_edges(cell));  // Clean whitespace/quotes and store
        }

        // Ensure the row has enough columns (at least email, password, role)
        if (cols.size() >= 3) {

            // ==========================================
            // EMAIL/ID MATCHING
            // ==========================================
            // ---> THE FIX: Check BOTH the Email (Col 0) AND the ID (Col 4) <---
            // Some systems store ID in column 4, so we check both for flexibility

            // Try matching against email (column 0)
            bool is_match = robust_match(cols[0], email_or_id);

            // Also try matching against ID if it exists (column 4)
            if (cols.size() >= 5 && robust_match(cols[4], email_or_id)) {
                is_match = true;  // Accept if ID matches
            }

            // If we found a matching email or ID, proceed with verification
            if (is_match) {

                // ==========================================
                // PASSWORD VERIFICATION
                // ==========================================
                // Passwords are case-sensitive for security (unlike emails)
                // Must match EXACTLY as stored in database
                if (cols[1] != password) {
                    return Login_Status::PASSWORD_INCORRECT;  // Email found but password wrong
                }

                // ==========================================
                // ROLE/PORTAL VERIFICATION
                // ==========================================
                // Make sure user is accessing the correct portal
                // E.g., can't be a student trying to log into the teacher portal
                string expected_role = role_to_string(role);
                if (!robust_match(cols[2], expected_role)) {
                    return Login_Status::ROLE_MISMATCH;  // Wrong portal for this user
                }

                // ==========================================
                // SUCCESS - ALL CHECKS PASSED
                // ==========================================
                return Login_Status::SUCCESS;
            }
        }
    }

    // Loop completed without finding a matching user
    return Login_Status::USER_NOT_FOUND;
}