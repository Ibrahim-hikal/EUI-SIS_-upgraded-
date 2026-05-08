#ifndef ADMIN_PROFILE_H
#define ADMIN_PROFILE_H
#include <string>
using namespace std;
class Admin_Profile {
private:
    std::string name;
    std::string email;
    std::string position;

    Admin_Profile() = default;

public:
    static Admin_Profile& get_instance() {
        static Admin_Profile instance;
        return instance;
    }

    Admin_Profile(Admin_Profile const&) = delete;
    void operator=(Admin_Profile const&) = delete;

    void load_profile(const std::string &current_email);
    void reset();

    // New function for ending the semester
    string end_semester();

    std::string get_name() const { return name; }
    std::string get_email() const { return email; }
    std::string get_position() const { return position; }
};
#endif //ADMIN_PROFILE_H
