# EUI-SIS Upgraded
[![Ask DeepWiki](https://devin.ai/assets/askdeepwiki.png)](https://deepwiki.com/Ibrahim-hikal/EUI-SIS_-upgraded-)

## Overview

EUI-SIS Upgraded is a comprehensive Student Information System (SIS) built as a desktop application for Egypt University of Informatics. Developed with C++ and the modern Slint UI framework, this application provides a robust platform for managing all core academic operations. It features three distinct user portals—Student, Teacher, and Administrator—each with a tailored dashboard and functionalities designed to streamline university processes, from course registration and attendance tracking to grade management and end-of-semester processing.

## Key Features

### Student Portal
- **Profile Management:** View personal and academic details, including Name, ID, GPA, and Faculty.
- **Course Registration:** Browse available courses, view prerequisites, request enrollment in new courses with specific lecture/tutorial slots, and track the status of requests.
- **Academic Dashboard:** Access a real-time overview of registered courses, including instructor details, attendance records, and current grades.
- **Academic History:** Review a complete history of all previously passed and failed courses.
- **Weekly Schedule:** View a color-coded weekly timetable of all registered lectures and tutorials.
- **Academic Requests:** Submit requests for course withdrawal and attendance excuses directly to the administration.
- **Personalization:** Change your profile picture through an integrated file dialog.

### Teacher Portal
- **Dashboard:** View personal profile and a list of all assigned courses for the semester.
- **Attendance Management:** Easily take and update weekly attendance for students in each assigned course.
- **Gradebook Management:** Enter, update, and manage grades for all assessments, including quizzes, assignments, midterms, and finals.
- **Teaching Schedule:** View a personalized weekly timetable of all assigned lectures and tutorials.

### Administrator Portal
- **User Management:** Add new student and teacher accounts to the system with automated ID and email generation.
- **Course & Schedule Management:** Assign courses to teachers and define the specific lecture and tutorial time slots for each offering.
- **Registration Management:** Review, approve, and refuse course registration requests submitted by students.
- **Academic Request Processing:** Handle student-submitted requests for course withdrawals and attendance excuses.
- **End-of-Semester Processing:** A powerful one-click function to finalize the semester. This calculates final GPAs for all students, archives course results, and clears temporary data (like requests and registrations) to prepare the system for the next term.

## Technology Stack

- **Backend:** C++20
- **GUI:** [Slint](https://slint.dev/)
- **Build System:** CMake
- **Data Storage:** Local CSV files for portability and simplicity.

## Project Structure

The repository is organized to separate concerns, making it modular and maintainable.

- `main.cpp`: The core application entry point that initializes the UI and connects backend logic to UI callbacks.
- `/Features_and_Functions`: Contains all feature-specific C++ source code (`.cpp`, `.h`) and Slint UI definitions (`.slint`), organized by functionality (e.g., `Profile_Pages`, `Course_Management`, `Login_Page`).
- `/Databases`: Acts as the local database, storing all application data in a structured hierarchy of CSV files.
    - `/Databases/Courses`: Individual CSV files for each course's attendance and grades.
    - `/Databases/Profile_Pics`: Stores user-uploaded profile pictures.

## Getting Started

### Prerequisites

- A C++ compiler supporting C++20 (e.g., MSVC, GCC, Clang).
- CMake (version 3.21 or newer).
- The Slint C++ library. Please follow the official [Slint installation guide](https://slint.dev/).

### Building on Windows

The repository includes a batch script to automate the build process on Windows.

1.  **Clone the Repository:**
    ```bash
    git clone https://github.com/Ibrahim-hikal/EUI-SIS_-upgraded-.git
    cd EUI-SIS_-upgraded-
    ```

2.  **Verify Slint Installation:**
    Ensure that CMake can locate your Slint installation. You may need to set the `CMAKE_PREFIX_PATH` environment variable to point to your Slint library directory. The `CMakeLists.txt` file is configured to find the Slint package.

3.  **Run the Build Script:**
    Simply execute the `build.bat` script from the root directory of the project.
    ```batch
    .\build.bat
    ```
    This script will:
    - Create a `build` directory.
    - Configure the project using CMake.
    - Compile the source code into an executable.
    - Copy the final `EUI_SIS.exe` to the project's root folder.

4.  **Prepare for Launch:**
    As noted in the build script, you must copy the `slint_cpp.dll` from your Slint installation's `bin` directory and place it in the root folder of this project, next to `EUI_SIS.exe`.

5.  **Run the Application:**
    Double-click `EUI_SIS.exe` to start the application.
