#pragma once
#include "../prefs.hpp"

constexpr const LanguageInfo htmlPrefs = {
	"Html",
	"html",
	".html",
	wxSTC_LEX_HTML,
	{{mySTC_TYPE_DEFAULT, nullptr}},
	mySTC_FOLD_COMMENT | mySTC_FOLD_COMPACT | mySTC_FOLD_PREPROC
};