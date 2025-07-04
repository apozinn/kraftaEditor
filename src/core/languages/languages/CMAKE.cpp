#pragma once
#include "../prefs.hpp"

namespace {
	constexpr const char *words1 = "add_custom_command add_compile_definitions add_compile_options add_custom_command add_custom_target add_definitions add_dependencies add_executable add_library add_link_options add_subdirectory add_test aux_source_directory build_command build_name cmake_minimum_required configure_file create_test_sourcelist else elseif enable_language enable_testing endforeach endfunction endif endmacro endwhile exec_program execute_process export export_library_dependencies file find_file find_library find_package find_path find_program fltk_wrap_ui foreach function get_cmake_property get_directory_property get_filename_component get_source_file_property get_target_property get_test_property if include include_directories include_external_msproject include_regular_expression install install_files install_programs install_targets link_directories link_libraries list load_cache load_command macro make_directory mark_as_advanced math message option output_required_files project qt_wrap_cpp qt_wrap_ui remove remove_definitions separate_arguments set set_directory_properties set_source_files_properties set_target_properties set_tests_properties site_name source_group string subdir_depends subdirs target_compile_definitions target_compile_features target_compile_options target_include_directories target_link_directories target_link_libraries target_link_options target_sources try_compile try_run use_mangled_mesa utility_source variable_requires vtk_make_instantiator vtk_wrap_java vtk_wrap_python vtk_wrap_tcl while write_file";
	
	constexpr const char *words2 = "ABSOLUTE ABSTRACT ADDITIONAL_MAKE_CLEAN_FILESALL AND APPEND ARGS ASCII BEFORE CACHE CACHE_VARIABLES CLEAR COMMAND COMMANDS COMMAND_NAME COMMENT COMPARE COMPILE_FLAGS COPYONLY DEFINED DEFINE_SYMBOL DEPENDS DOC EQUAL ESCAPE_QUOTES EXCLUDE EXCLUDE_FROM_ALL EXISTS EXPORT_MACRO EXT EXTRA_INCLUDE FATAL_ERROR FILE FILES FORCE FUNCTION GENERATED GLOB GLOB_RECURSE GREATER GROUP_SIZE HEADER_FILE_ONLY HEADER_LOCATION IMMEDIATE INCLUDES INCLUDE_DIRECTORIES INCLUDE_INTERNALS INCLUDE_REGULAR_EXPRESSION LESS LINK_DIRECTORIES LINK_FLAGS LOCATION MACOSX_BUNDLE MACROS MAIN_DEPENDENCY MAKE_DIRECTORY MATCH MATCHALL MATCHES MODULE NAME NAME_WE NOT NOTEQUAL NO_SYSTEM_PATH OBJECT_DEPENDS OPTIONAL OR OUTPUT OUTPUT_VARIABLE PATH PATHS POST_BUILD POST_INSTALL_SCRIPT PREFIX PREORDER PRE_BUILD PRE_INSTALL_SCRIPT PRE_LINK PROGRAM PROGRAM_ARGS PROPERTIES QUIET RANGE READ REGEX REGULAR_EXPRESSION REPLACE REQUIRED RETURN_VALUE RUNTIME_DIRECTORY SEND_ERROR SHARED SOURCES STATIC STATUS STREQUAL STRGREATER STRLESS SUFFIX TARGET TOLOWER TOUPPER VAR VARIABLES VERSION WIN32 WRAP_EXCLUDE WRITE APPLE MINGW MSYS CYGWIN BORLAND WATCOM MSVC MSVC_IDE MSVC60 MSVC70 MSVC71 MSVC80 CMAKE_COMPILER_2005 OFF ON";
}

constexpr const LanguageInfo CMAKE_PREFS = {
	"Cmake",
	"cmake",
	"*.cmake;",
	wxSTC_LEX_CMAKE,
	{{mySTC_TYPE_DEFAULT, nullptr},
	 {mySTC_TYPE_COMMENT, nullptr},
	 {mySTC_TYPE_COMMENT_LINE, nullptr},
	 {mySTC_TYPE_COMMENT_DOC, nullptr},
	 {mySTC_TYPE_NUMBER, nullptr},
	 {mySTC_TYPE_WORD1, words1}, // KEYWORDS
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
	 {mySTC_TYPE_WORD2, words2},			// EXTRA WORDS
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
	
};