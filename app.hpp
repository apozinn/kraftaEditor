#pragma once

wxString stringToWxString(std::string str)
{
	wxString r_str = str;
	return r_str;
}

wxFont fontWithOtherSize(wxWindow *cmp, int size)
{
	wxFont font = cmp->GetFont();
	font.SetPixelSize(wxSize(size, size));
	return font;
}

const char *FormatBytes(long long bytes, char *str)
{
	const char *sizes[5] = {"B", "KB", "MB", "GB", "TB"};
	int i;
	double dblByte = bytes;
	for (i = 0; i < 5 && bytes >= 1024; i++, bytes /= 1024)
		dblByte = bytes / 1024.0;

	sprintf(str, "%.2f", dblByte);
	return strcat(strcat(str, " "), sizes[i]);
}

static wxString GetFSWEventChangeTypeName(int changeType)
{
	switch (changeType)
	{
	case wxFSW_EVENT_CREATE:
		return "CREATE";
	case wxFSW_EVENT_DELETE:
		return "DELETE";
	case wxFSW_EVENT_RENAME:
		return "RENAME";
	case wxFSW_EVENT_MODIFY:
		return "MODIFY";
	case wxFSW_EVENT_ACCESS:
		return "ACCESS";
	case wxFSW_EVENT_ATTRIB:
		return "ATTRIBUTE";
#ifdef wxHAS_INOTIFY
	case wxFSW_EVENT_UNMOUNT:
		return "UNMOUNT";
#endif
	case wxFSW_EVENT_WARNING:
		return "WARNING";
	case wxFSW_EVENT_ERROR:
		return "ERROR";
	}
	return "INVALID_TYPE";
}

wxString GetAppDirs(std::string targetDir)
{
	std::string executablePath = wxStandardPaths::Get().GetExecutablePath().ToStdString();
	executablePath = executablePath.substr(0, executablePath.find("kraftaEditor") + 13) + osSlash + targetDir + osSlash;
	return wxString(executablePath);
}