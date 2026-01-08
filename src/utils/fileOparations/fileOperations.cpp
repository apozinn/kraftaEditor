#include "fileOperations/fileOperations.hpp"
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/string.h>
#include <wx/log.h>
#include <filesystem>
#include <string>
#include <exception>
#include <system_error>
#include <unordered_set>

namespace fs = std::filesystem;

namespace FileOperations
{
    bool CreateFileK(const wxString &path)
    {
        if (path.empty())
        {
            wxLogWarning("Empty path provided for file creation");
            return false;
        }

        if (wxFileExists(path))
        {
            wxLogWarning("Operation aborted: File already exists at path '%s'", path);
            return false;
        }

        try
        {
            wxFile file;
            if (!file.Create(path, true)) // Exclusive create
            {
                wxLogError("Failed to create file at path '%s'", path);
                return false;
            }
            file.Close();
            wxLogStatus("Successfully created file at path '%s'", path);
            return true;
        }
        catch (const std::exception &e)
        {
            wxLogError("File creation failed for path '%s': %s", path, e.what());
            return false;
        }
    }

    bool CreateDir(const wxString &path)
    {
        if (path.empty())
        {
            wxLogWarning("Empty path provided for directory creation");
            return false;
        }

        if (wxDirExists(path))
        {
            wxLogWarning("Operation aborted: Directory already exists at path '%s'", path);
            return false;
        }

        try
        {
            if (!wxFileName::Mkdir(path, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
            {
                wxLogError("Directory creation failed for path '%s'", path);
                return false;
            }
            wxLogStatus("Successfully created directory at path '%s'", path);
            return true;
        }
        catch (const std::exception &e)
        {
            wxLogError("Directory creation failed for path '%s': %s", path, e.what());
            return false;
        }
    }

    bool DeleteFileK(const wxString &path)
    {
        if (path.empty())
        {
            wxLogWarning("Empty path provided for file deletion");
            return false;
        }

        if (!wxFileExists(path))
        {
            wxLogWarning("Operation aborted: File not found at path '%s'", path);
            return false;
        }

        try
        {
            if (!wxRemoveFile(path))
            {
                wxLogError("File deletion failed for path '%s'", path);
                return false;
            }
            wxLogStatus("Successfully deleted file at path '%s'", path);
            return true;
        }
        catch (const std::exception &e)
        {
            wxLogError("File deletion failed for path '%s': %s", path, e.what());
            return false;
        }
    }

    bool DeleteDir(const wxString &path)
    {
        if (path.empty())
        {
            wxLogWarning("Empty path provided for directory deletion");
            return false;
        }

        if (!wxDirExists(path))
        {
            wxLogWarning("Operation aborted: Directory not found at path '%s'", path);
            return false;
        }

        try
        {
            if (!wxFileName::Rmdir(path, wxPATH_RMDIR_RECURSIVE))
            {
                wxLogError("Directory deletion failed for path '%s'", path);
                return false;
            }
            wxLogStatus("Successfully deleted directory at path '%s'", path);
            return true;
        }
        catch (const std::exception &e)
        {
            wxLogError("Directory deletion failed for path '%s': %s", path, e.what());
            return false;
        }
    }

    bool CopyFile(const std::string_view source, const std::string_view destination)
    {
        if (source.empty() || destination.empty())
        {
            wxLogWarning("Empty path provided for file copy");
            return false;
        }

        try
        {
            const fs::path src(source);
            const fs::path dest(destination);

            if (!fs::exists(src))
            {
                wxLogError("Source file does not exist: %s", source.data());
                return false;
            }

            if (fs::equivalent(src, dest))
            {
                wxLogWarning("Source and destination are the same file");
                return false;
            }

            fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
            wxLogStatus("Successfully copied file from '%s' to '%s'",
                        source.data(), destination.data());
            return true;
        }
        catch (const fs::filesystem_error &e)
        {
            wxLogError("File copy failed: %s (code: %d)",
                       e.what(), e.code().value());
            return false;
        }
    }

    bool CopyDirectoryContents(
        const std::string_view sourceDir,
        const std::string_view targetParentDir,
        const std::string_view dirName)
    {
        if (sourceDir.empty() || targetParentDir.empty())
        {
            wxLogWarning("Empty path provided for directory copy");
            return false;
        }

        try
        {
            const fs::path source(sourceDir);
            fs::path target(targetParentDir);

            if (!fs::exists(source))
            {
                wxLogError("Source directory does not exist: %s", sourceDir.data());
                return false;
            }

            if (!dirName.empty())
            {
                target /= dirName;
            }

            if (fs::exists(target) && fs::equivalent(source, target))
            {
                wxLogWarning("Source and destination directories are the same");
                return false;
            }

            fs::create_directories(target);

            for (const auto &entry : fs::recursive_directory_iterator(source))
            {
                const auto relativePath = fs::relative(entry.path(), source);
                const auto destPath = target / relativePath;

                if (fs::is_directory(entry.status()))
                {
                    fs::create_directory(destPath);
                }
                else if (fs::is_regular_file(entry.status()))
                {
                    fs::copy_file(entry.path(), destPath,
                                  fs::copy_options::overwrite_existing);
                }
            }

            wxLogStatus("Successfully copied directory contents from '%s' to '%s'",
                        sourceDir.data(), target.string().c_str());
            return true;
        }
        catch (const fs::filesystem_error &e)
        {
            wxLogError("Directory copy failed: %s (code: %d)",
                       e.what(), e.code().value());
            return false;
        }
    }

    bool IsImageFile(const wxString &path)
    {
        wxFileName fn(path);
        wxString ext = fn.GetExt().Lower();

        static const std::unordered_set<wxString> imageExts = {
            "png", "jpg", "jpeg", "bmp", "gif", "tiff", "tif", "webp", "ico",

            "heif", "heic", "raw", "arw", "cr2", "cr3", "nef", "orf", "rw2", "dng",

            "svg", "svgz", "ai", "eps", "pdf",

            "exr", "hdr", "pfm",

            "tga", "pcx", "ppm", "pgm", "pbm", "pnm",
            "dds", "xbm", "xpm", "icns"};

        return imageExts.contains(ext);
    }
}