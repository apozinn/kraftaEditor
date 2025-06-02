#pragma once
#include "../prefs.hpp"

const char *CppWordlist1 =
	"asm auto bool break case catch char class const const_cast "
	"continue default delete do double dynamic_cast else enum explicit "
	"export extern false float for friend goto if inline int long "
	"mutable namespace new operator private protected public register "
	"reinterpret_cast return short signed sizeof static static_cast "
	"struct switch template throw true try typedef typeid "
	"typename union unsigned using virtual void volatile wchar_t "
	"while";
const char *CppWordlist2 =
	"file this";
const char *CppWordlist3 =
	"a addindex addtogroup anchor arg attention author b brief bug c "
	"class code date def defgroup deprecated dontinclude e em endcode "
	"endhtmlonly endif endlatexonly endlink endverbatim enum example "
	"exception f$ f[ f] file fn hideinitializer htmlinclude "
	"htmlonly if image ingroup internal invariant interface "
	"latexonly li line link mainpage name namespace nosubgrouping note "
	"overload p page par param post pre ref relates remarks return "
	"retval sa section see showinitializer since skip skipline struct "
	"subsection test throw todo typedef union until var verbatim "
	"verbinclude version warning weakgroup $ @ \"\" & < > # { }";

const LanguageInfo CPP_PREFS = {
	"C++",
	"cpp",
	"*.c;*.cc;*.cpp;*.cxx;*.cs;*.h;*.hh;*.hpp;*.hxx;*.sma",
	wxSTC_LEX_CPP,
	{{mySTC_TYPE_DEFAULT, nullptr},
	 {mySTC_TYPE_COMMENT, nullptr},
	 {mySTC_TYPE_COMMENT_LINE, nullptr},
	 {mySTC_TYPE_COMMENT_DOC, nullptr},
	 {mySTC_TYPE_NUMBER, nullptr},
	 {mySTC_TYPE_WORD1, CppWordlist1}, // KEYWORDS
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
	 {mySTC_TYPE_WORD2, CppWordlist2},		// EXTRA WORDS
	 {mySTC_TYPE_WORD3, CppWordlist3},		// DOXY KEYWORDS
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
	
};