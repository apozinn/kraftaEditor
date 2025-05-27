#pragma once

#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

class FileManager {
public:
	bool CreateFile(wxString path) {
		return wxFile().Create(path);
	}

	bool CreateDir(wxString path) {
		return wxFileName::Mkdir(path);
	}

	bool DeleteFile(wxString path) {
		return wxRemoveFile(path);
	}

	bool DeleteDir(wxString path) {
		return wxFileName::Rmdir(path, wxPATH_RMDIR_RECURSIVE);
	}
};