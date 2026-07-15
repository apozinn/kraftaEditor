#include "lspDownloader.hpp"
#include "appPaths/appPaths.hpp"
#include "platformInfos/platformInfos.hpp"
#include <fstream>
#include <thread>
#include <wx/app.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#ifdef _WIN32
#include <shellapi.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <stdio.h>

LspDownloader::LspDownloader() {
	curl_global_init(CURL_GLOBAL_DEFAULT);
	m_curl = curl_easy_init();
}

LspDownloader::~LspDownloader() {
	if (m_curl)
		curl_easy_cleanup(m_curl);
	curl_global_cleanup();
}

size_t LspDownloader::WriteCallback(void *ptr, size_t size, size_t nmemb,
									void *userdata) {
	auto *file = static_cast<FILE *>(userdata);
	return fwrite(ptr, size, nmemb, file);
}

int LspDownloader::ProgressCallback_(void *userdata, curl_off_t total,
									 curl_off_t now, curl_off_t, curl_off_t) {
	auto *self = static_cast<LspDownloader *>(userdata);
	if (!self->m_onProgress || total == 0)
		return 0;

	int percent = (int)((now * 100) / total);
	wxString status = wxString::Format("Downloading... %lld / %lld MB",
									   (long long)(now / (1024 * 1024)),
									   (long long)(total / (1024 * 1024)));

	if (wxTheApp) {
		wxTheApp->CallAfter(
			[self, percent, status]() { self->m_onProgress(percent, status); });
	}

	return 0;
}

void LspDownloader::Download(const wxString &url, const wxString &destDir) {
	if (!m_curl) {
		if (m_onComplete)
			m_onComplete(false, "Failed to initialize curl");
		return;
	}

	std::thread([this, url, destDir]() {
		wxString tempFile = wxFileName::CreateTempFileName("krafta_lsp");
		FILE *fp = fopen(tempFile.ToStdString().c_str(), "wb");
		if (!fp) {
			if (wxTheApp) {
				wxTheApp->CallAfter([this]() {
					if (m_onComplete)
						m_onComplete(false, "Failed to create temp file");
				});
			}
			return;
		}

		curl_easy_setopt(m_curl, CURLOPT_URL, url.ToStdString().c_str());
		curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(m_curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback_);
		curl_easy_setopt(m_curl, CURLOPT_XFERINFODATA, this);
		curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(m_curl, CURLOPT_MAXREDIRS, 10L);
		curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(m_curl, CURLOPT_USERAGENT, "kraftaEditor/1.0");
		curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 2L);

		CURLcode res = curl_easy_perform(m_curl);
		fclose(fp);

		if (res != CURLE_OK) {
			wxString err = curl_easy_strerror(res);
			wxRemoveFile(tempFile);
			if (wxTheApp) {
				wxTheApp->CallAfter([this, err]() {
					if (m_onComplete)
						m_onComplete(false, "Download failed: " + err);
				});
			}
			return;
		}

		if (wxTheApp) {
			wxTheApp->CallAfter([this]() {
				if (m_onProgress)
					m_onProgress(99, "Extracting...");
			});
		}

		bool ok = Extract(tempFile, destDir);
		wxRemoveFile(tempFile);

		if (wxTheApp) {
			wxTheApp->CallAfter([this, ok]() {
				if (m_onComplete)
					m_onComplete(ok, ok ? "LSP installed successfully"
										: "Extraction failed");
			});
		}
	}).detach();
}

bool LspDownloader::Extract(const wxString &zipPath, const wxString &destDir) {
	wxFFileInputStream fileStream(zipPath);
	if (!fileStream.IsOk())
		return false;

	wxZipInputStream zip(fileStream);
	if (!zip.IsOk())
		return false;

	wxZipEntry *entry;
	while ((entry = zip.GetNextEntry()) != nullptr) {
		std::unique_ptr<wxZipEntry> guard(entry);

		wxString entryPath =
			destDir + wxFileName::GetPathSeparator() + entry->GetName();
		entryPath.Replace("/", wxFileName::GetPathSeparator());

		if (entry->IsDir()) {
			wxFileName::Mkdir(entryPath, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
			continue;
		}

		wxString parentDir = wxFileName(entryPath).GetPath();
		if (!wxDirExists(parentDir))
			wxFileName::Mkdir(parentDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

		wxFileOutputStream fileOut(entryPath);
		if (!fileOut.IsOk())
			return false;

		zip.Read(fileOut);

#ifndef _WIN32
		if (entry->GetExternalAttributes() & 0x00010000)
			chmod(entryPath.ToStdString().c_str(), 0755);
#endif
	}

	return true;
}

wxString LspDownloader::FindPython3() {
	std::vector<wxString> pythonCommands = {"python3", "python"};

	for (const auto &cmd : pythonCommands) {
		wxString command;
#ifdef _WIN32
		command = "where " + cmd + " 2>nul";
		FILE *pipe = _popen(command.ToStdString().c_str(), "r");
#else
		command = "which " + cmd + " 2>/dev/null";
		FILE *pipe = popen(command.ToStdString().c_str(), "r");
#endif
		if (!pipe)
			continue;

		char buffer[256];
		std::string result;
		while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
			result += buffer;
		}

#ifdef _WIN32
		_pclose(pipe);
#else
		pclose(pipe);
#endif

		if (!result.empty()) {
			result.erase(result.find_last_not_of("\n\r") + 1);
			return wxString(result);
		}
	}

	return "";
}

long LspDownloader::RunCommand(const wxString &cmd, wxArrayString &output) {
	std::string command = cmd.ToStdString() + " 2>&1";

#ifdef _WIN32
	FILE *pipe = _popen(command.c_str(), "r");
#else
	FILE *pipe = popen(command.c_str(), "r");
#endif

	if (!pipe)
		return -1;

	char buffer[256];
	while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
		wxString line = wxString::FromUTF8(buffer);
		line.Trim();
		if (!line.empty()) {
			output.push_back(line);
		}
	}

#ifdef _WIN32
	int status = _pclose(pipe);
	return status;
#else
	int status = pclose(pipe);
	return WEXITSTATUS(status);
#endif
}

void LspDownloader::InstallPythonPackage(const wxString &packageName,
										 const wxString &destDir) {
	std::thread([this, packageName, destDir]() {
		if (wxTheApp) {
			wxTheApp->CallAfter([this]() {
				if (m_onProgress)
					m_onProgress(0, "Checking Python 3 installation...");
			});
		}

		wxString python3 = FindPython3();
		if (python3.empty()) {
			if (wxTheApp) {
				wxTheApp->CallAfter([this]() {
					if (m_onComplete)
						m_onComplete(false, "Python 3 not found. Please "
											"install Python 3 first.");
				});
			}
			return;
		}

		if (!wxDirExists(destDir)) {
			wxFileName::Mkdir(destDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
		}

		if (wxTheApp) {
			wxTheApp->CallAfter([this]() {
				if (m_onProgress)
					m_onProgress(10, "Creating Python virtual environment...");
			});
		}

		wxString venvDir = destDir + "/venv";

		if (wxDirExists(venvDir)) {
			wxFileName::Rmdir(venvDir, wxPATH_RMDIR_RECURSIVE);
		}

		wxArrayString output;
		long result = RunCommand(python3 + " -m venv " + venvDir, output);

		if (result != 0) {
			if (wxTheApp) {
				wxTheApp->CallAfter([this]() {
					if (m_onProgress)
						m_onProgress(20, "Installing venv module...");
				});
			}

			result = RunCommand(python3 + " -m pip install virtualenv", output);
			if (result == 0) {
				result =
					RunCommand(python3 + " -m virtualenv " + venvDir, output);
			}
		}

		if (result != 0) {
			if (wxTheApp) {
				wxTheApp->CallAfter([this]() {
					if (m_onComplete)
						m_onComplete(
							false,
							"Failed to create Python virtual environment.");
				});
			}
			return;
		}

		if (wxTheApp) {
			wxTheApp->CallAfter([this, packageName]() {
				if (m_onProgress)
					m_onProgress(40,
								 "Installing " + packageName + " via pip...");
			});
		}

		wxString pipPath;
#ifdef __WXMSW__
		pipPath = venvDir + "/Scripts/pip.exe";
#else
		pipPath = venvDir + "/bin/pip";
#endif

		RunCommand(pipPath + " install --upgrade pip", output);

		if (wxTheApp) {
			wxTheApp->CallAfter([this]() {
				if (m_onProgress)
					m_onProgress(60, "Downloading and installing package...");
			});
		}

		result = RunCommand(pipPath + " install " + packageName, output);

		if (result != 0) {
			if (wxTheApp) {
				wxTheApp->CallAfter([this, packageName]() {
					if (m_onComplete)
						m_onComplete(false, "Failed to install " + packageName +
												" via pip.");
				});
			}
			return;
		}

		if (wxTheApp) {
			wxTheApp->CallAfter([this]() {
				if (m_onProgress)
					m_onProgress(90, "Verifying installation...");
			});
		}

		wxString binaryName = "pylsp";
		wxString binPath;
#ifdef __WXMSW__
		binPath = venvDir + "/Scripts/" + binaryName + ".exe";
#else
		binPath = venvDir + "/bin/" + binaryName;
#endif

		if (wxFileExists(binPath)) {
			if (wxTheApp) {
				wxTheApp->CallAfter([this, binPath]() {
					if (m_onProgress)
						m_onProgress(100, "Installation complete!");
					if (m_onComplete)
						m_onComplete(true,
									 "Installed successfully at: " + binPath);
				});
			}
		} else {
			wxArrayString findOutput;
			RunCommand(pipPath + " show -f " + packageName, findOutput);

			wxString msg = "Package installed but binary 'pylsp' not found.\n";
			msg += "Expected at: " + binPath + "\n";

			if (wxTheApp) {
				wxTheApp->CallAfter([this, msg]() {
					if (m_onComplete)
						m_onComplete(false, msg);
				});
			}
		}
	}).detach();
}

wxString LspDownloader::GetLspDir() {
	wxString baseDir = wxStandardPaths::Get().GetUserConfigDir() +
					   wxFileName::GetPathSeparator() + ".kraftaEditor" +
					   wxFileName::GetPathSeparator() + "lsp" +
					   wxFileName::GetPathSeparator();

	wxFileName fn(baseDir);
	if (!fn.DirExists()) {
		fn.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
	}

	return fn.GetFullPath();
}

void LspDownloader::DownloadClangd() {
	wxString destDir = GetLspDir() + "/clangd";
	wxString path = ApplicationPaths::GetLanguagePreferencesPath("c++") +
					"preferences.json";

	std::ifstream preferencesFile(path.ToStdString());

	if (!preferencesFile) {
		wxMessageBox(_("Unable to find the download link for the C++ LSP."));
		return;
	}

	json languagePreferencesObject = json::parse(preferencesFile);
	std::string clangdUrl;

	try {
		clangdUrl =
			languagePreferencesObject["lsp"]["server"]["download"]["url"]
									 [PlatformInfos::OsNameView()];
	} catch (const std::exception &e) {
		wxMessageBox(_("Unable to find the download link for the C++ LSP."),
					 e.what());
	}

	if (clangdUrl.empty())
		return;

	Download(clangdUrl, destDir);
}

void LspDownloader::InstallPylsp() {
	wxString destDir = GetLspDir() + "/pylsp";
	InstallPythonPackage("python-lsp-server", destDir);
}

wxString LspDownloader::GetPylspPath() {
	wxString venvDir = GetLspDir() + "/pylsp/venv";

	std::vector<wxString> paths = {venvDir + "/bin/pylsp",
								   venvDir + "/Scripts/pylsp.exe",
								   "/usr/bin/pylsp", "/usr/local/bin/pylsp"};

	for (const auto &path : paths) {
		if (wxFileExists(path)) {
			return path;
		}
	}
	return "";
}

bool LspDownloader::IsPylspInstalled() { return !GetPylspPath().empty(); }

wxString LspDownloader::GetClangdPath() {
	wxString path = ApplicationPaths::GetLanguagePreferencesPath("c++") +
					"preferences.json";
	std::ifstream preferencesFile(path.ToStdString());

	if (!preferencesFile) {
		wxMessageBox(_("Could not find the C++ LSP path."));
		return "";
	}

	json languagePreferencesObject = json::parse(preferencesFile);
	std::string clangdPath = GetLspDir().ToStdString();

	try {
		clangdPath =
			clangdPath +
			languagePreferencesObject["lsp"]["server"]["download"]["paths"]
									 [PlatformInfos::OsNameView()]
										 .get<std::string>();
	} catch (const std::exception &e) {
		wxMessageBox(_("Could not find the C++ LSP path."), e.what());
	}

	if (wxFileExists(clangdPath)) {
		return clangdPath;
	} else {
		wxMessageBox(_("Could not find the C++ LSP path."));
		return "";
	}
}

bool LspDownloader::IsClangdInstalled() { return !GetClangdPath().empty(); }