#pragma once
#include "../prefs.hpp"

const LanguageInfo htmlPrefs = {
	"Html",
	".html",
	wxSTC_LEX_HTML,
	{{mySTC_TYPE_DEFAULT, nullptr}},
	mySTC_FOLD_COMMENT | mySTC_FOLD_COMPACT | mySTC_FOLD_PREPROC,
	icons_dir + "file_ext" + osSlash + "unknown.png"};