/*
 * FILE: ImageManager.cpp
 * DESCRIPTION: Implementation of profile picture file management
 *
 * PURPOSE: Provides utility functions for:
 *          - Opening Windows file picker dialogs
 *          - Saving and storing user profile pictures
 *          - Retrieving stored profile pictures
 *          - Handling default profile pictures
 *
 * PLATFORM: Windows-specific code using Windows API
 *           Uses <windows.h> for file dialogs and GetOpenFileNameA()
 *
 * DATABASE LOCATION: Databases/Profile_Pics/ folder
 *
 * NAMING CONVENTION: User profile pictures are stored as {user_id}.{ext}
 *                    Example: "24-101229.png", "test@eui.edu.eg.jpg"
 *
 * DEPENDENCIES:
 *   - ImageManager.h (header)
 *   - Windows API (GetOpenFileNameA)
 *   - std::filesystem (file operations)
 */

#include "ImageManager.h"
// Readability pass: includes and declarations are kept visually grouped.
#include <iostream>        // For console output and error reporting
#include <windows.h>       // For Windows file picker dialog API (GetOpenFileNameA)
#include <filesystem>      // For modern C++ file system operations

// Create an alias for filesystem to make code shorter
namespace fs = std::filesystem;

// ==========================================
// FILE PICKER DIALOG
// ==========================================

/*
 * FUNCTION: ImageManager::select_image_dialog()
 * PURPOSE: Opens a Windows file picker dialog for user to select an image
 *
 * PROCESS:
 *   1. Initialize Windows OPENFILENAME structure (dialog configuration)
 *   2. Configure dialog to only show image files (.png, .jpg, .jpeg)
 *   3. Show dialog to user (blocking call)
 *   4. Return selected file path if user clicked OK, empty string if cancelled
 *
 * DIALOG SETTINGS:
 *   - Filter: Only shows .png, .jpg, .jpeg files
 *   - Validation: Requires path and file to exist
 *   - Behavior: Doesn't change current directory, doesn't dereference links
 *
 * WINDOWS API DETAILS:
 *   - OPENFILENAMEA: Structure containing dialog configuration (A = ANSI strings)
 *   - ZeroMemory: Clears all memory to safe defaults
 *   - GetActiveWindow: Keeps dialog on top of our application
 *   - GetOpenFileNameA: Shows the file picker and returns result
 *
 * RETURNS:
 *   - Full path to selected image if user chose OK (e.g., "C:\\Users\\john\\photo.png")
 *   - Empty string "" if user cancelled or dialog failed
 *
 * USAGE:
 *   std::string path = ImageManager::select_image_dialog();
 *   if (!path.empty()) {
 *       // User selected a file, proceed with saving
 *   }
 */
std::string ImageManager::select_image_dialog() {
    // ==========================================
    // DIALOG CONFIGURATION STRUCTURE
    // ==========================================
    OPENFILENAMEA ofn;          // File picker dialog configuration
    char szFile[260] = { 0 };   // Buffer to store selected file path (max 260 chars for Windows)

    // Initialize dialog structure with zeros (all fields to defaults)
    ZeroMemory(&ofn, sizeof(ofn));

    // ==========================================
    // DIALOG SETTINGS
    // ==========================================
    ofn.lStructSize = sizeof(ofn);              // Required: size of this structure
    ofn.hwndOwner = GetActiveWindow();          // Parent window (keeps dialog on top of app)
    ofn.lpstrFile = szFile;                     // Buffer that will hold the selected path
    ofn.nMaxFile = sizeof(szFile);              // Max size of path buffer (260 chars)

    // Filter string for file types to show
    // Format: "Display Name\0*.ext1;*.ext2\0"
    // This shows: "Image Files" filter with .png, .jpg, .jpeg
    ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg\0";
    ofn.nFilterIndex = 1;                       // Start with first filter (1-based index)

    // Dialog behavior flags
    ofn.Flags = OFN_PATHMUSTEXIST |             // Path must exist (prevents typing invalid paths)
                OFN_FILEMUSTEXIST |             // File must exist (must select existing file)
                OFN_NOCHANGEDIR |               // Don't change app's current directory
                OFN_NODEREFERENCELINKS;         // Don't follow shortcut links

    // ==========================================
    // SHOW DIALOG AND RETURN RESULT
    // ==========================================
    // GetOpenFileNameA: Shows dialog, returns true if OK clicked, false if Cancelled
    if (GetOpenFileNameA(&ofn)) {
        // User clicked OK - return the selected file path
        return std::string(szFile);
    }

    // User clicked Cancel - return empty string
    return "";
}

// ==========================================
// IMAGE STORAGE & MANAGEMENT
// ==========================================

/*
 * FUNCTION: ImageManager::save_profile_picture()
 * PURPOSE: Copies user's selected image to the database folder and renames it
 *
 * PROCESS:
 *   1. Ensure target directory exists, create if needed
 *   2. Delete any old profile pictures for this user (prevent conflicts)
 *   3. Determine file extension from source
 *   4. Copy file from user's location to Databases/Profile_Pics/{user_id}.{ext}
 *   5. Return the path to the newly saved file
 *
 * NAMING CONVENTION:
 *   - All profile pictures stored as: {user_id}.{extension}
 *   - Examples: "24-101229.png", "test@eui.edu.eg.jpg"
 *   - This allows easy lookup by user_id
 *
 * CLEANUP STRATEGY:
 *   - Before saving, delete any existing pictures for this user
 *   - Tries all extensions (.png, .jpg, .jpeg) to find old file
 *   - Prevents multiple old versions piling up
 *
 * ERROR HANDLING:
 *   - Catches filesystem exceptions (invalid paths, permission denied, etc.)
 *   - Prints error message to console
 *   - Returns empty string on failure
 *
 * PARAMETERS:
 *   - source_path: Full path to the image user selected (e.g., "C:\\Users\\john\\photo.png")
 *   - user_id: User's unique identifier (e.g., "24-101229" or "john@eui.edu.eg")
 *
 * RETURNS:
 *   - Success: Path to the newly saved file (e.g., "Databases/Profile_Pics/24-101229.png")
 *   - Failure: Empty string ""
 *
 * USAGE:
 *   std::string result = ImageManager::save_profile_picture(selected_path, student_id);
 *   if (!result.empty()) {
 *       // Picture saved successfully at result path
 *   }
 */
std::string ImageManager::save_profile_picture(const std::string& source_path, const std::string& user_id) {
    try {
        // ==========================================
        // SETUP TARGET DIRECTORY
        // ==========================================
        std::string target_dir = "Databases/Profile_Pics/";

        // Create directory if it doesn't exist yet
        if (!fs::exists(target_dir)) {
            fs::create_directories(target_dir);
        }

        // ==========================================
        // CLEANUP OLD PROFILE PICTURES
        // ==========================================
        // Delete any existing pictures for this user (try all possible extensions)
        // This prevents accumulating old versions and naming conflicts
        for (const std::string& ext : {".png", ".jpg", ".jpeg"}) {
            std::string old_file = target_dir + user_id + ext;
            if (fs::exists(old_file)) {
                fs::remove(old_file);  // Physically delete the old file from disk
            }
        }

        // ==========================================
        // PREPARE NEW FILENAME AND PATH
        // ==========================================
        // Extract file extension from source (preserves original format)
        std::string extension = fs::path(source_path).extension().string();
        // Create new filename using user_id and original extension
        std::string new_filename = user_id + extension;
        // Build full destination path
        std::string final_path = target_dir + new_filename;

        // Safety check: if somehow file already exists, delete it first
        if (fs::exists(final_path)) {
            fs::remove(final_path);
        }

        // ==========================================
        // COPY FILE TO DESTINATION
        // ==========================================
        // Copy from user's selected location to database folder
        fs::copy_file(source_path, final_path);

        // Return the path where file was saved
        return final_path;

    } catch (const std::exception& e) {
        // Print detailed error if operation failed
        std::cerr << "File Error: " << e.what() << std::endl;
        return "";  // Return empty string to indicate failure
    }
}

// ==========================================
// PROFILE PICTURE RETRIEVAL
// ==========================================

/*
 * FUNCTION: ImageManager::get_user_pfp_path()
 * PURPOSE: Retrieves the path to a user's profile picture (or default if none exists)
 *
 * PROCESS:
 *   1. Search for profile picture with user_id and any supported extension
 *   2. If found, return the path
 *   3. If not found, return path to default profile picture
 *
 * SEARCH LOGIC:
 *   - Looks for: Databases/Profile_Pics/{user_id}.png
 *   - Then tries: Databases/Profile_Pics/{user_id}.jpg
 *   - Then tries: Databases/Profile_Pics/{user_id}.jpeg
 *   - If none found: Returns Databases/Profile_Pics/default_pic.jpeg
 *
 * FALLBACK STRATEGY:
 *   - Default picture used when user hasn't uploaded custom picture
 *   - Prevents errors when trying to display missing pictures
 *   - Users see a placeholder avatar instead
 *
 * DEBUGGING:
 *   - Prints debug info to console showing path lookup progress
 *   - Indicates if default picture found or if it's missing
 *   - Helps troubleshooting when pictures don't display
 *
 * PARAMETERS:
 *   - user_id: The user's unique identifier
 *
 * RETURNS:
 *   - Success: Path to user's custom picture (e.g., "Databases/Profile_Pics/24-101229.png")
 *   - Fallback: Path to default picture (e.g., "Databases/Profile_Pics/default_pic.jpeg")
 *
 * USAGE:
 *   std::string pic_path = ImageManager::get_user_pfp_path(student_id);
 *   // pic_path now contains path to either custom or default picture
 *   // Display this image in the UI
 */
std::string ImageManager::get_user_pfp_path(const std::string& user_id) {
    // Build base path without extension (will try different extensions)
    std::string base_path = "Databases/Profile_Pics/" + user_id;

    // ==========================================
    // TRY TO FIND CUSTOM PROFILE PICTURE
    // ==========================================
    // Loop through supported image formats
    for (const std::string& ext : {".png", ".jpg", ".jpeg"}) {
        // Try this extension
        if (fs::exists(base_path + ext)) {
            // Found it! Return the path with this extension
            return base_path + ext;
        }
    }

    // ==========================================
    // NO CUSTOM PICTURE - USE DEFAULT
    // ==========================================
    std::string default_path = "Databases/Profile_Pics/default_pic.jpeg";

    // Print debug information to help with troubleshooting
    std::cout << "\n--- PICTURE DEBUG ---" << std::endl;
    std::cout << "Trying to load default picture from: " << std::filesystem::absolute(default_path) << std::endl;

    // Check if default picture file exists
    if (!fs::exists(default_path)) {
        // Warning: default picture is missing!
        std::cerr << "ERROR: The file DOES NOT exist at that location!" << std::endl;
    } else {
        // Good: default picture found
        std::cout << "SUCCESS: The file exists!" << std::endl;
    }
    std::cout << "---------------------\n" << std::endl;

    // Return default picture path (even if it doesn't exist, let the UI handle the error)
    return default_path;
}