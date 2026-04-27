#ifndef ADMIN_PROFILE_H
#define ADMIN_PROFILE_H
#include <string>
#include <iostream>

using namespace std;

class Admin_Profile {
        private:
            string name;
            string email;
            string position;
        // Private constructor for Singleton
    Admin_Profile(){}
        public:
            // Delete copy constructor and assignment operator to enforce Singleton
            Admin_Profile(const Admin_Profile&) = delete;
            void operator=(const Admin_Profile&) = delete;

            // Get the single instance
            static Admin_Profile& get_instance() {
                static Admin_Profile instance;
                return instance;
            }

            // Load data from the CSV based on the admin's email
            void load_profile(const string& current_email);
            void reset();
            string get_email_login() const;
            // Getters
            string get_name() const { return name; }
            string get_email() const { return email; }
            string get_position() const { return position; }

    };
#endif //ADMIN_PROFILE_H
