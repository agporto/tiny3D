// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "tiny3d/io/ModelIO.h"

#include <memory>

#include "tiny3d/utility/FileSystem.h"
#include "tiny3d/utility/Logging.h"
#include "tiny3d/utility/ProgressBar.h"
#include "tiny3d/geometry/TriangleMesh.h"
#include "tiny3d/io/TriangleMeshIO.h"

namespace tiny3d {
namespace io {

bool ReadTriangleModel(const std::string& filename,
                       geometry::TriangleMesh& mesh,
                       ReadTriangleModelOptions params /*={}*/) {
    if (params.print_progress) {
        auto progress_text = std::string("Reading model file ") + filename;
        auto pbar = utility::ProgressBar(100, progress_text, true);
        params.update_progress = [pbar](double percent) mutable -> bool {
            pbar.SetCurrentCount(size_t(percent));
            return true;
        };
    }

    std::string extension = utility::filesystem::GetFileExtensionInLowerCase(filename);
    if (extension != "ply") {
        utility::LogWarning("Only .ply models are supported. Got extension: {}", extension);
        return false;
    }

    // 🔵 Create a ReadTriangleMeshOptions from the ReadTriangleModelOptions
    ReadTriangleMeshOptions mesh_options;
    mesh_options.update_progress = params.update_progress;
    // (If you had more fields that mattered, you would map them here.)

    if (!ReadTriangleMeshFromPLY(filename, mesh, mesh_options)) {
        utility::LogWarning("Failed to read PLY model: {}", filename);
        return false;
    }

    return true;
}

}  // namespace io
}  // namespace tiny3d

