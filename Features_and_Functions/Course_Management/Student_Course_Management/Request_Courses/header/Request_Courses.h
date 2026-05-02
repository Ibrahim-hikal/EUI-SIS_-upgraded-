#ifndef PROJECT_REQUEST_COURSES_H
#define PROJECT_REQUEST_COURSES_H

#include <string>
#include <memory>
#include <vector>
#include "../../../Shared/Student.h"

struct CourseTreeNode; // Forward declaration hides the mess

// 1. ITERATOR INTERFACE
class CourseTreeIterator {
public:
    virtual ~CourseTreeIterator() = default;
    virtual bool has_next() = 0;
    virtual Course& next() = 0;
    virtual void reset() = 0;
};

// 2. BINARY SEARCH TREE
class CourseBinaryTree {
private:
    CourseTreeNode* root;

    CourseTreeNode* insert_recursive(CourseTreeNode* node, const Course& course);
    CourseTreeNode* search_recursive(CourseTreeNode* node, const std::string& code) const;
    CourseTreeNode* find_min(CourseTreeNode* node) const;
    CourseTreeNode* delete_recursive(CourseTreeNode* node, const std::string& code);
    void delete_tree(CourseTreeNode* node);
    int get_size_recursive(CourseTreeNode* node) const;

public:
    CourseBinaryTree();
    ~CourseBinaryTree();

    void insert(const Course& course);
    Course* search(const std::string& code) const;
    bool remove(const std::string& code);
    void clear();
    int get_size() const;
    bool is_empty() const;

    std::unique_ptr<CourseTreeIterator> create_inorder_iterator() const;
    std::unique_ptr<CourseTreeIterator> create_preorder_iterator() const;
    std::unique_ptr<CourseTreeIterator> create_postorder_iterator() const;
    std::unique_ptr<CourseTreeIterator> create_levelorder_iterator() const;
};

// 3. REQUEST MANAGER
class Request_Courses {
private:
    Student* student;
    CourseBinaryTree available_courses_tree;
    int registered_count;
    static constexpr int MAX_COURSES_PER_SEMESTER = 5;

    void load_available_courses();
    int count_registered_courses();

public:
    explicit Request_Courses(const std::string& student_id);
    ~Request_Courses();

    std::unique_ptr<CourseTreeIterator> get_available_courses_inorder();
    std::unique_ptr<CourseTreeIterator> get_available_courses_preorder();
    std::unique_ptr<CourseTreeIterator> get_available_courses_postorder();
    std::unique_ptr<CourseTreeIterator> get_available_courses_levelorder();

    Course* find_course(const std::string& course_code);
    bool request_course(const std::string& course_code, const std::string& l_day, const std::string& l_time, const std::string& t_day, const std::string& t_time);
    int get_available_courses_count() const;
    Student* get_student() const { return student; }
};

#endif // PROJECT_REQUEST_COURSES_H