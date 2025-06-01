#pragma once

#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

// Copies a file from source to destination. Returns true on success.
inline bool CreateApplicationFile(std::string_view from, std::string_view to)
{
    try
    {
        fs::copy_file(from, to, fs::copy_options::overwrite_existing);
    }
    catch (const std::exception &e)
    {
        wxLogError(wxString("Error copying file from ") + wxString(from) + " to " + wxString(to) + "\n" + wxString(e.what()));
        return false;
    }

    return fs::exists(to);
}

// Copies files from one directory to another, creating directories if needed.
inline bool CreateApplicationAssetsDirectories(
    std::string_view sourceDir,
    std::string_view baseTargetDir,
    std::string_view dirName)
{
    fs::path sourcePath(sourceDir);
    fs::path targetBase(baseTargetDir);
    fs::path targetPath = targetBase / dirName;

    if (dirName == "file_ext")
    {
        targetPath = (osName == "Windows")
                         ? (targetBase / "icons\\file_ext\\")
                         : (targetBase / "icons/file_ext/");
    }

    try
    {
        fs::create_directories(targetPath);

        for (const auto &entry : fs::directory_iterator(sourcePath))
        {
            if (fs::is_regular_file(entry.status()))
            {
                fs::path destination = targetPath / entry.path().filename();
                fs::copy_file(entry.path(), destination, fs::copy_options::overwrite_existing);
            }
        }
    }
    catch (const std::exception &e)
    {
        wxLogError(wxString("Error creating directory or copying files to ") + wxString(dirName) + "\n" + wxString(e.what()));
        return false;
    }

    return fs::exists(targetPath);
}