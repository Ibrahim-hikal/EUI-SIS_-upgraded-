#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

#include <string>
#include <filesystem>


class ImageManager {
public:
    // Opens the Windows File Picker and returns the selected path
    static std::string select_image_dialog();

    // Copies selected image to the database folder and renames it
    // Returns the relative path to the new image if successful
    static std::string save_profile_picture(const std::string& source_path, const std::string& user_id);

    // Returns the path to the user's pfp or the default if not found
    static std::string get_user_pfp_path(const std::string& user_id);
};

#endif // IMAGEMANAGER_H