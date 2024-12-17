#pragma once

#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

class FileManager {
public:
	void ListAllFiles(const std::string& path, std::function<void(const std::string&, const std::string&, const std::string&)> cb) {

	}

	void ListChildrens(const std::string& path, std::function<void(const std::string&, const std::string&, const std::string&)> cb) {

	}

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

	std::string CropPath(std::string path, std::string directoryTarget) {
		/*std::string pathTobeReturned;
		while (!pathTobeReturned.empty()) {
			path = path.find_first_of()
		}*/
	}
};