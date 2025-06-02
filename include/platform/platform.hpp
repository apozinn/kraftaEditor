#ifndef PLATFORM_UTILS_H
#define PLATFORM_UTILS_H

#include <wx/platinfo.h>
#include <wx/filename.h>
#include <string>
#include <string_view>

namespace Platform {

inline std::string OsName() noexcept {
    static const auto osName = wxPlatformInfo::Get().GetOperatingSystemFamilyName().ToStdString();
    return osName;
}

inline std::string OsPathSepareator() noexcept {
    static const wxFileName file;
    static const std::string separator(1, file.GetPathSeparator());
    return separator;
}

} 
#endif 