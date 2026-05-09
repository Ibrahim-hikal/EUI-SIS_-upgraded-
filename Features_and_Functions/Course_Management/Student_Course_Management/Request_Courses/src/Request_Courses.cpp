#include "../header/Request_Courses.h"
// Readability pass: includes and declarations are kept visually grouped.
#include <stack>
#include <queue>
#include <iostream>
using namespace std;
// ==========================================
// HIDDEN DATA STRUCTURES (Nodes & Iterators)
// ==========================================
struct CourseTreeNode {
    Course course;
    CourseTreeNode* left;
    CourseTreeNode* right;
    explicit CourseTreeNode(const Course& c) : course(c), left(nullptr), right(nullptr) {}
};

class InOrderIterator : public CourseTreeIterator {
private:
    vector<Course*> courses;
    size_t current_index;
    void fill_stack(CourseTreeNode* node) {
        if (node == nullptr) return;
        fill_stack(node->left);
        courses.push_back(&node->course);
        fill_stack(node->right);
    }
public:
    explicit InOrderIterator(CourseTreeNode* root_node) : current_index(0) { fill_stack(root_node); }
    bool has_next() override { return current_index < courses.size(); }
    Course& next() override { return *courses[current_index++]; }
    void reset() override { current_index = 0; }
};

class PreOrderIterator : public CourseTreeIterator {
private:
    vector<Course*> courses;
    size_t current_index;
    void fill_stack(CourseTreeNode* node) {
        if (node == nullptr) return;
        courses.push_back(&node->course);
        fill_stack(node->left);
        fill_stack(node->right);
    }
public:
    explicit PreOrderIterator(CourseTreeNode* root_node) : current_index(0) { fill_stack(root_node); }
    bool has_next() override { return current_index < courses.size(); }
    Course& next() override { return *courses[current_index++]; }
    void reset() override { current_index = 0; }
};

class PostOrderIterator : public CourseTreeIterator {
private:
    vector<Course*> courses;
    size_t current_index;
    void fill_stack(CourseTreeNode* node) {
        if (node == nullptr) return;
        fill_stack(node->left);
        fill_stack(node->right);
        courses.push_back(&node->course);
    }
public:
    explicit PostOrderIterator(CourseTreeNode* root_node) : current_index(0) { fill_stack(root_node); }
    bool has_next() override { return current_index < courses.size(); }
    Course& next() override { return *courses[current_index++]; }
    void reset() override { current_index = 0; }
};

class LevelOrderIterator : public CourseTreeIterator {
private:
    vector<Course*> courses;
    size_t current_index;
    void fill_queue(CourseTreeNode* node) {
        if (node == nullptr) return;
        queue<CourseTreeNode*> bfs_queue;
        bfs_queue.push(node);
        while (!bfs_queue.empty()) {
            CourseTreeNode* current = bfs_queue.front();
            bfs_queue.pop();
            courses.push_back(&current->course);
            if (current->left != nullptr) bfs_queue.push(current->left);
            if (current->right != nullptr) bfs_queue.push(current->right);
        }
    }
public:
    explicit LevelOrderIterator(CourseTreeNode* root_node) : current_index(0) { fill_queue(root_node); }
    bool has_next() override { return current_index < courses.size(); }
    Course& next() override { return *courses[current_index++]; }
    void reset() override { current_index = 0; }
};


// ==========================================
// COURSE BINARY TREE IMPLEMENTATION
// ==========================================
CourseBinaryTree::CourseBinaryTree() : root(nullptr) {}

CourseBinaryTree::~CourseBinaryTree() { clear(); }

CourseTreeNode* CourseBinaryTree::insert_recursive(CourseTreeNode* node, const Course& course) {
    if (node == nullptr) return new CourseTreeNode(course);
    if (course.code < node->course.code) node->left = insert_recursive(node->left, course);
    else if (course.code > node->course.code) node->right = insert_recursive(node->right, course);
    return node;
}

CourseTreeNode* CourseBinaryTree::search_recursive(CourseTreeNode* node, const string& code) const {
    if (node == nullptr) return nullptr;
    if (code == node->course.code) return node;
    else if (code < node->course.code) return search_recursive(node->left, code);
    else return search_recursive(node->right, code);
}

CourseTreeNode* CourseBinaryTree::find_min(CourseTreeNode* node) const {
    while (node->left != nullptr) node = node->left;
    return node;
}

CourseTreeNode* CourseBinaryTree::delete_recursive(CourseTreeNode* node, const string& code) {
    if (node == nullptr) return nullptr;
    if (code < node->course.code) node->left = delete_recursive(node->left, code);
    else if (code > node->course.code) node->right = delete_recursive(node->right, code);
    else {
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

void CourseBinaryTree::delete_tree(CourseTreeNode* node) {
    if (node == nullptr) return;
    delete_tree(node->left);
    delete_tree(node->right);
    delete node;
}

int CourseBinaryTree::get_size_recursive(CourseTreeNode* node) const {
    if (node == nullptr) return 0;
    return 1 + get_size_recursive(node->left) + get_size_recursive(node->right);
}

void CourseBinaryTree::insert(const Course& course) { root = insert_recursive(root, course); }
Course* CourseBinaryTree::search(const string& code) const {
    CourseTreeNode* result = search_recursive(root, code);
    return (result != nullptr) ? &result->course : nullptr;
}
bool CourseBinaryTree::remove(const string& code) {
    CourseTreeNode* temp = root;
    root = delete_recursive(root, code);
    return temp != root;
}
void CourseBinaryTree::clear() { delete_tree(root); root = nullptr; }
int CourseBinaryTree::get_size() const { return get_size_recursive(root); }
bool CourseBinaryTree::is_empty() const { return root == nullptr; }

unique_ptr<CourseTreeIterator> CourseBinaryTree::create_inorder_iterator() const { return make_unique<InOrderIterator>(root); }
unique_ptr<CourseTreeIterator> CourseBinaryTree::create_preorder_iterator() const { return make_unique<PreOrderIterator>(root); }
unique_ptr<CourseTreeIterator> CourseBinaryTree::create_postorder_iterator() const { return make_unique<PostOrderIterator>(root); }
unique_ptr<CourseTreeIterator> CourseBinaryTree::create_levelorder_iterator() const { return make_unique<LevelOrderIterator>(root); }


// ==========================================
// REQUEST COURSES MANAGER IMPLEMENTATION
// ==========================================
Request_Courses::Request_Courses(const string& student_id) {
    student = new Student(student_id);
    Student::load_offered_courses(); 
    student->load_data();
    registered_count = count_registered_courses();
    load_available_courses();
}

Request_Courses::~Request_Courses() { delete student; }

void Request_Courses::load_available_courses() {
    if (student == nullptr) return;
    auto eligible_courses = student->get_eligible_courses();
    for (const auto& course : eligible_courses) {
        available_courses_tree.insert(course);
    }
}

int Request_Courses::count_registered_courses() {
    if (student == nullptr) return 0;
    const string& reg = student->registeredCourses;
    int count = 0;
    size_t pos = 0;
    while ((pos = reg.find('_', pos)) != string::npos) {
        count++;
        pos++;
    }
    return count;
}

unique_ptr<CourseTreeIterator> Request_Courses::get_available_courses_inorder() { return available_courses_tree.create_inorder_iterator(); }
unique_ptr<CourseTreeIterator> Request_Courses::get_available_courses_preorder() { return available_courses_tree.create_preorder_iterator(); }
unique_ptr<CourseTreeIterator> Request_Courses::get_available_courses_postorder() { return available_courses_tree.create_postorder_iterator(); }
unique_ptr<CourseTreeIterator> Request_Courses::get_available_courses_levelorder() { return available_courses_tree.create_levelorder_iterator(); }

Course* Request_Courses::find_course(const string& course_code) { return available_courses_tree.search(course_code); }

bool Request_Courses::request_course(const string& course_code, const string& l_day, const string& l_time, const string& t_day, const string& t_time) {
    if (registered_count >= MAX_COURSES_PER_SEMESTER) return false;

    // Attempt to register with the custom times
    bool success = student->register_course(course_code, l_day, l_time, t_day, t_time);

    if (success) {
        registered_count++;
        student->save_requests_to_csv();
    }
    return success;
}

int Request_Courses::get_available_courses_count() const { return available_courses_tree.get_size(); }