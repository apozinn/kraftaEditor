#pragma once

#include <wx/string.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <string>

/**
 * @namespace FileOperations
 * @brief Provides cross-platform file system operations
 *
 * This namespace encapsulates common file system operations with proper error handling
 * and logging. All methods are thread-safe and provide consistent behavior
 * across different operating systems.
 */
namespace FileOperations
{
    /**
     * @brief Creates a new empty file
     * @param path The full path of the file to create
     * @return true if file was created successfully, false otherwise
     *
     * @throws std::runtime_error if catastrophic failure occurs
     * @note Will fail if:
     * - File already exists
     * - Path is invalid
     * - Permission denied
     * - Disk is full
     */
    bool CreateFileK(const wxString &path);

    /**
     * @brief Creates a new directory including parent directories if needed
     * @param path The full path of the directory to create
     * @return true if directory was created successfully, false otherwise
     *
     * @throws std::runtime_error if catastrophic failure occurs
     * @note Will fail if:
     * - Directory already exists
     * - Path is invalid
     * - Permission denied
     */
    bool CreateDir(const wxString &path);

    /**
     * @brief Permanently deletes a file
     * @param path The full path of the file to delete
     * @return true if file was deleted successfully, false otherwise
     *
     * @throws std::runtime_error if catastrophic failure occurs
     * @warning This operation is irreversible
     * @note Will fail if:
     * - File doesn't exist
     * - File is in use
     * - Permission denied
     */
    bool DeleteFileK(const wxString &path);

    /**
     * @brief Recursively deletes a directory and all its contents
     * @param path The full path of the directory to delete
     * @return true if directory was deleted successfully, false otherwise
     *
     * @throws std::runtime_error if catastrophic failure occurs
     * @warning This operation is permanent and recursive
     * @note Will fail if:
     * - Directory doesn't exist
     * - Any file in directory is in use
     * - Permission denied
     */
    bool DeleteDir(const wxString &path);

    /**
     * @brief Copies a file from source to destination
     * @param source Path of the file to copy
     * @param destination Target path for the copied file
     * @return true if copy succeeded, false otherwise
     *
     * @throws std::runtime_error if catastrophic failure occurs
     * @note Will fail if:
     * - Source file doesn't exist
     * - Destination path exists
     * - Permission denied
     * - Disk is full
     * @warning Will overwrite existing destination file if permitted
     */
    bool CopyFile(const std::string_view source, const std::string_view destination);

    /**
     * @brief Recursively copies directory contents to new location
     * @param sourceDir Path of the directory to copy
     * @param targetParentDir Parent directory where new copy will be created
     * @param dirName Name for the new copied directory
     * @return true if copy succeeded, false otherwise
     *
     * @throws std::runtime_error if catastrophic failure occurs
     * @note Will fail if:
     * - Source directory doesn't exist
     * - Target directory exists
     * - Permission denied
     * - Disk is full
     * @warning Preserves directory structure and overwrites existing files
     */
    bool CopyDirectoryContents(const std::string_view sourceDir,
                               const std::string_view targetParentDir,
                               const std::string_view dirName);

    /**
     * @brief Checks if a file is an image
     * @param path The full path of the file to check
     * @return true if file is an image, false otherwise
     */
    bool IsImageFile(const wxString &path);
};