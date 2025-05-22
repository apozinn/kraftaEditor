#include <fstream>
#include <wx/stdpaths.h>

class UserConfig
{
	std::string configPath;

public:
	json Get();
	bool Update(json new_data);
	json GetThemes();
};