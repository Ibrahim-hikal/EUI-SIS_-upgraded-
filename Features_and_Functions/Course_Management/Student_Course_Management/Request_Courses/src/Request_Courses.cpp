//-------------Request Courses Implementation (Binary Tree with Iterator Pattern)-------------

#include "../header/Request_Courses.h"

Request_Courses::Request_Courses(const std::string& student_id) {
    student = new Student(student_id);
    Student::load_offered_courses();
    student->load_data();
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
    if (student == nullptr) return false;

    // Verify course exists in tree
    Course* course = find_course(course_code);
    if (course == nullptr) return false;

    // Request the course through student
    return Student::register_course(course_code);
}

int Request_Courses::get_available_courses_count() const {
    return available_courses_tree.get_size();
}
