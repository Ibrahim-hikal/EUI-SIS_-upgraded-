#ifndef TEACHER_PROFILE_H
#define TEACHER_PROFILE_H

#include <string>

class Teacher_Profile {
private:
    std::string id;
    std::string name;
    std::string email;

    Teacher_Profile() = default;

public:
    static Teacher_Profile& get_instance();
    Teacher_Profile(const Teacher_Profile&) = delete;
    void operator=(const Teacher_Profile&) = delete;

    void load_profile(const std::string& target_id);

    std::string get_id() const { return id; }
    std::string get_name() const { return name; }
    std::string get_email() const { return email; }

    void reset() { id = ""; name = "Loading..."; email = ""; }
};
#endif //TEACHER_PROFILE_H