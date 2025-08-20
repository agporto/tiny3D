// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#pragma once

#include <string>

namespace tiny3d {
namespace io {

enum FileGeometry {
    CONTENTS_UNKNOWN = 0,
    CONTAINS_POINTS = (1 << 0),
    CONTAINS_LINES = (1 << 1),
    CONTAINS_TRIANGLES = (1 << 2),
};

/// Returns the kind of geometry that the file contains. This is a quick
/// function designed to query the file in order to determine whether to
/// call ReadTriangleMesh(), ReadLineSet(), or ReadPointCloud()
FileGeometry ReadFileGeometryType(const std::string& path);

FileGeometry ReadFileGeometryTypePLY(const std::string& path);
FileGeometry ReadFileGeometryTypeXYZ(const std::string& path);


}  // namespace io
}  // namespace tiny3d
