//-------------Request Courses Header File (Binary Tree with Iterator Pattern)-------------

#ifndef PROJECT_REQUEST_COURSES_H
#define PROJECT_REQUEST_COURSES_H

#include <string>
#include <memory>
#include <vector>
#include <stack>
#include <queue>
#include <stdexcept>
#include "../../../Admin_Course_Management/Accept_and_Refuse_Courses/header/Accept_and_Refuse_Courses.h"


// COURSE TREE NODE
struct CourseTreeNode {
    Course course;
    CourseTreeNode* left;
    CourseTreeNode* right;

    explicit CourseTreeNode(const Course& c)
        : course(c), left(nullptr), right(nullptr) {}
};


// COURSE TREE ITERATOR (Abstract Base Class)
class CourseTreeIterator {
public:
    virtual ~CourseTreeIterator() = default;

    virtual bool has_next() = 0;
    virtual Course& next() = 0;
    virtual void reset() = 0;
};


// IN-ORDER ITERATOR (Left-Root-Right) - Sorted by course code
class InOrderIterator : public CourseTreeIterator {
private:
    std::vector<Course*> courses;
    size_t current_index;

    void fill_stack(CourseTreeNode* node) {
        if (node == nullptr) return;
        fill_stack(node->left);
        courses.push_back(&node->course);
        fill_stack(node->right);
    }

public:
    explicit InOrderIterator(CourseTreeNode* root_node)
        : courses(), current_index(0) {
        fill_stack(root_node);
    }

    bool has_next() override {
        return current_index < courses.size();
    }

    Course& next() override {
        if (!has_next()) {
            throw std::out_of_range("No more elements in iterator");
        }
        return *courses[current_index++];
    }

    void reset() override {
        current_index = 0;
    }
};


// PRE-ORDER ITERATOR (Root-Left-Right)
class PreOrderIterator : public CourseTreeIterator {
private:
    std::vector<Course*> courses;
    size_t current_index;

    void fill_stack(CourseTreeNode* node) {
        if (node == nullptr) return;
        courses.push_back(&node->course);
        fill_stack(node->left);
        fill_stack(node->right);
    }

public:
    explicit PreOrderIterator(CourseTreeNode* root_node)
        : courses(), current_index(0) {
        fill_stack(root_node);
    }

    bool has_next() override {
        return current_index < courses.size();
    }

    Course& next() override {
        if (!has_next()) {
            throw std::out_of_range("No more elements in iterator");
        }
        return *courses[current_index++];
    }

    void reset() override {
        current_index = 0;
    }
};


// POST-ORDER ITERATOR (Left-Right-Root)
class PostOrderIterator : public CourseTreeIterator {
private:
    std::vector<Course*> courses;
    size_t current_index;

    void fill_stack(CourseTreeNode* node) {
        if (node == nullptr) return;
        fill_stack(node->left);
        fill_stack(node->right);
        courses.push_back(&node->course);
    }

public:
    explicit PostOrderIterator(CourseTreeNode* root_node)
        : courses(), current_index(0) {
        fill_stack(root_node);
    }

    bool has_next() override {
        return current_index < courses.size();
    }

    Course& next() override {
        if (!has_next()) {
            throw std::out_of_range("No more elements in iterator");
        }
        return *courses[current_index++];
    }

    void reset() override {
        current_index = 0;
    }
};


// LEVEL-ORDER ITERATOR (Breadth-First)
class LevelOrderIterator : public CourseTreeIterator {
private:
    std::vector<Course*> courses;
    size_t current_index;

    void fill_queue(CourseTreeNode* node) {
        if (node == nullptr) return;

        std::queue<CourseTreeNode*> bfs_queue;
        bfs_queue.push(node);

        while (!bfs_queue.empty()) {
            CourseTreeNode* current = bfs_queue.front();
            bfs_queue.pop();

            courses.push_back(&current->course);

            if (current->left != nullptr) {
                bfs_queue.push(current->left);
            }
            if (current->right != nullptr) {
                bfs_queue.push(current->right);
            }
        }
    }

public:
    explicit LevelOrderIterator(CourseTreeNode* root_node)
        : courses(), current_index(0) {
        fill_queue(root_node);
    }

    bool has_next() override {
        return current_index < courses.size();
    }

    Course& next() override {
        if (!has_next()) {
            throw std::out_of_range("No more elements in iterator");
        }
        return *courses[current_index++];
    }

    void reset() override {
        current_index = 0;
    }
};


// COURSE BINARY TREE
class CourseBinaryTree {
private:
    CourseTreeNode* root;

    static CourseTreeNode* insert_recursive(CourseTreeNode* node, const Course& course) {
        if (node == nullptr) {
            return new CourseTreeNode(course);
        }

        if (course.code < node->course.code) {
            node->left = insert_recursive(node->left, course);
        } else if (course.code > node->course.code) {
            node->right = insert_recursive(node->right, course);
        }

        return node;
    }

    static CourseTreeNode* search_recursive(CourseTreeNode* node, const std::string& code) {
        if (node == nullptr) {
            return nullptr;
        }

        if (code == node->course.code) {
            return node;
        } else if (code < node->course.code) {
            return search_recursive(node->left, code);
        } else {
            return search_recursive(node->right, code);
        }
    }

    static CourseTreeNode* find_min(CourseTreeNode* node) {
        while (node->left != nullptr) {
            node = node->left;
        }
        return node;
    }

    static CourseTreeNode* delete_recursive(CourseTreeNode* node, const std::string& code) {
        if (node == nullptr) {
            return nullptr;
        }

        if (code < node->course.code) {
            node->left = delete_recursive(node->left, code);
        } else if (code > node->course.code) {
            node->right = delete_recursive(node->right, code);
        } else {
            if (node->left == nullptr && node->right == nullptr) {
                delete node;
                return nullptr;
            }

            if (node->left == nullptr) {
                CourseTreeNode* temp = node->right;
                delete node;
                return temp;
            }

            if (node->right == nullptr) {
                CourseTreeNode* temp = node->left;
                delete node;
                return temp;
            }

            CourseTreeNode* min_node = find_min(node->right);
            node->course = min_node->course;
            node->right = delete_recursive(node->right, min_node->course.code);
        }

        return node;
    }

    static void delete_tree(CourseTreeNode* node) {
        if (node == nullptr) {
            return;
        }

        delete_tree(node->left);
        delete_tree(node->right);
        delete node;
    }

    static int get_size_recursive(CourseTreeNode* node) {
        if (node == nullptr) return 0;
        return 1 + get_size_recursive(node->left) + get_size_recursive(node->right);
    }

public:
    CourseBinaryTree() : root(nullptr) {}

    ~CourseBinaryTree() {
        delete_tree(root);
    }

    void insert(const Course& course) {
        root = insert_recursive(root, course);
    }

    [[nodiscard]] Course* search(const std::string& code) const {
        CourseTreeNode* result = search_recursive(root, code);
        return (result != nullptr) ? &result->course : nullptr;
    }

    bool remove(const std::string& code) {
        CourseTreeNode* temp = root;
        root = delete_recursive(root, code);
        return temp != root;
    }

    [[nodiscard]] CourseTreeNode* get_root() const { return root; }

    [[nodiscard]] bool is_empty() const { return root == nullptr; }

    void clear() {
        delete_tree(root);
        root = nullptr;
    }

    [[nodiscard]] int get_size() const {
        return get_size_recursive(root);
    }

    std::unique_ptr<CourseTreeIterator> create_inorder_iterator() {
        return std::make_unique<InOrderIterator>(root);
    }

    std::unique_ptr<CourseTreeIterator> create_preorder_iterator() {
        return std::make_unique<PreOrderIterator>(root);
    }

    std::unique_ptr<CourseTreeIterator> create_postorder_iterator() {
        return std::make_unique<PostOrderIterator>(root);
    }

    std::unique_ptr<CourseTreeIterator> create_levelorder_iterator() {
        return std::make_unique<LevelOrderIterator>(root);
    }
};

//---------------------------------------------------------------------------
// REQUEST COURSES (Main Class)
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

    // Get available courses using different traversal strategies
    std::unique_ptr<CourseTreeIterator> get_available_courses_inorder();
    std::unique_ptr<CourseTreeIterator> get_available_courses_preorder();
    std::unique_ptr<CourseTreeIterator> get_available_courses_postorder();
    std::unique_ptr<CourseTreeIterator> get_available_courses_levelorder();

    // Alternative: Get all courses (for backward compatibility if needed)
    [[nodiscard]] const std::vector<Course>& get_available_courses();

    // Search for a specific course
    Course* find_course(const std::string& course_code);

    // Request a course
    bool request_course(const std::string& course_code);

    // Get number of available courses
    [[nodiscard]] int get_available_courses_count() const;

    // Get student reference
    [[nodiscard]] Student* get_student() const { return student; }
};


#endif //PROJECT_REQUEST_COURSES_H
