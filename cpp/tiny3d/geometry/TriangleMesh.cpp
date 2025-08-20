// ----------------------------------------------------------------------------
// -                      Tiny3D: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------
//
// Minimal version for basic functionality. Implements methods declared
// in the basic TriangleMesh.h. Removed implementations for non-basic
// features (adjacency, UVs, materials, merge, advanced methods).
// Fixed Clear() method.
//

#include "tiny3d/geometry/TriangleMesh.h"

#include <Eigen/Dense>
#include <vector>
#include <numeric> // For std::fill if needed elsewhere, not strictly needed here now

// #include "tiny3d/geometry/KDTreeFlann.h" // Not needed for basic version
#include "tiny3d/utility/Logging.h"
#include "tiny3d/utility/Parallel.h" // Keep if normal computation uses it
// #include "tiny3d/utility/Random.h" // Not needed for basic version

namespace tiny3d {
namespace geometry {

TriangleMesh &TriangleMesh::Clear() {
    // Clear members inherited from MeshBase
    MeshBase::Clear();
    // Clear members specific to TriangleMesh (basic version)
    triangles_.clear();
    triangle_normals_.clear();

    // Removed clearing of non-basic/undeclared members:
    // adjacency_list_.clear();
    // triangle_uvs_.clear();
    // materials_.clear();
    // triangle_material_ids_.clear();
    // textures_.clear();

    return *this;
}

TriangleMesh &TriangleMesh::Transform(const Eigen::Matrix4d &transformation) {
    // Transform members inherited from MeshBase (vertices, vertex normals/colors)
    MeshBase::Transform(transformation);
    // Transform triangle normals (if they exist)
     if (HasTriangleNormals()) {
         MeshBase::TransformNormals(transformation, triangle_normals_);
     }
    return *this;
}

TriangleMesh &TriangleMesh::Rotate(const Eigen::Matrix3d &R,
                                   const Eigen::Vector3d &center) {
    // Rotate members inherited from MeshBase
    MeshBase::Rotate(R, center);
    // Rotate triangle normals (if they exist)
     if (HasTriangleNormals()) {
         MeshBase::RotateNormals(R, triangle_normals_);
     }
    return *this;
}

TriangleMesh &TriangleMesh::ComputeTriangleNormals(bool normalized /* = true*/) {
    if (!HasVertices() || !HasTriangles()) {
        utility::LogWarning(
                "Cannot compute triangle normals. Mesh has no vertices or "
                "triangles.");
        return *this;
    }

    triangle_normals_.resize(triangles_.size());

    // Potential parallelization opportunity
    // #pragma omp parallel for schedule(static) 
    // num_threads(utility::EstimateMaxThreads())
    for (size_t i = 0; i < triangles_.size(); i++) {
        const auto &triangle = triangles_[i];
        // Check for valid indices
        if (triangle(0) < 0 || static_cast<size_t>(triangle(0)) >= vertices_.size() ||
            triangle(1) < 0 || static_cast<size_t>(triangle(1)) >= vertices_.size() ||
            triangle(2) < 0 || static_cast<size_t>(triangle(2)) >= vertices_.size()) {
             utility::LogWarning("Triangle {} has invalid vertex indices.", i);
             triangle_normals_[i].setZero(); // Assign zero normal for invalid triangle
             continue;
        }

        const Eigen::Vector3d &v0 = vertices_[triangle(0)];
        const Eigen::Vector3d &v1 = vertices_[triangle(1)];
        const Eigen::Vector3d &v2 = vertices_[triangle(2)];
        Eigen::Vector3d v01 = v1 - v0;
        Eigen::Vector3d v02 = v2 - v0;
        triangle_normals_[i] = v01.cross(v02);
    }

    if (normalized) {
        NormalizeNormals(); // This will normalize both vertex and triangle normals
    }
    return *this;
}

TriangleMesh &TriangleMesh::ComputeVertexNormals(bool normalized /* = true*/) {
    if (!HasVertices() || !HasTriangles()) {
        utility::LogWarning(
                "Cannot compute vertex normals. Mesh has no vertices or "
                "triangles.");
        return *this;
    }

    // Ensure triangle normals are computed (but not necessarily normalized yet)
    if (!HasTriangleNormals()) {
        ComputeTriangleNormals(false); // Compute non-normalized triangle normals first
         // Check again if computation failed
         if (!HasTriangleNormals()) {
              utility::LogError("Failed to compute triangle normals, cannot compute vertex normals.");
              return *this;
         }
    }

    vertex_normals_.resize(vertices_.size());
    std::fill(vertex_normals_.begin(), vertex_normals_.end(),
              Eigen::Vector3d::Zero());

    for (size_t i = 0; i < triangles_.size(); i++) {
        const auto &triangle = triangles_[i];
         // Check for valid indices before accessing vertex_normals_
        if (triangle(0) >= 0 && static_cast<size_t>(triangle(0)) < vertices_.size() &&
            triangle(1) >= 0 && static_cast<size_t>(triangle(1)) < vertices_.size() &&
            triangle(2) >= 0 && static_cast<size_t>(triangle(2)) < vertices_.size()) {
            // Accumulate triangle normal onto its vertices
            vertex_normals_[triangle(0)] += triangle_normals_[i];
            vertex_normals_[triangle(1)] += triangle_normals_[i];
            vertex_normals_[triangle(2)] += triangle_normals_[i];
         } else {
              // Warning already logged in ComputeTriangleNormals if indices were bad
              // Or add specific warning here if needed
         }
    }

    if (normalized) {
        NormalizeNormals(); // This will normalize both vertex and triangle normals
    }
    return *this;
}

// --- Implementations for Non-Basic Methods Removed ---

} // namespace geometry
} // namespace tiny3d
