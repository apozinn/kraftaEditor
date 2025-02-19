#pragma once

#include <exception>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

bool CreateApplicationAssetsDirectories(std::string target, std::string dirName)
{
    const std::filesystem::path assetsFiles{target};
    std::string error;
    for (auto const &dir_entry : std::filesystem::directory_iterator{assetsFiles})
    {
        fs::path sourceFile = dir_entry.path();
        fs::path targetParent = applicationPath + dirName;
        if (dirName == "file_ext")
        {
            if (osName == "Windows")
            {
                targetParent = applicationPath + "icons\\file_ext\\";
            }
            else
                targetParent = applicationPath + "icons/file_ext/";
        }
        auto target = targetParent / sourceFile.filename();

        try
        {
            fs::create_directories(targetParent);
            fs::copy_file(sourceFile, target, fs::copy_options::overwrite_existing);
        }
        catch (std::exception &e)
        {
            error = e.what();
        }
    }

    if (wxDirExists(target))
    {
        return true;
    }
    else
    {
        wxLogError(wxString("An error occurred while creating the " + dirName + " directory" + "\n" + error));
        return false;
    }
}