#include "userConfig.hpp"

json UserConfig::Get()
{
	wxString appDataDir = wxStandardPaths::Get().GetUserConfigDir() + "/.kraftaEditor";
	config_path = appDataDir.ToStdString() + "/userconfig.json";
	wxFileName appDataDir_op(appDataDir);

	if (!appDataDir_op.Exists())
	{
		bool dir_created = wxFileName::Mkdir(appDataDir);
		if (dir_created)
		{
			wxFile newConfigFile;
			bool created = newConfigFile.Create(appDataDir + "/userconfig.json");
			if (created)
			{
				std::ofstream newConfigFile_locale(config_path);
				json new_json_obj = {
					{"show_minimap", true},
					{"show_menu", true},
				};
				newConfigFile_locale << std::setw(4) << new_json_obj << std::endl;
			}
		}
	}

	json data;

	std::ifstream config_file(config_path);
	if (config_file)
	{
		data = json::parse(config_file);
	}

	return data;
}

bool UserConfig::Update(json new_data)
{
	std::ofstream config_file(config_path);
	if (config_file)
	{
		config_file << std::setw(4) << new_data << std::endl;
		return true;
	}
	else return false;
}

json UserConfig::GetThemes()
{
	std::string as = wxStandardPaths::Get().GetExecutablePath().ToStdString();
	as = as.substr(0, as.find("krafta-editor") + 13) + "/src/themes.json";

	json data;
	try
	{
		std::ifstream themes(as);
		if (themes)
		{
			data = json::parse(themes);
		}
	}
	catch (const json::exception& e)
	{
		std::cout << "message: " << e.what() << '\n'
			<< "exception id: " << e.id << std::endl;
	}

	auto systemInfo = wxSystemSettings::GetAppearance();
	if (systemInfo.IsSystemDark()) {
		return data["dark"];
	}
	else return data["light"];

}