// ----------------------------------------------------------------------------
// -                      Tiny3D: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------
//
// Minimal version for basic functionality. Removed adjacency list, UVs,
// materials, textures, merge operators, and advanced methods.
//

#pragma once

#include <Eigen/Core>
#include <memory>
#include <vector>
#include <cmath> // For std::isnan

#include "tiny3d/geometry/MeshBase.h"
// #include "tiny3d/utility/Helper.h" // May not be needed

namespace tiny3d {
namespace geometry {

// Forward declare if needed by MeshBase or other includes
// class PointCloud;

/// \class TriangleMesh
///
/// \brief Triangle mesh contains vertices and triangles represented by the
/// indices to the vertices. Basic version includes vertex/triangle normals
/// and vertex colors inherited from MeshBase.
class TriangleMesh : public MeshBase {
public:
    /// \brief Default Constructor.
    TriangleMesh() : MeshBase(Geometry::GeometryType::TriangleMesh) {}
    /// \brief Parameterized Constructor.
    ///
    /// \param vertices list of vertices.
    /// \param triangles list of triangles.
    TriangleMesh(const std::vector<Eigen::Vector3d> &vertices,
                 const std::vector<Eigen::Vector3i> &triangles)
        : MeshBase(Geometry::GeometryType::TriangleMesh, vertices),
          triangles_(triangles) {}
    ~TriangleMesh() override {}

public:
    // --- Core Overrides ---
    TriangleMesh &Clear() override;
    // Note: IsEmpty(), GetMinBound(), GetMaxBound(), GetCenter() are inherited from MeshBase if sufficient

    // --- Transformations ---
    TriangleMesh &Transform(const Eigen::Matrix4d &transformation) override;
    TriangleMesh &Rotate(const Eigen::Matrix3d &R,
                         const Eigen::Vector3d &center) override;

    // --- Basic Properties ---

    /// Returns `true` if the mesh contains triangles.
    bool HasTriangles() const {
        // Ensure vertices exist too as triangles index into vertices
        return !vertices_.empty() && !triangles_.empty();
    }

    /// Returns `true` if the mesh contains triangle normals.
    bool HasTriangleNormals() const {
        // Normals should exist only if triangles exist and sizes match
        return HasTriangles() && triangles_.size() == triangle_normals_.size();
    }
    // Note: HasVertexNormals(), HasVertexColors() are inherited from MeshBase

    // --- Normal Computation ---

    /// Normalize both triangle normals and vertex normals to length 1.
    TriangleMesh &NormalizeNormals() {
        // Normalize vertex normals (inherited)
        MeshBase::NormalizeNormals();
        // Normalize triangle normals
        for (size_t i = 0; i < triangle_normals_.size(); i++) {
            // Use stableNormalize to handle zero vectors gracefully
            triangle_normals_[i].stableNormalize();
            // Check for NaN might still be needed depending on stableNormalize guarantees
             if (std::isnan(triangle_normals_[i](0))) {
                 // Assign a default normal if normalization failed
                 triangle_normals_[i] = Eigen::Vector3d(0.0, 0.0, 1.0);
             }
        }
        return *this;
    }

    /// \brief Computes triangle normals.
    /// \param normalized If true, normalize the computed normals to unit length.
    /// Assumes vertices and triangles are populated.
    TriangleMesh &ComputeTriangleNormals(bool normalized = true);

    /// \brief Computes vertex normals by averaging adjacent triangle normals.
    /// \param normalized If true, normalize the computed normals to unit length.
    /// Calls ComputeTriangleNormals if triangle normals are missing.
    TriangleMesh &ComputeVertexNormals(bool normalized = true);

    // --- Merge Operators Removed ---
    // TriangleMesh &operator+=(const TriangleMesh &mesh);
    // TriangleMesh operator+(const TriangleMesh &mesh) const;

    // --- Advanced Methods Removed ---
    // (Adjacency, UVs, Area, Volume, Editing, etc.)

public:
    /// List of triangles denoted by the index of points forming the triangle.
    std::vector<Eigen::Vector3i> triangles_;
    /// Triangle normals. Size should match triangles_.
    std::vector<Eigen::Vector3d> triangle_normals_;

    // --- Non-Basic Members Removed ---
    // std::vector<std::unordered_set<int>> adjacency_list_;
    // std::vector<Eigen::Vector2d> triangle_uvs_;
    // std::vector<Material> materials_;           // Assuming Material type
    // std::vector<int> triangle_material_ids_;
    // std::vector<Image> textures_;              // Assuming Image type
};

} // namespace geometry
} // namespace tiny3d
