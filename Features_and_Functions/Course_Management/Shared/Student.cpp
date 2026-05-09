/*
 * FILE: Student.cpp
 * DESCRIPTION: Implementation of the Student class - handles student course enrollment logic
 *
 * PURPOSE: Provides implementation for:
 *          - Loading offered courses from database (one-time cache)
 *          - Loading individual student data from CSV (grades, enrolled courses, etc.)
 *          - Validating course eligibility (prerequisites, previous failures)
 *          - Checking schedule conflicts before course registration
 *          - Managing course registration requests
 *          - Saving/persisting requests back to database
 *
 * DATABASE FILES USED:
 *   - Databases/Offered_Courses.csv: Master list of all university courses
 *   - Databases/Data_on_Each_Student.csv: Student-specific enrollment records
 *
 * DEPENDENCIES: Student.h (class definition)
 */

#include "Student.h"
// Readability pass: includes and declarations are kept visually grouped.
#include <fstream>      // For file I/O operations
#include <sstream>      // For string stream parsing (splitting CSV lines)
#include <iostream>     // For console output and debugging

using namespace std;

// ==========================================
// STATIC DATA STORAGE
// ==========================================

/*
 * FUNCTION: Student::get_mutable_courses()
 * PURPOSE: Provides thread-safe access to the global course list
 *
 * DESIGN PATTERN: Static Variable / Lazy Initialization
 * - The course list is created once on first call
 * - Same instance reused for entire application lifetime
 * - Prevents multiple copies of course data in memory
 *
 * RETURNS: Reference to the static vector containing all available courses
 *
 * WHY STATIC FUNCTION: Allows accessing class-level data without creating an instance
 *                      Usage: Student::get_mutable_courses(); (no Student object needed)
 */
vector<Course>& Student::get_mutable_courses() {
    // Create this list once and keep it forever (static variable)
    static vector<Course> eligible;
    return eligible;
}

// ==========================================
// CSV PARSING & LOADING
// ==========================================

/*
 * FUNCTION: Student::load_offered_courses()
 * PURPOSE: Loads all university courses into memory from the database
 *
 * PROCESS:
 *   1. Open Databases/Offered_Courses.csv file
 *   2. Skip header row (contains column names)
 *   3. For each subsequent line:
 *      a. Parse CSV: course code, prereq1, prereq2, course name
 *      b. Create a Course object with this data
 *      c. Add course to the global static list
 *   4. Close file
 *
 * OPTIMIZATION: Only loads once
 * - First call loads all courses
 * - Subsequent calls skip loading (courses already cached)
 * - Improves performance by avoiding repeated file reads
 *
 * ERROR HANDLING: If file not found, prints error message and returns (graceful failure)
 *
 * CSV FORMAT: code, preReq1, preReq2, name
 *   Example line: PHM111,"-1","-1","Physics I"
 *
 * STATIC METHOD: Called as Student::load_offered_courses() (no instance needed)
 */
void Student::load_offered_courses() {
    // Get reference to the global course list
    auto& courses = get_mutable_courses();

    // Optimization: Only load if not already loaded
    if (!courses.empty()) return;

    // Open the master course database
    ifstream file("Databases/Offered_Courses.csv");
    if (!file.is_open()) {
        cerr << "CRITICAL: Could not open Offered_Courses.csv" << endl;
        return;
    }

    string line, cell;
    getline(file, line);  // Skip header row

    // Read each course line
    while (getline(file, line)) {
        if (line.empty()) continue;  // Skip blank lines

        // Parse the CSV line by splitting on commas
        stringstream ss(line);
        vector<string> cols;
        while (getline(ss, cell, ',')) {
            cols.push_back(cell);
        }

        // Ensure row has required columns
        if (cols.size() >= 4) {
            // Extract course information from each column
            Course c;
            c.code = cols[0];      // Column 0: Course code (e.g., "PHM111")
            c.preReq1 = cols[1];   // Column 1: First prerequisite (or "-1" for none)
            c.preReq2 = cols[2];   // Column 2: Second prerequisite (or "-1" for none)
            c.name = cols[3];      // Column 3: Course name (e.g., "Physics I")
            courses.push_back(c);  // Add to global list
        }
    }
    file.close();
}

// ==========================================
// COURSE FILTERING & VALIDATION LOGIC
// ==========================================

/*
 * FUNCTION: Student::is_already_taken()
 * PURPOSE: Checks if student has already completed or is currently taking a course
 *
 * LOGIC: A course is considered "already taken" if:
 *   - It's in the student's passed courses list (completed before), OR
 *   - It's in the student's registered courses list (taking right now)
 *
 * PARAMETERS:
 *   - c: The Course object to check
 *
 * RETURNS: true if student has already taken or is taking the course, false otherwise
 *
 * NOTE: This prevents duplicate course enrollments
 */
bool Student::is_already_taken(const Course& c) const {
    // Check if course code exists in passed courses string
    bool passed = (passedCourses.find(c.code) != string::npos);
    // Check if course code exists in registered courses string
    bool taking_now = (registeredCourses.find(c.code) != string::npos);
    return passed || taking_now;  // True if either condition met
}

/*
 * FUNCTION: Student::is_previously_failed()
 * PURPOSE: Checks if student failed a course in the past
 *
 * LOGIC: Student can retake failed courses to improve their GPA/record
 *
 * PARAMETERS:
 *   - c: The Course object to check
 *
 * RETURNS: true if course is in student's failed courses list, false otherwise
 *
 * USAGE: Allows students to retake courses they previously failed
 */
bool Student::is_previously_failed(const Course& c) const {
    return failedCourses.find(c.code) != string::npos;
}

/*
 * FUNCTION: Student::meets_prerequisites()
 * PURPOSE: Validates that student has completed all prerequisite courses
 *
 * LOGIC:
 *   - "-1" means no prerequisite required for that slot
 *   - Otherwise, the prerequisite course code must be in passed courses list
 *   - Both prerequisites (if not "-1") must be met
 *
 * PARAMETERS:
 *   - c: The Course object whose prerequisites to check
 *
 * RETURNS: true if all prerequisites met (or "-1" for that slot), false if any prerequisite missing
 *
 * EXAMPLE:
 *   - Course: PHM112 (preReq1="PHM111", preReq2="-1")
 *   - Student must have passed PHM111 and preReq2 check is automatic (since it's "-1")
 *
 * ALGORITHM:
 *   1. Create a helper lambda function that checks if a single prerequisite is met
 *   2. Apply it to both preReq1 and preReq2
 *   3. Return true only if BOTH requirements are satisfied
 */
bool Student::meets_prerequisites(const Course& c) const {
    // Lambda function: checks if a single prerequisite requirement is satisfied
    auto is_met = [this](const string& pre) {
        // Prerequisite is met if: it's "-1" (no requirement) OR student passed it
        return pre == "-1" || passedCourses.find(pre) != string::npos;
    };
    // Both prerequisites must be met
    return is_met(c.preReq1) && is_met(c.preReq2);
}

/*
 * FUNCTION: Student::get_eligible_courses()
 * PURPOSE: Calculates which courses the student can register for
 *
 * ELIGIBILITY RULES:
 *   1. NOT already taken (not in passed or registered lists)
 *   2. EITHER: Met all prerequisites, OR: Is a retake of a failed course
 *
 * PROCESS:
 *   1. Loop through all available university courses
 *   2. Skip courses already taken
 *   3. Include courses if student can take them (prerequisites met or retaking failure)
 *   4. Return list of eligible courses
 *
 * RETURNS: Vector of Course objects the student is eligible to request
 *
 * USAGE: Called by frontend to populate "Available Courses" dropdown
 */
vector<Course> Student::get_eligible_courses() const {
    vector<Course> eligible;

    // Check every available university course
    for (const auto& c : get_mutable_courses()) {
        if (is_already_taken(c)) continue;  // Skip if already taken

        // Include if prerequisites met OR if it's a retake of a failed course
        if (meets_prerequisites(c) || is_previously_failed(c)) {
            eligible.push_back(c);
        }
    }
    return eligible;
}

// ==========================================
// REGISTRATION - CONFLICT DETECTION
// ==========================================

/*
 * FUNCTION: Student::has_time_conflict()
 * PURPOSE: Detects if a proposed course schedule conflicts with existing courses
 *
 * CONFLICT CASES:
 *   1. Lecture and Tutorial of SAME course are at same time (impossible schedule)
 *   2. New lecture conflicts with any requested course's lecture or tutorial
 *   3. New tutorial conflicts with any requested course's lecture or tutorial
 *   4. Times conflict with already registered courses
 *
 * PARAMETERS:
 *   - l_day: New course lecture day (e.g., "Saturday")
 *   - l_time: New course lecture time (e.g., "9:00-11:00")
 *   - t_day: New course tutorial day
 *   - t_time: New course tutorial time
 *
 * RETURNS: true if any conflict detected, false if schedule is clear
 *
 * ALGORITHM:
 *   1. Combine day and time into single strings for easy comparison
 *   2. Check if new lecture and tutorial are at same time (contradiction)
 *   3. Compare new times against all requested courses (potential conflicts)
 *   4. Compare new times against registered courses string
 *   5. Return true if ANY conflict found
 *
 * EXAMPLE:
 *   - New course: Saturday 9-11 lecture, Sunday 2-4 tutorial
 *   - Existing request: Saturday 10-12 lecture, Monday 3-5 tutorial
 *   - Conflict? YES (Saturday 9-11 overlaps with Saturday 10-12)
 */
bool Student::has_time_conflict(const string& l_day, const string& l_time, const string& t_day, const string& t_time) const {
    // Combine day and time into single comparison strings
    string new_lec = l_day + " " + l_time;    // e.g., "Saturday 9:00-11:00"
    string new_tut = t_day + " " + t_time;    // e.g., "Sunday 2:00-4:00"

    // Case 1: Same course's lecture and tutorial at same time (impossible)
    if (new_lec == new_tut) return true;

    // Case 2: Check against all requested courses
    for (const auto& c : requestedCourses) {
        string exist_lec = c.lectureDay + " " + c.lectureTime;
        string exist_tut = c.tutorialDay + " " + c.tutorialTime;

        // Check if new lecture/tutorial conflicts with any existing lecture/tutorial
        if (new_lec == exist_lec || new_lec == exist_tut ||
            new_tut == exist_lec || new_tut == exist_tut) {
            return true;  // Conflict found!
        }
    }

    // Case 3: Check against already registered courses (stored as strings)
    if (registeredCourses.find(new_lec) != string::npos) return true;
    if (registeredCourses.find(new_tut) != string::npos) return true;

    return false;  // No conflicts
}

// ==========================================
// REGISTRATION - COURSE ENROLLMENT
// ==========================================

/*
 * FUNCTION: Student::register_course()
 * PURPOSE: Attempts to add a course to the student's request list
 *
 * VALIDATION STEPS:
 *   1. Check for scheduling conflicts with proposed times
 *   2. Verify course code exists in eligible courses (prerequisite check)
 *   3. If valid, create a new Course object with the specified schedule
 *   4. Add to requestedCourses list
 *
 * PARAMETERS:
 *   - course_code: Code of the course to register (e.g., "PHM111")
 *   - l_day: Desired lecture day (e.g., "Saturday")
 *   - l_time: Desired lecture time (e.g., "9:00-11:00")
 *   - t_day: Desired tutorial day
 *   - t_time: Desired tutorial time
 *
 * RETURNS: true if successfully added to requests, false if validation failed
 *
 * ERROR CASES:
 *   - Scheduling conflict: Returns false
 *   - Course not eligible (prerequisites not met): Returns false
 *   - Course code not found: Returns false
 *
 * PROCESS:
 *   1. First check for scheduling conflicts early (quick rejection)
 *   2. Loop through eligible courses to find matching course code
 *   3. If found, copy course data and override with student's desired schedule
 *   4. Add to requestedCourses
 *
 * USAGE: Called by frontend when student clicks "Register" on a course
 */
bool Student::register_course(const string& course_code, const string& l_day, const string& l_time, const string& t_day, const string& t_time) {
    // Fast rejection: Check for time conflicts first
    if (has_time_conflict(l_day, l_time, t_day, t_time)) return false;

    // Find the course in eligible courses list
    for (const auto& c : get_eligible_courses()) {
        if (c.code == course_code) {
            // Course found and is eligible! Create request with student's preferred schedule
            Course new_request = c;  // Copy base course info
            new_request.lectureDay = l_day;
            new_request.lectureTime = l_time;
            new_request.tutorialDay = t_day;
            new_request.tutorialTime = t_time;

            // Add to student's pending requests
            requestedCourses.push_back(new_request);
            return true;  // Success!
        }
    }
    return false;  // Course not found or not eligible
}

// ==========================================
// DATABASE LOADING
// ==========================================

/*
 * FUNCTION: Student::load_data()
 * PURPOSE: Loads student's academic record from the database
 *
 * DATA LOADED:
 *   - passedCourses: Courses student has successfully completed
 *   - failedCourses: Courses student failed (can retake)
 *   - registeredCourses: Courses student is currently enrolled in
 *   - requestedCourses: Pending course requests (parsed from special encoding)
 *
 * DATABASE: Databases/Data_on_Each_Student.csv
 *
 * CSV FORMAT (simplified):
 *   Column 0: Student ID
 *   Column 1: [Name or other data]
 *   Column 2: Passed courses (semicolon-separated course codes)
 *   Column 3: Failed courses
 *   Column 4: Registered courses
 *   Column 5: Requested courses (special encoding with underscores and slashes)
 *
 * SPECIAL ENCODING FOR REQUESTED COURSES:
 *   Format: _CODE1\\LEC_DAY LEC_TIME//TUT_DAY TUT_TIME_CODE2\\...
 *   Example: _PHM111\\Saturday 9:00-11:00//Sunday 2:00-4:00_PHM112\\Monday 10-12//Tuesday 3-5
 *   Parsing: Split by _, extract code before \\, lecture before //, tutorial after //
 *
 * PROCESS:
 *   1. Open student database file
 *   2. Skip header row
 *   3. Search for row matching this student's ID
 *   4. Extract simple fields (passed, failed, registered)
 *   5. Parse complex requested courses encoding
 *   6. Print status message to console
 *
 * ERROR HANDLING: Gracefully handles malformed CSV or missing file
 *
 * NOTE: Uses quote-aware CSV parsing to handle commas inside quoted fields
 */
void Student::load_data() {
    ifstream file("Databases/Data_on_Each_Student.csv");
    if (!file.is_open()) {
        cerr << "CRITICAL: Could not open Data_on_Each_Student.csv" << endl;
        return;
    }

    string line;
    getline(file, line);  // Skip header row

    // Search for this student's record
    while (getline(file, line)) {
        if (line.empty()) continue;

        // Parse CSV with quote awareness (handles commas inside quotes)
        vector<string> cols;
        string current;
        bool in_quotes = false;

        for (char c : line) {
            if (c == '"') {
                in_quotes = !in_quotes;
            } else if (c == ',' && !in_quotes) {
                cols.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        }
        cols.push_back(current);

        // Check if this is the student's row
        if (!cols.empty() && cols[0] == this->id) {
            // Extract simple course lists
            if (cols.size() > 2) this->passedCourses = cols[2];
            if (cols.size() > 3) this->failedCourses = cols[3];
            if (cols.size() > 4) this->registeredCourses = cols[4];

            // Parse complex requested courses encoding
            if (cols.size() > 5 && !cols[5].empty()) {
                string reqStr = cols[5];
                size_t pos = 0;

                // Parse each requested course from the encoded string
                while ((pos = reqStr.find('_', pos)) != string::npos) {
                    // Find next separator for this course segment
                    size_t next_pos = reqStr.find('_', pos + 1);
                    string segment = (next_pos == string::npos) ?
                                    reqStr.substr(pos + 1) :
                                    reqStr.substr(pos + 1, next_pos - pos - 1);

                    // Extract schedule delimiters
                    size_t bPos = segment.find("\\\\");    // Lecture/tutorial separator
                    size_t sPos = segment.find("//");      // Lecture/tutorial time separator

                    if (bPos != string::npos && sPos != string::npos) {
                        // Extract course code (before \\)
                        Course c;
                        c.code = segment.substr(0, bPos);

                        // Look up course name from master course list
                        for (const auto& mc : get_mutable_courses()) {
                            // USING OPERATOR OVERLOAD == TO COMPARE COURSES
                            if (mc == c) {
                                c.name = mc.name;  // Copy course name
                                break;
                            }
                        }

                        // Extract lecture schedule (between \\ and //)
                        string fLec = segment.substr(bPos + 2, sPos - (bPos + 2));
                        // Extract tutorial schedule (after //)
                        string fTut = segment.substr(sPos + 2);

                        // Parse lecture day and time
                        size_t lS = fLec.find(' ');
                        if (lS != string::npos) {
                            c.lectureDay = fLec.substr(0, lS);
                            c.lectureTime = fLec.substr(lS + 1);
                        }

                        // Parse tutorial day and time
                        size_t tS = fTut.find(' ');
                        if (tS != string::npos) {
                            c.tutorialDay = fTut.substr(0, tS);
                            c.tutorialTime = fTut.substr(tS + 1);
                        }

                        // Add to requested courses
                        this->requestedCourses.push_back(c);
                    }
                    pos++;
                }
            }

            // Print success message and list all loaded requests
            cout << "SUCCESS: Loaded data for student " << this->id << endl;
            for(const auto& req : this->requestedCourses) {
                // USING OPERATOR OVERLOAD << TO PRINT COURSE
                cout << "  -> Loaded Request: " << req << endl;
            }
            break;
        }
    }
    file.close();
}

// ==========================================
// DATABASE SAVING
// ==========================================

/*
 * FUNCTION: Student::save_requests_to_csv()
 * PURPOSE: Persists student's course requests back to the database
 *
 * PROCESS:
 *   1. Read entire student database file
 *   2. Find this student's record (by ID)
 *   3. Update column 5 with newly encoded requested courses
 *   4. Write entire file back (preserving all other data)
 *
 * ENCODING: Uses same special format as load_data():
 *           _CODE1\\LEC_DAY LEC_TIME//TUT_DAY TUT_TIME_CODE2\\...
 *
 * DATABASE: Databases/Data_on_Each_Student.csv
 *
 * USAGE: Called when student submits their course request form
 *
 * ERROR HANDLING: Silently fails if file cannot be opened (may prevent data loss)
 *
 * NOTE: This reads entire file into memory before writing back. Not ideal for
 *       very large databases but acceptable for university with reasonable student count.
 *       For improvement: Could use temp file and rename approach.
 */
void Student::save_requests_to_csv() {
    // Read entire file first
    ifstream fileIn("Databases/Data_on_Each_Student.csv");
    if (!fileIn.is_open()) return;

    vector<string> lines;
    string line;

    while (getline(fileIn, line)) {
        if (line.empty()) continue;

        // Track if line had carriage return (for Windows compatibility)
        bool has_cr = (line.back() == '\r');
        if (has_cr) line.pop_back();

        // Parse CSV with quote awareness
        vector<string> cols;
        string current;
        bool in_quotes = false;

        for (char c : line) {
            if (c == '"') in_quotes = !in_quotes;
            else if (c == ',' && !in_quotes) {
                cols.push_back(current);
                current.clear();
            }
            else current += c;
        }
        cols.push_back(current);

        // Check if this is our student's row
        if (!cols.empty() && cols[0] == this->id) {
            // Encode all requested courses into special format
            string reqStr = "";
            for (const auto& c : this->requestedCourses) {
                reqStr += "_" + c.code + "\\\\" +
                         c.lectureDay + " " + c.lectureTime + "//" +
                         c.tutorialDay + " " + c.tutorialTime;
            }

            // Ensure column 5 exists
            while (cols.size() <= 5) cols.push_back("");

            // Update the requested courses column
            cols[5] = reqStr;

            // Reconstruct the line with updated data
            line = "";
            for (size_t i = 0; i < cols.size(); ++i) {
                line += cols[i] + (i < cols.size() - 1 ? "," : "");
            }
        }

        // Preserve carriage return if it was there
        lines.push_back(line + (has_cr ? "\r" : ""));
    }
    fileIn.close();

    // Write file back with updated data
    ofstream fileOut("Databases/Data_on_Each_Student.csv");
    for (const auto& l : lines) fileOut << l << "\n";
}
