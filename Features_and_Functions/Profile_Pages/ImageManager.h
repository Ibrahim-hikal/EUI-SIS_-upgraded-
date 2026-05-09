/*
 * FILE: ImageManager.h
 * DESCRIPTION: Header file for profile picture management functionality
 *
 * PURPOSE: Handles all image-related operations for user profiles including:
 *          - Opening file picker dialogs for image selection
 *          - Saving and storing profile pictures in the database folder
 *          - Retrieving user profile pictures or defaults
 *
 * USAGE: This is a utility class with static methods only (no instantiation needed)
 *        Call methods like: ImageManager::save_profile_picture(path, id);
 */

#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

// Standard library includes for file system operations
#include <string>        // For std::string - used for file paths and user IDs
#include <filesystem>    // For file system operations and path manipulations


// ==========================================
// IMAGE MANAGER CLASS
// ==========================================
// Readability note: This class contains only static utility functions for image handling
// No instance variables are needed since all methods are static (utility pattern)
class ImageManager {
public:
    // ==========================================
    // IMAGE SELECTION & STORAGE METHODS
    // ==========================================

    // Opens the Windows File Picker dialog and returns the selected image file path
    // Returns: The absolute path to the selected image file, or empty string if cancelled
    static std::string select_image_dialog();

    // Copies an image from the user's selected location to the database folder
    // Renames it to {user_id}.png for consistency and easy lookup
    // Parameters:
    //   - source_path: The full path to the original image file
    //   - user_id: The unique identifier for the user (used for naming)
    // Returns: The relative path to the new image if successful, or empty string if failed
    static std::string save_profile_picture(const std::string& source_path, const std::string& user_id);

    // ==========================================
    // IMAGE RETRIEVAL METHODS
    // ==========================================

    // Locates and returns the path to a user's profile picture
    // If the user doesn't have a custom picture, returns the default avatar path
    // Parameters:
    //   - user_id: The unique identifier for the user to look up
    // Returns: The path to the user's profile picture, or the default image path
    static std::string get_user_pfp_path(const std::string& user_id);
};

#endif // IMAGEMANAGER_H