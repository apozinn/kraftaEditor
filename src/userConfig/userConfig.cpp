#include "userConfig.hpp"

json UserConfig::Get()
{
	json standardUserConfig = {
		{"show_minimap", true},
		{"show_menu", true},
		{"show_statusBar", true},
		{"windowMaximized", true},
		{"windowSizeX", 1000},
		{"windowSizeY", 700}};

	configPath = wxString(applicationPath + "userconfig.json").ToStdString();
	if (!wxFileExists(configPath))
	{
		wxFile newConfigFile;
		bool created = newConfigFile.Create(configPath);
		if (created)
		{
			std::ofstream newConfigFile_locale(configPath);
			
			newConfigFile_locale << std::setw(4) << standardUserConfig << std::endl;
		}
		else
		{
			wxMessageBox("an error occurred while creating the configuration file");
			mainFrame->Close(true);
		}
	}

	json data;
	std::ifstream config_file(configPath);
	if (config_file)
	{
		//create the element if it does not exist in the userConfig file
		data = json::parse(config_file);
		  for(auto& element : standardUserConfig.items()) {
			if(!data.contains(element.key())) {
				data[element.key()] = element.value();
				Update(data);
			}
		  }
	}

	return data;
}

bool UserConfig::Update(json new_data)
{
	std::ofstream config_file(configPath);
	if (config_file)
	{
		config_file << std::setw(4) << new_data << std::endl;
		return true;
	}
	else
		return false;
}

json UserConfig::GetThemes()
{
	std::string location = wxStandardPaths::Get().GetExecutablePath().ToStdString();
	int appLocation = location.find("kraftaEditor");
	location = location.substr(0, appLocation + 13) + "src/themes.json";

	json data;
	try
	{
		std::ifstream themes(location);
		if (themes)
		{
			data = json::parse(themes);
		}
	}
	catch (const json::exception &e)
	{
		wxMessageBox(wxString("an error occurred while extracting themes") + wxString(e.what()));
	}

	auto systemInfo = wxSystemSettings::GetAppearance();
	if (systemInfo.IsSystemDark())
	{
		return data["dark"];
	}
	else
		return data["light"];
}