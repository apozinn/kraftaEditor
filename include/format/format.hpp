#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

/**
 * @namespace Format
 * @brief Contains utility functions for data formatting
 */
namespace Format {
    /**
     * @brief Converts a byte count into a human-readable string with appropriate unit
     * @param bytes The number of bytes to format (must be non-negative)
     * @return A formatted string with the most appropriate unit (B, KB, MB, GB, TB, PB)
     * 
     * @details
     * This function takes a byte count and converts it to the largest possible binary unit
     * (using 1024-based divisions) while keeping the value above 1. The result is formatted
     * with 2 decimal places, with trailing ".00" removed for cleaner output.
     * 
     * Negative values are automatically converted to "0 B".
     * 
     * The supported units are:
     * - B (Bytes)
     * - KB (Kibibytes)
     * - MB (Mebibytes)
     * - GB (Gibibytes)
     * - TB (Tebibytes)
     * - PB (Pebibytes)
     * 
     * @code
     * FormatBytes(500)      // Returns "500 B"
     * FormatBytes(2048)     // Returns "2 KB"
     * FormatBytes(1500000)  // Returns "1.43 MB"
     * FormatBytes(-100)     // Returns "0 B"
     * @endcode
     */
    inline std::string FormatBytes(long long bytes)
    {
        static constexpr const char *sizes[] = {"B", "KB", "MB", "GB", "TB", "PB"};
        constexpr int max_order = sizeof(sizes) / sizeof(sizes[0]) - 1;
    
        if (bytes < 0)
        {
            return "0 B";
        }
    
        int order = 0;
        double dblBytes = static_cast<double>(bytes);
    
        while (dblBytes >= 1024 && order < max_order)
        {
            dblBytes /= 1024;
            ++order;
        }
    
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << dblBytes << " " << sizes[order];
    
        std::string result = oss.str();
        size_t dot_pos = result.find('.');
        if (dot_pos != std::string::npos && result.substr(dot_pos) == ".00")
        {
            result.erase(dot_pos, 3);
        }
    
        return result;
    }
}