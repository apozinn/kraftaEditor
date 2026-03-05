#include <string>
#include <sstream>
#include <iomanip>
#include <functional>

/**
 * @brief Converts a project path into a unique hex hash string
 * @param path The absolute path to the workspace folder
 * @return A unique string identifier for the workspace
 */
inline std::string ConvertPathToHash(const std::string& path) {
    // Generate a size_t hash based on the string content
    size_t hashValue = std::hash<std::string>{}(path);

    // Convert the numeric hash to a hex string for file system compatibility
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hashValue;
    
    return ss.str();
}