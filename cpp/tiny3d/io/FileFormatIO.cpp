// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "FileFormatIO.h"

#include <map>

#include "tiny3d/utility/FileSystem.h"

namespace tiny3d {
namespace io {

static std::map<std::string, FileGeometry (*)(const std::string&)> gExt2Func = {
        {"ply", ReadFileGeometryTypePLY},
        {"xyz", ReadFileGeometryTypeXYZ},
};

FileGeometry ReadFileGeometryType(const std::string& path) {
    auto ext = utility::filesystem::GetFileExtensionInLowerCase(path);
    auto it = gExt2Func.find(ext);
    if (it != gExt2Func.end()) {
        return it->second(path);
    } else {
        return CONTENTS_UNKNOWN;
    }
}

}  // namespace io
}  // namespace tiny3d
