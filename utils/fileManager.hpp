#pragma once

#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

class FileManager {
public:
	bool CreateFile(wxString path) {
		wxFile fname;
		bool created = fname.Create(path);
		return created;
	}

	bool CreateDir(wxString path) {
		bool created = wxFileName::Mkdir(path);
		return created;
	}

	bool DeleteFile(wxString path) {
		int deleted = remove(path);
		if (!deleted) return false;
		return true;
	}

	bool DeleteDir(wxString path) {
		bool deleted = wxFileName::Rmdir(path);
		return deleted;
	}
};