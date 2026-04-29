//-------------Request Courses Implementation (Binary Tree with Iterator Pattern)-------------

#include "../header/Request_Courses.h"
#include <iostream>


Request_Courses::Request_Courses(const std::string& student_id) {
    student = new Student(student_id);
    Student::load_offered_courses();
    student->load_data();
    registered_count = count_registered_courses();
    load_available_courses();
}

Request_Courses::~Request_Courses() {
    delete student;
}

void Request_Courses::load_available_courses() {
    if (student == nullptr) return;

    // Get all eligible courses from student
    const auto& eligible_courses = Student::get_eligible_courses();

    // Insert all eligible courses into the binary tree (automatically sorted by course code)
    for (const auto& course : eligible_courses) {
        available_courses_tree.insert(course);
    }
}

int Request_Courses::count_registered_courses() {
    if (student == nullptr) return 0;

    const std::string& reg = student->registeredCourses;
    int count = 0;
    size_t pos = 0;
    while ((pos = reg.find('_', pos)) != std::string::npos) {
        count++;
        pos++;
    }
    return count;
}

std::unique_ptr<CourseTreeIterator> Request_Courses::get_available_courses_inorder() {
    return available_courses_tree.create_inorder_iterator();
}

std::unique_ptr<CourseTreeIterator> Request_Courses::get_available_courses_preorder() {
    return available_courses_tree.create_preorder_iterator();
}

std::unique_ptr<CourseTreeIterator> Request_Courses::get_available_courses_postorder() {
    return available_courses_tree.create_postorder_iterator();
}

std::unique_ptr<CourseTreeIterator> Request_Courses::get_available_courses_levelorder() {
    return available_courses_tree.create_levelorder_iterator();
}

const std::vector<Course>& Request_Courses::get_available_courses() {
    // For backward compatibility - returns the eligible courses
    return Student::get_eligible_courses();
}

Course* Request_Courses::find_course(const std::string& course_code) {
    return available_courses_tree.search(course_code);
}

bool Request_Courses::request_course(const std::string& course_code) {
    // 1. Check if the student has already hit the limit
    if (registered_count >= MAX_COURSES_PER_SEMESTER) {
        std::cout << "Cannot request course. Maximum limit of "
                  << MAX_COURSES_PER_SEMESTER << " courses reached." << std::endl;
        return false;
    }

    // 2. Make sure the course actually exists in the tree
    Course* target_course = find_course(course_code);
    if (target_course == nullptr) {
        std::cout << "Course not found." << std::endl;
        return false;
    }

    // Request the course through student
    bool success = Student::register_course(course_code);
    if (success) {
        registered_count++;
    }
    return success;
}

int Request_Courses::get_available_courses_count() const {
    return available_courses_tree.get_size();
}
