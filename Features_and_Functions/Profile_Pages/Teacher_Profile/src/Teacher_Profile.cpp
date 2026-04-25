#include "../header/Teacher_Profile.h"

Teacher_Profile& Teacher_Profile::get_instance() {
    static Teacher_Profile instance;
    return instance;
}

void Teacher_Profile::load_profile(const std::string& target_id) {
    this->id = target_id;
    this->email = target_id + "@eui.edu.eg";

    // TODO: Logic to put the target id as whoever teacher is signed in
    // For now, hardcoding an existing instructor to test the attendance UI
    this->name = "Ahmed Mahmoud";
}