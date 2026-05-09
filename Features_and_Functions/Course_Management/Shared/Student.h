/*
 * FILE: Student.h
 * DESCRIPTION: Core data structures for representing university students and courses
 *
 * PURPOSE: Defines the Student and Course classes used throughout the system for:
 *          - Managing student academic records (grades, courses, prerequisites)
 *          - Storing course information (code, times, prerequisites)
 *          - Validating course eligibility and scheduling conflicts
 *
 * KEY RESPONSIBILITIES:
 *   - Student class: Loads student data from CSV, calculates eligibility, manages requests
 *   - Course class: Represents a single course offering with all scheduling info
 *
 * DEPENDENCIES: Student.cpp (implementation), various CSV database files
 */

#ifndef STUDENT_H
#define STUDENT_H

// Standard library includes for data structures and I/O
#include <string>       // For std::string - used for student/course data
#include <vector>       // For std::vector - stores lists of courses
#include <iostream>     // For standard output (debugging and console operations)

using namespace std;

// ==========================================
// COURSE STRUCTURE
// ==========================================
/*
 * CLASS: Course
 * PURPOSE: Represents a single course offering with its schedule and prerequisites
 *
 * ATTRIBUTES:
 *   - code: Unique identifier (e.g., "CSE101")
 *   - name: Full course title
 *   - preReq1/preReq2: Prerequisite course codes (may be empty)
 *   - lectureDay/Time: When the main lecture meets
 *   - tutorialDay/Time: When the tutorial/lab session meets
 */
class Course {
public:
    // ==========================================
    // COURSE PROPERTIES
    // ==========================================
    string code;            // Unique course identifier (e.g., "PHM111")
    string name;            // Course full name (e.g., "Physics I")
    string preReq1;         // First prerequisite course code (if any)
    string preReq2;         // Second prerequisite course code (if any)
    string lectureDay;      // Day of lecture (e.g., "Saturday", "Sunday")
    string tutorialDay;     // Day of tutorial/lab (may differ from lecture)
    string lectureTime;     // Lecture time slot (e.g., "9:00-11:00")
    string tutorialTime;    // Tutorial/lab time slot

    // ==========================================
    // OPERATOR OVERLOADING
    // ==========================================

    // Overload == operator to easily compare courses by their code
    // Enables: if (course1 == course2) { ... }
    bool operator==(const Course& other) const {
        return this->code == other.code;
    }

    // Overload << operator to easily print a course's details to console
    // Enables: cout << course << endl;  // Outputs "[CSE021] Data Structures"
    friend ostream& operator<<(ostream& os, const Course& c) {
        os << "[" << c.code << "] " << c.name;
        return os;
    }
};

// ==========================================
// STUDENT CLASS
// ==========================================
/*
 * CLASS: Student
 * PURPOSE: Represents a student's academic profile and course enrollment history
 *
 * RESPONSIBILITIES:
 *   - Load and store student academic records from database
 *   - Calculate which courses the student is eligible to take
 *   - Validate time conflicts in course schedules
 *   - Manage course registration requests
 *   - Track passed/failed/enrolled courses
 *
 * DATA SOURCES: Reads from CSV files in Databases/ directory
 */
class Student {
public:
    // ==========================================
    // PUBLIC PROPERTIES (Student Data)
    // ==========================================
    string id;                      // Unique student identifier
    string name;                    // Student's full name
    string passedCourses;           // Semicolon-separated list of passed course codes
    string failedCourses;           // Semicolon-separated list of failed course codes
    string registeredCourses;       // Semicolon-separated list of current registered courses
    vector<Course> requestedCourses; // Courses the student has requested to enroll in
    string excuseRequested;         // Semicolon-separated list of courses with excuses
    string totalExcuses;            // Total number of excuses used
    string withdrawalRequests;      // Semicolon-separated list of withdrawal requests
    string faculty;                 // Faculty/department (e.g., "Engineering")
    string gpa;                     // Current GPA (stored as string for easy CSV compatibility)

    // ==========================================
    // CONSTRUCTORS
    // ==========================================
    // Default constructor - creates an empty student object
    Student() = default;

    // Parameterized constructor - creates a student with an ID and loads their data
    explicit Student(string student_id) : id(move(student_id)) {}

    // ==========================================
    // PUBLIC METHODS - CORE FUNCTIONALITY
    // ==========================================

    // Static method that loads all university courses once into memory
    // Called once at startup to avoid repeated file reads
    static void load_offered_courses();

    // Loads this student's complete academic record from the database
    // Populates: id, name, passedCourses, failedCourses, registeredCourses, etc.
    void load_data();

    // Calculates which courses this student is eligible to take based on:
    //   - Prerequisites met (passed required courses)
    //   - Course not already taken or failed
    //   - Schedule doesn't conflict with registered courses
    // Returns: A vector of Course objects the student can request
    vector<Course> get_eligible_courses() const;

    // Checks if a proposed schedule has any time conflicts
    // Parameters:
    //   - l_day: Lecture day (e.g., "Saturday")
    //   - l_time: Lecture time (e.g., "9:00-11:00")
    //   - t_day: Tutorial day
    //   - t_time: Tutorial time
    // Returns: true if conflict found, false if schedule is clear
    bool has_time_conflict(const string &l_day, const string &l_time, const string &t_day, const string &t_time) const;

    // Attempts to add a course to the student's request list
    // Validates prerequisites and scheduling before accepting
    // Parameters:
    //   - course_code: The course to request (e.g., "CSE101")
    //   - l_day/l_time: Lecture schedule
    //   - t_day/t_time: Tutorial schedule
    // Returns: true if successfully added, false if invalid (reasons handled by backend)
    bool register_course(const string& course_code, const string& l_day, const string& l_time, const string& t_day, const string& t_time);

    // ==========================================
    // SAVING METHODS
    // ==========================================
    // Saves all pending course requests to the database CSV file
    // Called when student submits their course request form
    void save_requests_to_csv();

private:
    // ==========================================
    // PRIVATE METHODS - INTERNAL HELPERS
    // ==========================================

    // Static helper that provides mutable access to the global course list
    // Used internally for adding/updating courses
    static vector<Course>& get_mutable_courses();

    // ==========================================
    // PRIVATE VALIDATION METHODS
    // ==========================================

    // Checks if this course is already in the student's passed or registered list
    bool is_already_taken(const Course& c) const;

    // Checks if this course is in the student's failed courses list
    bool is_previously_failed(const Course& c) const;

    // Checks if the student has completed all prerequisite courses
    bool meets_prerequisites(const Course& c) const;
};

#endif // STUDENT_H