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
    ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS;

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
        for (const std::string& ext : {".png", ".jpg", ".jpeg"}) {
            std::string old_file = target_dir + user_id + ext;
            if (fs::exists(old_file)) {
                fs::remove(old_file); // Destroy the old file so it doesn't conflict
            }
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
    std::string default_path = "Databases/Profile_Pics/default_pic.jpeg";

    std::cout << "\n--- PICTURE DEBUG ---" << std::endl;
    std::cout << "Trying to load default picture from: " << std::filesystem::absolute(default_path) << std::endl;

    if (!fs::exists(default_path)) {
        std::cerr << "ERROR: The file DOES NOT exist at that location!" << std::endl;
    } else {
        std::cout << "SUCCESS: The file exists!" << std::endl;
    }
    std::cout << "---------------------\n" << std::endl;
    return default_path;
}