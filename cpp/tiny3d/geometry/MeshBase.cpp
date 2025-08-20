// ----------------------------------------------------------------------------
// -                      Tiny3D: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------
//
// Minimal version for basic functionality. Implements methods declared
// in the basic MeshBase.h. Removed implementations for non-basic
// features (oriented BB, convex hull, merge operators).
// Assumes helper functions (ComputeMinBound, TransformPoints etc.) exist.
//

#include "tiny3d/geometry/MeshBase.h"

#include <Eigen/Dense>
#include <vector>

#include "tiny3d/geometry/BoundingVolume.h" // For AxisAlignedBoundingBox, OrientedBoundingBox?

// Define helper functions if they aren't available elsewhere
// These are simplified examples and might differ from Tiny3D's actual helpers
namespace tiny3d {
namespace geometry {

// --- Class Implementations ---

MeshBase &MeshBase::Clear() {
    vertices_.clear();
    vertex_normals_.clear();
    vertex_colors_.clear();
    return *this;
}

bool MeshBase::IsEmpty() const {
    return !HasVertices();
}

Eigen::Vector3d MeshBase::GetMinBound() const {
    return ComputeMinBound(vertices_);
}

Eigen::Vector3d MeshBase::GetMaxBound() const {
    return ComputeMaxBound(vertices_);
}

Eigen::Vector3d MeshBase::GetCenter() const {
    return ComputeCenter(vertices_);
}

AxisAlignedBoundingBox MeshBase::GetAxisAlignedBoundingBox() const {
    // Assuming AxisAlignedBoundingBox has a constructor taking min/max bounds
    return AxisAlignedBoundingBox(GetMinBound(), GetMaxBound());
    // Or if it has CreateFromPoints:
    // return AxisAlignedBoundingBox::CreateFromPoints(vertices_);
}

// --- Oriented Bounding Boxes Removed ---
// OrientedBoundingBox MeshBase::GetOrientedBoundingBox(bool robust) const { ... }
// OrientedBoundingBox MeshBase::GetMinimalOrientedBoundingBox(bool robust) const { ... }

MeshBase &MeshBase::Transform(const Eigen::Matrix4d &transformation) {
    TransformPoints(transformation, vertices_);
    if (HasVertexNormals()) {
         TransformNormals(transformation, vertex_normals_);
    }
    // Colors are not generally transformed
    return *this;
}

MeshBase &MeshBase::Translate(const Eigen::Vector3d &translation, bool relative) {
    TranslatePoints(translation, vertices_, relative);
    // Normals and colors are not affected by translation
    return *this;
}

MeshBase &MeshBase::Scale(const double scale, const Eigen::Vector3d &center) {
    ScalePoints(scale, vertices_, center);
    // Scaling affects normals if non-uniform, but typically MeshBase assumes uniform
    // Normals don't change direction with uniform scale, only magnitude (which normalize handles)
    // Colors are not affected
    return *this;
}

MeshBase &MeshBase::Rotate(const Eigen::Matrix3d &R, const Eigen::Vector3d &center) {
    RotatePoints(R, vertices_, center);
     if (HasVertexNormals()) {
        RotateNormals(R, vertex_normals_);
     }
    // Colors are not affected by rotation
    return *this;
}

// --- Merge Operators Removed ---
// MeshBase &MeshBase::operator+=(const MeshBase &mesh) { ... }
// MeshBase MeshBase::operator+(const MeshBase &mesh) const { ... }

// --- Advanced Methods Removed ---
// ComputeConvexHull() implementation would be here

} // namespace geometry
} // namespace tiny3d
