#pragma once
#include "../prefs.hpp"

const LanguageInfo UNKNOWN_PREFS = {
	"Unknown",
	"",
	wxSTC_LEX_AUTOMATIC,
	{{mySTC_TYPE_DEFAULT, nullptr},
	 {mySTC_TYPE_COMMENT, nullptr},
	 {mySTC_TYPE_COMMENT_LINE, nullptr},
	 {mySTC_TYPE_COMMENT_DOC, nullptr},
	 {mySTC_TYPE_NUMBER, nullptr},
	 {mySTC_TYPE_WORD1, nullptr}, // KEYWORDS
	 {mySTC_TYPE_STRING, nullptr},
	 {mySTC_TYPE_CHARACTER, nullptr},
	 {mySTC_TYPE_UUID, nullptr},
	 {mySTC_TYPE_PREPROCESSOR, nullptr},
	 {mySTC_TYPE_OPERATOR, nullptr},
	 {mySTC_TYPE_IDENTIFIER, nullptr},
	 {mySTC_TYPE_STRING_EOL, nullptr},
	 {mySTC_TYPE_DEFAULT, nullptr}, // VERBATIM
	 {mySTC_TYPE_REGEX, nullptr},
	 {mySTC_TYPE_COMMENT_SPECIAL, nullptr}, // DOXY
	 {mySTC_TYPE_WORD2, nullptr},			// EXTRA WORDS
	 {mySTC_TYPE_WORD3, nullptr},			// DOXY KEYWORDS
	 {mySTC_TYPE_ERROR, nullptr},			// KEYWORDS ERROR
	 {-1, nullptr},
	 {-1, nullptr},
	 {-1, nullptr},
	 {-1, nullptr},
	 {-1, nullptr},
	 {-1, nullptr},
	 {-1, nullptr},
	 {-1, nullptr},
	 {-1, nullptr},
	 {-1, nullptr},
	 {-1, nullptr},
	 {-1, nullptr},
	 {-1, nullptr}},
	mySTC_FOLD_COMMENT | mySTC_FOLD_COMPACT | mySTC_FOLD_PREPROC,
	icons_dir + "file_ext" + osSlash + "unknown.png"};