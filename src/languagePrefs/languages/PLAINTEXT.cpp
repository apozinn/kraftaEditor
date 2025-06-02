#pragma once
#include "../prefs.hpp"

const LanguageInfo PLAINTEXT_PREFS = {
	"txt",
	"txt",
	"*.txt;",
	wxSTC_LEX_AUTOMATIC,
	{{mySTC_TYPE_DEFAULT, nullptr}},
	mySTC_FOLD_COMMENT | mySTC_FOLD_COMPACT | mySTC_FOLD_PREPROC,
	
};