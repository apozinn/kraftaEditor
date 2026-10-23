#include "app/frames/mainFrame.hpp"
#include "controlPanel.hpp"
#include "gui/panels/selectionPanel/selectionPanel.hpp"
#include "languagesPreferences/languagesPreferences.hpp"
#include "themesManager/themeApplier.hpp"
#include <wx/arrstr.h>
#include <wx/dir.h>

#if __WXMSW__
#include <wx/msw/darkmode.h>
#include <wx/msw/private.h>
#endif

void ControlPanel::OpenThemeSelectionPanel() {
	if (!m_frameParent) {
		wxMessageBox(_("A critical GUI component could not be found."));
		return;
	}

	if (auto *mainFrame = dynamic_cast<MainFrame *>(wxTheApp->GetTopWindow())) {
		auto themeSelection = new SelectionPanel(mainFrame);
		if (themeSelection) {

			wxArrayString files;
			size_t count =
				wxDir::GetAllFiles(ApplicationPaths::AssetsPath("themes"),
								   &files, "*.json", wxDIR_FILES);

			std::vector<OptionsListStruct> m_list;

			for (const auto &file_w : files) {
				try {
					std::ifstream file(file_w.ToStdString());
					nlohmann::json themeData = nlohmann::json::parse(file);

					m_list.push_back({themeData["themeName"].get<std::string>(),
									  "", static_cast<int>(m_list.size())});

				} catch (const std::exception &e) {
					wxLogError(wxString::Format(_("Failed to get themes: %s"),
												e.what()));
					throw std::runtime_error("Theme file not found");
				}
			}

			themeSelection->ShowList(
				m_list, [this, files, mainFrame](const OptionsListStruct &opt) {
					std::ifstream file(files.Item(opt.id).ToStdString());
					nlohmann::json themeData = nlohmann::json::parse(file);
					ThemesManager::Get().currentTheme = themeData;
					UserSettingsManager::Get().SetSetting<std::string>("theme/themeFilePath",
														  files.Item(opt.id).ToStdString());
					LanguagesPreferences::Get().Reload();

					ThemeApplier::ApplyTheme(mainFrame, themeData);

					try {
						if (ThemesManager::Get().IsDarkTheme()) {
							wxTheApp->SetAppearance(wxApp::Appearance::Dark);
#ifdef __WXMSW__
    wxTheApp->MSWEnableDarkMode(wxApp::DarkMode_Always);
#endif
						} else {
							wxTheApp->SetAppearance(wxApp::Appearance::Light);
						}
					} catch (const std::exception &e) {
						wxMessageBox(e.what());
						std::cerr << "Standard Exception: " << e.what()
								  << std::endl;
					}
				});
		}
	}
}