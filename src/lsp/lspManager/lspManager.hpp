#pragma once

#include <functional>
#include <map>
#include <memory>
#include <wx/string.h>
#include "lsp/lspDownloader/lspDownloader.hpp"
#include "lsp/lspClient/lspClient.hpp"
#include "ui/ids.hpp"

/**
 * @brief Manages LSP server installations and lifecycle
 * 
 * Singleton class that handles LSP server detection, installation,
 * and provides a unified interface for language server operations.
 */
class LspManager {
public:
    /**
     * @brief Gets the singleton instance
     * @return Reference to the LspManager instance
     */
    static LspManager& Get();

    /**
     * @brief Checks if a server is installed
     * @param serverName Name of the server (e.g., "clangd", "pylsp")
     * @return True if the server is installed
     */
    bool IsServerInstalled(const wxString& serverName);

    /**
     * @brief Gets the path to a server executable
     * @param serverName Name of the server
     * @return Full path to the server executable or empty string
     */
    wxString GetServerPath(const wxString& serverName);

    /**
     * @brief Verifies and optionally installs LSP for a language
     * @param languageName Display name of the language
     * @param languageLspName LSP server name (e.g., "pylsp", "clangd")
     * @param lspDownloadLink Download URL or "pip:" prefix for Python packages
     * @param onComplete Callback when verification/installation completes
     */
    void VerifyIfLanguageHasLsp(const wxString& languageName,
                                const wxString& languageLspName,
                                const wxString& lspDownloadLink,
                                std::function<void(bool)> onComplete);

    /**
     * @brief Verifies and installs Python LSP (pylsp)
     * @param onComplete Callback when verification/installation completes
     */
    void VerifyPythonLsp(std::function<void(bool)> onComplete);

    /**
     * @brief Verifies and installs C++ LSP (clangd)
     * @param onComplete Callback when verification/installation completes
     */
    void VerifyCppLsp(std::function<void(bool)> onComplete);

    /**
     * @brief Gets the LSP client for a server
     * @param serverName Name of the server
     * @return Pointer to the LSP client or nullptr if not available
     */
    LspClient* GetClient(const wxString& serverName);

    /**
     * @brief Starts an LSP server
     * @param serverName Name of the server
     * @param rootUri URI of the workspace root
     * @param onReady Callback when server is ready
     * @return True if server started successfully
     */
    bool StartServer(const wxString& serverName, const wxString& rootUri,
                     std::function<void()> onReady = nullptr);

    /**
     * @brief Stops an LSP server
     * @param serverName Name of the server
     */
    void StopServer(const wxString& serverName);

    /**
     * @brief Stops all running LSP servers
     */
    void StopAllServers();

private:
    LspManager();
    ~LspManager() = default;

    LspManager(const LspManager&) = delete;
    LspManager& operator=(const LspManager&) = delete;

    wxString m_lspFolderPath;
    std::map<wxString, std::unique_ptr<LspClient>> m_clients;
};