#pragma once

#include <curl/curl.h>
#include <functional>
#include <wx/string.h>
#include <wx/arrstr.h>

/**
 * @brief Handles downloading and installation of LSP servers
 * 
 * Provides functionality to download, extract, and install LSP servers
 * including Python-based servers via pip and binary servers via HTTP.
 */
class LspDownloader {
public:
    LspDownloader();
    ~LspDownloader();

    /**
     * @brief Progress callback function type
     * @param percent Progress percentage (0-100)
     * @param status Human-readable status message
     */
    using ProgressCallback = std::function<void(int percent, wxString status)>;
    
    /**
     * @brief Completion callback function type
     * @param success True if operation completed successfully
     * @param message Status or error message
     */
    using CompleteCallback = std::function<void(bool success, wxString message)>;

    /**
     * @brief Sets the progress callback
     * @param callback Function to call on progress updates
     */
    void SetOnProgress(ProgressCallback callback) { m_onProgress = callback; }
    
    /**
     * @brief Sets the completion callback
     * @param callback Function to call when operation completes
     */
    void SetOnComplete(CompleteCallback callback) { m_onComplete = callback; }

    /**
     * @brief Downloads and extracts a binary LSP server from URL
     * @param url Download URL
     * @param destDir Destination directory for extraction
     */
    void Download(const wxString& url, const wxString& destDir);

    /**
     * @brief Installs a Python package via pip in a virtual environment
     * @param packageName Name of the Python package to install
     * @param destDir Base directory for virtual environment
     */
    void InstallPythonPackage(const wxString& packageName, const wxString& destDir);

    /**
     * @brief Gets the LSP server installation directory
     * @return Path to the LSP directory
     */
    static wxString GetLspDir();

    /**
     * @brief Downloads and installs clangd
     */
    void DownloadClangd();

    /**
     * @brief Installs pylsp (Python LSP server) via pip
     */
    void InstallPylsp();

    /**
     * @brief Gets the path to the pylsp executable
     * @return Full path to pylsp or empty string if not found
     */
    static wxString GetPylspPath();

    /**
     * @brief Checks if pylsp is installed
     * @return True if pylsp is installed
     */
    static bool IsPylspInstalled();

    /**
     * @brief Gets the path to the clangd executable
     * @return Full path to clangd or empty string if not found
     */
    static wxString GetClangdPath();

    /**
     * @brief Checks if clangd is installed
     * @return True if clangd is installed
     */
    static bool IsClangdInstalled();

private:
    CURL* m_curl = nullptr;
    ProgressCallback m_onProgress;
    CompleteCallback m_onComplete;

    /**
     * @brief CURL write callback for file download
     */
    static size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata);

    /**
     * @brief CURL progress callback
     */
    static int ProgressCallback_(void* userdata, curl_off_t total, curl_off_t now,
                                 curl_off_t, curl_off_t);

    /**
     * @brief Extracts a ZIP archive
     * @param zipPath Path to ZIP file
     * @param destDir Destination directory
     * @return True if extraction succeeded
     */
    bool Extract(const wxString& zipPath, const wxString& destDir);

    /**
     * @brief Finds Python 3 executable
     * @return Path to Python 3 or empty string if not found
     */
    wxString FindPython3();

    /**
     * @brief Runs a command and captures output
     * @param cmd Command to run
     * @param output Captured output lines
     * @return Exit code of the command
     */
    long RunCommand(const wxString& cmd, wxArrayString& output);
};