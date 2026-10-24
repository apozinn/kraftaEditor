#include "lspManager.hpp"

#include "appPaths/appPaths.hpp"
#include "lsp/lspClient/lspClient.hpp"
#include "lsp/lspDownloader/lspDownloader.hpp"
#include "lsp/lspInstallDialog/lspInstallDialog.hpp"
#include "platformInfos/platformInfos.hpp"
#include "userSettings/userSettings.hpp"

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/utils.h>

LspManager::LspManager() { m_lspFolderPath = LspDownloader::GetLspDir(); }

LspManager &LspManager::Get() {
	static LspManager instance;
	return instance;
}

std::shared_ptr<LspClient>
LspManager::GetOrCreateClient(const wxString &workspaceRoot,
							  const wxString &serverPath,
							  const wxString &extraArgs) {
	if (workspaceRoot.IsEmpty() || serverPath.IsEmpty())
		return nullptr;

	wxFileName fn(workspaceRoot);
	fn.MakeAbsolute();
	const std::string key = fn.GetFullPath().ToStdString();

	std::lock_guard<std::mutex> lock(m_clientsMutex);

	auto it = m_clients.find(key);
	if (it != m_clients.end()) {
		auto &client = it->second;
		if (client && client->IsRunning())
			return client;
		m_clients.erase(it);
	}

	auto client = std::make_shared<LspClient>();
	if (!client->Start(serverPath, extraArgs))
		return nullptr;

	m_clients[key] = client;
	return client;
}

std::shared_ptr<LspClient>
LspManager::GetClient(const wxString &workspaceRoot) const {
	if (workspaceRoot.IsEmpty())
		return nullptr;

	wxFileName fn(workspaceRoot);
	fn.MakeAbsolute();
	const std::string key = fn.GetFullPath().ToStdString();

	std::lock_guard<std::mutex> lock(m_clientsMutex);
	auto it = m_clients.find(key);
	if (it != m_clients.end() && it->second && it->second->IsRunning())
		return it->second;
	return nullptr;
}

void LspManager::ReleaseClient(const wxString &workspaceRoot) {
	if (workspaceRoot.IsEmpty())
		return;

	wxFileName fn(workspaceRoot);
	fn.MakeAbsolute();
	const std::string key = fn.GetFullPath().ToStdString();

	std::shared_ptr<LspClient> client;
	{
		std::lock_guard<std::mutex> lock(m_clientsMutex);
		auto it = m_clients.find(key);
		if (it == m_clients.end())
			return;
		client = it->second;
		m_clients.erase(it);
	}

	if (client)
		client->Stop();
}

void LspManager::ShutdownAll() {
	std::unordered_map<std::string, std::shared_ptr<LspClient>> copy;
	{
		std::lock_guard<std::mutex> lock(m_clientsMutex);
		copy.swap(m_clients);
	}

	for (auto &[key, client] : copy)
		if (client)
			client->Stop();
}

bool LspManager::IsServerInstalled(const wxString &serverName) {
	if (serverName == "clangd") {
		return LspDownloader::IsClangdInstalled();
	} else if (serverName == "pylsp" || serverName == "python-lsp-server") {
		return LspDownloader::IsPylspInstalled();
	} else if (serverName == "typescript-language-server") {
		return LspDownloader::IsTypescriptLanguageServerInstalled();
	}

	wxString serverDir = m_lspFolderPath + serverName;
	if (!wxDirExists(serverDir)) {
		return false;
	}

	wxDir dir(serverDir);
	if (!dir.IsOpened()) {
		return false;
	}

	wxString filename;
	bool cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
	while (cont) {
		wxString fullPath =
			serverDir + wxFileName::GetPathSeparator() + filename;
		if (wxFileExists(fullPath) && filename.Contains(serverName)) {
			return true;
		}
		cont = dir.GetNext(&filename);
	}

	return false;
}

wxString LspManager::GetServerPath(const wxString &serverName) {
	if (serverName == "clangd") {
		return LspDownloader::GetClangdPath();
	} else if (serverName == "pylsp" || serverName == "python-lsp-server") {
		return LspDownloader::GetPylspPath();
	} else if (serverName == "typescript-language-server") {
		return LspDownloader::GetTypescriptLanguageServerPath();
	}

	return "";
}

void LspManager::VerifyIfLanguageHasLsp(const wxString &languageName,
										const wxString &languageLspName,
										const wxString &lspDownloadLink,
										std::function<void(bool)> onComplete) {
	if (IsServerInstalled(languageLspName)) {
		auto statusBar =
			wxWindow::FindWindowById(+GUI::ControlID::StatusBarLSPStatus);
		if (statusBar) {
			auto text = dynamic_cast<wxStaticText *>(statusBar);
			if (text) {
				text->SetLabel("LSP Active(" + languageLspName + ")");
				text->GetParent()->Layout();
			}
		}

		if (onComplete)
			onComplete(true);
		return;
	}

	auto UserSettingsJson = UserSettingsManager::Get().currentSettings;
	if (UserSettingsJson.contains("prompts") &&
		UserSettingsJson["prompts"].contains("dontAskForInstallLspList")) {

		auto &lista = UserSettingsJson["prompts"]["dontAskForInstallLspList"];
		std::string lspStr = languageLspName.ToStdString();

		if (std::find(lista.begin(), lista.end(), lspStr) != lista.end()) {
			if (onComplete)
				onComplete(false);
			return;
		}
	}

	if (wxTheApp) {
		wxTheApp->CallAfter([this, languageName, languageLspName,
							 lspDownloadLink, onComplete]() {
			LspInstallDialog dlg(wxTheApp->GetTopWindow(), languageName,
								 languageLspName);

			if (dlg.ShowModal() == wxID_OK) {
				auto *downloader = new LspDownloader();

				downloader->SetOnProgress([](int percent, wxString status) {});

				downloader->SetOnComplete([downloader, languageLspName,
										   onComplete](bool success,
													   wxString message) {
					if (success) {
						wxMessageBox(message, "LSP Installed",
									 wxOK | wxICON_INFORMATION);
					} else {
						wxMessageBox("Failed: " + message, "Error",
									 wxOK | wxICON_ERROR);
					}

					wxTheApp->CallAfter([downloader]() { delete downloader; });

					if (onComplete)
						onComplete(success);
				});

				wxString destDir = m_lspFolderPath + languageLspName;

				if (languageLspName == "pylsp" ||
					languageLspName == "python-lsp-server") {
					downloader->InstallPythonPackage("python-lsp-server",
													 destDir);
				} else if (languageLspName == "typescript-language-server") {
					downloader->InstallTypescriptLanguageServer();
				} else if (lspDownloadLink.empty() ||
						   lspDownloadLink.StartsWith("pip:")) {
					wxString packageName = languageLspName;
					if (lspDownloadLink.StartsWith("pip:")) {
						packageName = lspDownloadLink.Mid(4);
					}
					downloader->InstallPythonPackage(packageName, destDir);
				} else {
					downloader->Download(lspDownloadLink, destDir);
				}
			} else {
				if (onComplete)
					onComplete(false);
			}
		});
	} else {
		if (onComplete)
			onComplete(false);
	}
}

void LspManager::VerifyPythonLsp(std::function<void(bool)> onComplete) {
	VerifyIfLanguageHasLsp("Python", "pylsp", "pip:python-lsp-server",
						   onComplete);
}

void LspManager::VerifyCppLsp(std::function<void(bool)> onComplete) {

	wxString path = ApplicationPaths::GetLanguagePreferencesPath("c++") +
					"preferences.json";
	std::ifstream preferencesFile(path.ToStdString());

	if (!preferencesFile) {
		wxMessageBox(_("Could not find the C++ LSP path."));
		return;
	}

	json languagePreferencesObject = json::parse(preferencesFile);
	std::string clangdPath = LspDownloader::GetLspDir().ToStdString();
	std::string clangdUrl;

	try {
		clangdPath =
			clangdPath +
			languagePreferencesObject["lsp"]["server"]["download"]["paths"]
									 [PlatformInfos::OsNameView()]
										 .get<std::string>();

		clangdUrl =
			languagePreferencesObject["lsp"]["server"]["download"]["urls"]
									 [PlatformInfos::OsNameView()];
	} catch (const std::exception &e) {
		wxMessageBox(_("Could not find the C++ LSP path."), e.what());
	}

	VerifyIfLanguageHasLsp("C++", "clangd", clangdUrl, onComplete);
}