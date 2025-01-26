#pragma once

wxString stringToWxString(std::string str) {
	wxString r_str = str;
	return r_str;
}

wxFont fontWithOtherSize(wxWindow* cmp, int size) {
	wxFont font = cmp->GetFont();
	font.SetPixelSize(wxSize(size, size));
	return font;
}

const char* FormatBytes(long long bytes, char* str) {
	const char* sizes[5] = { "B", "KB", "MB", "GB", "TB" };
	int i;
	double dblByte = bytes;
	for (i = 0; i < 5 && bytes >= 1024; i++, bytes /= 1024)
		dblByte = bytes / 1024.0;

	sprintf(str, "%.2f", dblByte);
	return strcat(strcat(str, " "), sizes[i]);
}

struct NotifyInteractions {
	const char* label;
	const char* id;
};

static wxString GetFSWEventChangeTypeName(int changeType) {
	switch (changeType) {
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

wxString GetAppDirs(std::string targetDir) {
	std::string executablePath = wxStandardPaths::Get().GetExecutablePath().ToStdString();
	if (__WXWINDOWS__) {
		executablePath = executablePath.substr(0, executablePath.find("krafta-editor") + 13) + "\\" + targetDir + "\\";
	}
	else {
		executablePath = executablePath.substr(0, executablePath.find("krafta-editor") + 13) + "/" + targetDir + "/";
	}
	return wxString(executablePath);
}

void DrawBorder(wxWindow* target, wxClientDC& dc, int sides) {
	//function to draw a border to element
	dc.SetPen(wxPen(wxColor(UserTheme["border"].template get<std::string>()), 0.20));

	switch (sides) {
	case BORDER_SIDE_TOP: {
		dc.DrawLine(0, 0, target->GetSize().GetWidth(), 0);
	}
	case BORDER_SIDE_BOTTOM: {
		dc.DrawLine(0, target->GetSize().GetHeight() - 1, target->GetSize().GetWidth(), target->GetSize().GetHeight() - 1);
	}
	case BORDER_SIDE_LEFT: {
		dc.DrawLine(0, 200, 0, 0);
	}
	case BORDER_SIDE_RIGHT: {
		dc.DrawLine(0, 0, 200, 0);
	}
	}
}