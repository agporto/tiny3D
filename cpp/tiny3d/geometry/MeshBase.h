// ----------------------------------------------------------------------------
// -                      Tiny3D: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------
//
// Minimal version for basic functionality. Removed enums, oriented bounding
// boxes, convex hull, merge operators.
//

#pragma once

#include <Eigen/Core>
#include <memory>
#include <vector>
#include <cmath> // For std::isnan

#include "tiny3d/geometry/Geometry3D.h"
// Assuming AxisAlignedBoundingBox is needed and declared elsewhere
#include "tiny3d/geometry/BoundingVolume.h" // Likely where AABB is defined
// #include "tiny3d/utility/Helper.h" // May not be needed

namespace tiny3d {
namespace geometry {

// Forward declarations if needed
// class PointCloud;
// class TriangleMesh;

/// \class MeshBase
///
/// \brief MeshBase Class. Base class for meshes.
///
/// Contains vertices. Optionally, may also contain vertex normals and vertex
/// colors.
class MeshBase : public Geometry3D {
public:
    // --- Enums Removed ---
    // enum class SimplificationContraction { Average, Quadric };
    // enum class FilterScope { All, Color, Normal, Vertex };

    /// \brief Default Constructor.
    MeshBase() : Geometry3D(Geometry::GeometryType::MeshBase) {}
    ~MeshBase() override {}

public:
    // --- Core Methods ---
    MeshBase &Clear() override;
    bool IsEmpty() const override;
    Eigen::Vector3d GetMinBound() const override;
    Eigen::Vector3d GetMaxBound() const override;
    Eigen::Vector3d GetCenter() const override;

    /// Creates the axis-aligned bounding box around the vertices of the object.
    AxisAlignedBoundingBox GetAxisAlignedBoundingBox() const override;

    // --- Oriented Bounding Boxes Removed ---
    // virtual OrientedBoundingBox GetOrientedBoundingBox(
    //         bool robust = false) const override;
    // virtual OrientedBoundingBox GetMinimalOrientedBoundingBox(
    //         bool robust = false) const override;

    // --- Transformations ---
    MeshBase &Transform(const Eigen::Matrix4d &transformation) override;
    MeshBase &Translate(const Eigen::Vector3d &translation,
                        bool relative = true) override;
    MeshBase &Scale(const double scale,
                    const Eigen::Vector3d &center) override;
    MeshBase &Rotate(const Eigen::Matrix3d &R,
                     const Eigen::Vector3d &center) override;

    // --- Merge Operators Removed ---
    // MeshBase &operator+=(const MeshBase &mesh);
    // MeshBase operator+(const MeshBase &mesh) const;

    // --- Basic Properties ---

    /// Returns `True` if the mesh contains vertices.
    bool HasVertices() const { return !vertices_.empty(); }

    /// Returns `True` if the mesh contains vertex normals.
    bool HasVertexNormals() const {
        return HasVertices() && vertex_normals_.size() == vertices_.size();
    }

    /// Returns `True` if the mesh contains vertex colors.
    bool HasVertexColors() const {
        return HasVertices() && vertex_colors_.size() == vertices_.size();
    }

    // --- Utility Methods ---

    /// Normalize vertex normals to length 1.
    MeshBase &NormalizeNormals() {
        for (size_t i = 0; i < vertex_normals_.size(); i++) {
            // Use stableNormalize to handle zero vectors gracefully
            vertex_normals_[i].stableNormalize();
             if (std::isnan(vertex_normals_[i](0))) {
                 // Assign a default normal if normalization failed
                 vertex_normals_[i] = Eigen::Vector3d(0.0, 0.0, 1.0);
             }
        }
        return *this;
    }

    /// \brief Assigns each vertex in the Mesh the same color.
    ///
    /// \param color RGB colors of vertices.
    MeshBase &PaintUniformColor(const Eigen::Vector3d &color) {
        // Resize vertex_colors_ if necessary, then fill with color
        vertex_colors_.resize(vertices_.size());
        std::fill(vertex_colors_.begin(), vertex_colors_.end(), color);
        return *this;
    }

    // --- Advanced Methods Removed ---
    // ComputeConvexHull()

protected:
    // Constructor for derived classes
    MeshBase(Geometry::GeometryType type) : Geometry3D(type) {}
    // Constructor for derived classes initializing vertices
    MeshBase(Geometry::GeometryType type,
             const std::vector<Eigen::Vector3d> &vertices)
        : Geometry3D(type), vertices_(vertices) {}

    // Helper functions assumed to exist elsewhere or be simple static methods
    // These might need to be included/defined if not part of Geometry3D
    // static Eigen::Vector3d ComputeMinBound(const std::vector<Eigen::Vector3d>& points);
    // static Eigen::Vector3d ComputeMaxBound(const std::vector<Eigen::Vector3d>& points);
    // static Eigen::Vector3d ComputeCenter(const std::vector<Eigen::Vector3d>& points);
    // static void TransformPoints(const Eigen::Matrix4d& trans, std::vector<Eigen::Vector3d>& points);
    // static void TransformNormals(const Eigen::Matrix4d& trans, std::vector<Eigen::Vector3d>& normals);
    // static void TranslatePoints(const Eigen::Vector3d& trans, std::vector<Eigen::Vector3d>& points, bool relative);
    // static void ScalePoints(double scale, std::vector<Eigen::Vector3d>& points, const Eigen::Vector3d& center);
    // static void RotatePoints(const Eigen::Matrix3d& R, std::vector<Eigen::Vector3d>& points, const Eigen::Vector3d& center);
    // static void RotateNormals(const Eigen::Matrix3d& R, std::vector<Eigen::Vector3d>& normals);
    // static void ResizeAndPaintUniformColor(std::vector<Eigen::Vector3d>& colors, size_t size, const Eigen::Vector3d& color);


public:
    /// Vertex coordinates.
    std::vector<Eigen::Vector3d> vertices_;
    /// Vertex normals. Size should match vertices_.
    std::vector<Eigen::Vector3d> vertex_normals_;
    /// RGB colors of vertices. Size should match vertices_.
    std::vector<Eigen::Vector3d> vertex_colors_;
};

} // namespace geometry
} // namespace tiny3d
