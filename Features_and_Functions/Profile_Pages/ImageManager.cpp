#include "ImageManager.h"
#include <iostream>
#include <windows.h>
#include <filesystem> // Ensure this is here

namespace fs = std::filesystem;

std::string ImageManager::select_image_dialog() {
    OPENFILENAMEA ofn;
    char szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow(); // Better than NULL, keeps dialog on top
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn)) {
        return std::string(szFile);
    }
    return "";
}

std::string ImageManager::save_profile_picture(const std::string& source_path, const std::string& user_id) {
    try {
        std::string target_dir = "Databases/Profile_Pics/";

        if (!fs::exists(target_dir)) {
            fs::create_directories(target_dir);
        }

        std::string extension = fs::path(source_path).extension().string();
        std::string new_filename = user_id + extension;
        std::string final_path = target_dir + new_filename;

        if (fs::exists(final_path)) {
            fs::remove(final_path); // Physically delete it from the disk
        }

        // Now copy the fresh one
        fs::copy_file(source_path, final_path);

        return final_path;
    } catch (const std::exception& e) {
        std::cerr << "File Error: " << e.what() << std::endl;
        return "";
    }
}

std::string ImageManager::get_user_pfp_path(const std::string& user_id) {
    std::string base_path = "Databases/Profile_Pics/" + user_id;

    for (const std::string& ext : {".png", ".jpg", ".jpeg"}) {
        if (fs::exists(base_path + ext)) {
            return base_path + ext;
        }
    }

    return "Databases/Profile_Pics/default_pic.png";
}