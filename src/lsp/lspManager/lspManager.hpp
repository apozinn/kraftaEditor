#pragma once

#include "lsp/lspClient/lspClient.hpp"
#include "lsp/lspDownloader/lspDownloader.hpp"
#include "ui/ids.hpp"

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include <wx/string.h>

class LspManager {
  public:
	static LspManager &Get();

	bool IsServerInstalled(const wxString &serverName);
	wxString GetServerPath(const wxString &serverName);

	void VerifyIfLanguageHasLsp(const wxString &languageName,
								const wxString &languageLspName,
								const wxString &lspDownloadLink,
								std::function<void(bool)> onComplete);

	void VerifyPythonLsp(std::function<void(bool)> onComplete);
	void VerifyCppLsp(std::function<void(bool)> onComplete);

	std::shared_ptr<LspClient> GetOrCreateClient(const wxString &workspaceRoot,
												 const wxString &serverPath,
												 const wxString &extraArgs);

	std::shared_ptr<LspClient> GetClient(const wxString &workspaceRoot) const;

	void ReleaseClient(const wxString &workspaceRoot);

	void ShutdownAll();

  private:
	LspManager();
	~LspManager() = default;

	LspManager(const LspManager &) = delete;
	LspManager &operator=(const LspManager &) = delete;

	wxString m_lspFolderPath;

	mutable std::mutex m_clientsMutex;
	std::unordered_map<std::string, std::shared_ptr<LspClient>> m_clients;
};