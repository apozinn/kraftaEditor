#pragma once
#include "../prefs.hpp"

const char *reservedWords1 =
	"abstract arguments await boolean "
	"char class const continue break "
	"debugger default delete do byte "
	"double else enum eval case "
	"export extends false catch "
	"try typeof var void while with "
	"this throw throws transient true ";

const char *reservedWords2 =
	"finally float for function "
	"in instanceof int interface "
	"let long native new "
	"null package private protected "
	"public	return short final "
	"static super switch synchronized "
	"goto if implements	import from";

const LanguageInfo JAVASCRIPT_PREFS = {
	"Javascript",
	"*.js;*.jsx",
	wxSTC_LEX_CPP,
	{{mySTC_TYPE_DEFAULT, nullptr},
	{mySTC_TYPE_COMMENT, nullptr},
	{mySTC_TYPE_COMMENT_LINE, nullptr},
	{mySTC_TYPE_COMMENT_DOC, nullptr},
	{mySTC_TYPE_NUMBER, nullptr},
	{mySTC_TYPE_WORD1, reservedWords1}, // KEYWORDS
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
	{mySTC_TYPE_WORD2, reservedWords2}, // EXTRA WORDS
	{mySTC_TYPE_WORD3, nullptr}, // DOXY KEYWORDS
	{mySTC_TYPE_ERROR, nullptr}, // KEYWORDS ERROR
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
	icons_dir + "file_ext" + osSlash + "javascript.png"};