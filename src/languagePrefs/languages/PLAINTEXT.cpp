#pragma once
#include "../prefs.hpp"

const LanguageInfo PLAINTEXT_PREFS = {
	"Text",
	"*.txt;",
	wxSTC_LEX_AUTOMATIC,
	{{mySTC_TYPE_DEFAULT, nullptr}},
	mySTC_FOLD_COMMENT | mySTC_FOLD_COMPACT | mySTC_FOLD_PREPROC,
	icons_dir + "file_ext" + osSlash + "txt.png"};