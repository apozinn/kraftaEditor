#include "lspManager.hpp"
#include "userSettings/userSettings.hpp"
#include <algorithm>
#include <wx/app.h>
#include <wx/dir.h>
#include <wx/msgdlg.h>

LspManager &LspManager::Get() {
	static LspManager instance;
	return instance;
}

LspManager::LspManager() { m_lspFolderPath = LspDownloader::GetLspDir(); }

bool LspManager::IsServerInstalled(const wxString &serverName) {
	if (serverName == "clangd") {
		return LspDownloader::IsClangdInstalled();
	} else if (serverName == "pylsp" || serverName == "python-lsp-server") {
		return LspDownloader::IsPylspInstalled();
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

				downloader->SetOnComplete(
					[this, downloader, languageLspName,
					 onComplete](bool success, wxString message) {
						if (success) {
							wxMessageBox(message, "LSP Installed",
										 wxOK | wxICON_INFORMATION);
						} else {
							wxMessageBox("Failed: " + message, "Error",
										 wxOK | wxICON_ERROR);
						}

						delete downloader;
						if (onComplete)
							onComplete(success);
					});

				wxString destDir = m_lspFolderPath + languageLspName;

				if (languageLspName == "pylsp" ||
					languageLspName == "python-lsp-server") {
					downloader->InstallPythonPackage("python-lsp-server",
													 destDir);
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
			languagePreferencesObject["lsp"]["server"]["download"]["url"]
									 [PlatformInfos::OsNameView()];
	} catch (const std::exception &e) {
		wxMessageBox(_("Could not find the C++ LSP path."), e.what());
	}

	VerifyIfLanguageHasLsp("C++", "clangd", clangdUrl, onComplete);
}

LspClient *LspManager::GetClient(const wxString &serverName) {
	auto it = m_clients.find(serverName);
	if (it != m_clients.end()) {
		return it->second.get();
	}
	return nullptr;
}

bool LspManager::StartServer(const wxString &serverName,
							 const wxString &rootUri,
							 std::function<void()> onReady) {
	wxString serverPath = GetServerPath(serverName);
	if (serverPath.empty()) {
		return false;
	}

	auto it = m_clients.find(serverName);
	if (it != m_clients.end() && it->second->IsRunning()) {
		return true;
	}

	auto client = std::make_unique<LspClient>();
	if (!client->Start(serverPath, "")) {
		return false;
	}

	LspClient *clientPtr = client.get();
	m_clients[serverName] = std::move(client);

	clientPtr->Initialize(rootUri, onReady);

	return true;
}

void LspManager::StopServer(const wxString &serverName) {
	auto it = m_clients.find(serverName);
	if (it != m_clients.end()) {
		it->second->Stop();
		m_clients.erase(it);
	}
}

void LspManager::StopAllServers() {
	for (auto &pair : m_clients) {
		pair.second->Stop();
	}
	m_clients.clear();
}