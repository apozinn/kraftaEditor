#pragma once
#include <wx/string.h>

/**
 * @struct NotifyInteractions
 * @brief Represents a notification interaction element with label and identifier
 * 
 * This structure is used to define interactive elements in notifications,
 * containing both the display label and a unique identifier for handling events.
 */
struct NotifyInteractions {
    const char* label;   ///< Display text for the interaction element
    const char* id;      ///< Unique identifier for the interaction element
};

/**
 * @namespace StyleConstants
 * @brief Contains style-related constants for the application
 */
namespace StyleConstants {
    constexpr int TYPES_COUNT = 32;  ///< Maximum number of supported style types
}

/**
 * @struct AppPathWithErrorMessage
 * @brief Bundles a filesystem path with its associated error message
 * 
 * Used when validating paths to provide both the path and any validation error.
 * This is particularly useful for error reporting in file operations.
 */
struct AppPathWithErrorMessage {
    const wxString path;          ///< The filesystem path being referenced
    const wxString errorMessage;  ///< Description of any error related to this path
};