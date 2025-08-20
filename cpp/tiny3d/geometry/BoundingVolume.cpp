// ----------------------------------------------------------------------------
// -                      Tiny3D: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------
//
// Minimal version containing only AxisAlignedBoundingBox implementation.
// OrientedBoundingBox implementation removed.
//

#include "tiny3d/geometry/BoundingVolume.h"

#include <Eigen/Dense> // For cwise operations
#include <iostream>
#include <vector>
#include <numeric> // For std::accumulate
#include <limits>  // For numeric_limits

#include "tiny3d/utility/Logging.h"
#include <fmt/format.h>


namespace tiny3d {
namespace geometry {

// --- OrientedBoundingBox Implementation Removed ---


// --- AxisAlignedBoundingBox Implementation ---

AxisAlignedBoundingBox::AxisAlignedBoundingBox(const Eigen::Vector3d& min_bound,
                                               const Eigen::Vector3d& max_bound)
    : Geometry3D(Geometry::GeometryType::AxisAlignedBoundingBox),
      min_bound_(min_bound),
      max_bound_(max_bound),
      color_(1, 1, 1) // Default color
{
    // Ensure min_bound <= max_bound component-wise
    if ((max_bound_.array() < min_bound_.array()).any()) {
        utility::LogWarning(
                "[AxisAlignedBoundingBox] Max bound {} is smaller than min bound {} "
                "in one or more axes. Correcting bounds.",
                max_bound_, min_bound_);
        // Correct the bounds
        Eigen::Vector3d corrected_min = min_bound_.array().min(max_bound_.array()).matrix();
        Eigen::Vector3d corrected_max = min_bound_.array().max(max_bound_.array()).matrix();
        min_bound_ = corrected_min;
        max_bound_ = corrected_max;
    }
}


AxisAlignedBoundingBox& AxisAlignedBoundingBox::Clear() {
    min_bound_.setZero();
    max_bound_.setZero();
    color_.setOnes(); // Reset color to default white
    return *this;
}

bool AxisAlignedBoundingBox::IsEmpty() const {
    // Consider empty if volume is non-positive or bounds are invalid
    return Volume() <= 1e-12 || (max_bound_.array() < min_bound_.array()).any();
}

Eigen::Vector3d AxisAlignedBoundingBox::GetMinBound() const {
    return min_bound_;
}

Eigen::Vector3d AxisAlignedBoundingBox::GetMaxBound() const {
    return max_bound_;
}

Eigen::Vector3d AxisAlignedBoundingBox::GetCenter() const {
    // Avoid potential issues if bounds are invalid (e.g., NaN)
    if (IsEmpty()) {
        return Eigen::Vector3d::Zero();
    }
    return (min_bound_ + max_bound_) * 0.5;
}

AxisAlignedBoundingBox AxisAlignedBoundingBox::GetAxisAlignedBoundingBox() const {
    return *this; // Return self
}

// --- OBB Methods Removed ---
// OrientedBoundingBox AxisAlignedBoundingBox::GetOrientedBoundingBox(...) const { ... }
// OrientedBoundingBox AxisAlignedBoundingBox::GetMinimalOrientedBoundingBox(...) const { ... }

AxisAlignedBoundingBox& AxisAlignedBoundingBox::Transform(
        const Eigen::Matrix4d& /*transformation*/) {
    // A general transformation results in an OBB, not AABB.
    utility::LogError(
            "[AxisAlignedBoundingBox::Transform] Cannot apply general transform. "
            "Convert to OrientedBoundingBox first or use Translate/Scale.");
    return *this;
}

AxisAlignedBoundingBox& AxisAlignedBoundingBox::Translate(
        const Eigen::Vector3d& translation, bool relative) {
    if (relative) {
        min_bound_ += translation;
        max_bound_ += translation;
    } else {
        // Move the center to the new translation
        const Eigen::Vector3d current_center = GetCenter();
        const Eigen::Vector3d shift = translation - current_center;
        min_bound_ += shift;
        max_bound_ += shift;
    }
    return *this;
}

AxisAlignedBoundingBox& AxisAlignedBoundingBox::Scale(
        const double scale, const Eigen::Vector3d& center) {
    // Scale bounds relative to the center
    min_bound_ = center + scale * (min_bound_ - center);
    max_bound_ = center + scale * (max_bound_ - center);
    // Ensure min <= max after scaling, especially if scale is negative
    if (scale < 0) {
         std::swap(min_bound_, max_bound_);
    }
    return *this;
}

AxisAlignedBoundingBox& AxisAlignedBoundingBox::Rotate(
        const Eigen::Matrix3d& /*R*/, const Eigen::Vector3d& /*center*/) {
    // Rotation results in an OBB, not AABB.
    utility::LogError(
            "[AxisAlignedBoundingBox::Rotate] Cannot rotate AABB. "
            "Convert to OrientedBoundingBox first.");
    return *this;
}

AxisAlignedBoundingBox& AxisAlignedBoundingBox::operator+=(
        const AxisAlignedBoundingBox& other) {
    if (this->IsEmpty()) {
        *this = other; // If current is empty, just copy the other
    } else if (!other.IsEmpty()) {
        // Expand bounds to include the other box
        min_bound_ = min_bound_.array().min(other.min_bound_.array()).matrix();
        max_bound_ = max_bound_.array().max(other.max_bound_.array()).matrix();
    }
    // If other is empty, do nothing
    return *this;
}

AxisAlignedBoundingBox AxisAlignedBoundingBox::CreateFromPoints(
        const std::vector<Eigen::Vector3d>& points) {
    if (points.empty()) {
        utility::LogWarning("[AxisAlignedBoundingBox::CreateFromPoints] Input points vector is empty.");
        // Return a default empty box
        return AxisAlignedBoundingBox(Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero());
    }
    // Use the protected helper methods from Geometry3D (assuming they exist)
    // Or reimplement here using std::accumulate
    Eigen::Vector3d min_b = std::accumulate(
            points.begin() + 1, points.end(), points[0],
            [](const Eigen::Vector3d& a, const Eigen::Vector3d& b) {
                return a.array().min(b.array()).matrix();
            });
     Eigen::Vector3d max_b = std::accumulate(
            points.begin() + 1, points.end(), points[0],
            [](const Eigen::Vector3d& a, const Eigen::Vector3d& b) {
                return a.array().max(b.array()).matrix();
            });

    return AxisAlignedBoundingBox(min_b, max_b);
}

double AxisAlignedBoundingBox::Volume() const {
    // Check for invalid bounds before calculating volume
    if ((max_bound_.array() < min_bound_.array()).any()) {
        return 0.0;
    }
    return GetExtent().prod();
}

std::vector<Eigen::Vector3d> AxisAlignedBoundingBox::GetBoxPoints() const {
    std::vector<Eigen::Vector3d> points(8);
    Eigen::Vector3d extent = GetExtent();
    // Check for invalid bounds
    if (extent.minCoeff() < 0) {
        // Return points at min_bound if extent is negative (invalid box)
        std::fill(points.begin(), points.end(), min_bound_);
        return points;
    }
    points[0] = min_bound_;
    points[1] = min_bound_ + Eigen::Vector3d(extent(0), 0, 0);
    points[2] = min_bound_ + Eigen::Vector3d(0, extent(1), 0);
    points[3] = min_bound_ + Eigen::Vector3d(0, 0, extent(2));
    points[4] = min_bound_ + Eigen::Vector3d(extent(0), extent(1), 0); // Corrected index 4
    points[5] = min_bound_ + Eigen::Vector3d(0, extent(1), extent(2)); // Corrected index 5
    points[6] = min_bound_ + Eigen::Vector3d(extent(0), 0, extent(2)); // Corrected index 6
    points[7] = max_bound_; // Corrected index 7
    return points;
}

std::vector<size_t> AxisAlignedBoundingBox::GetPointIndicesWithinBoundingBox(
        const std::vector<Eigen::Vector3d>& points) const {
    std::vector<size_t> indices;
    // Add epsilon for floating point comparisons if needed
    const double epsilon = 1e-9;
    for (size_t idx = 0; idx < points.size(); idx++) {
        const auto& point = points[idx];
        // Check if point is within or on the boundary
        if ((point.array() >= (min_bound_.array() - epsilon)).all() &&
            (point.array() <= (max_bound_.array() + epsilon)).all()) {
            indices.push_back(idx);
        }
    }
    return indices;
}

std::string AxisAlignedBoundingBox::GetPrintInfo() const {
    return fmt::format("AxisAlignedBoundingBox: min: ({:.4f}, {:.4f}, {:.4f}), max: ({:.4f}, {:.4f}, {:.4f})",
                       min_bound_(0), min_bound_(1), min_bound_(2),
                       max_bound_(0), max_bound_(1), max_bound_(2));
}

} // namespace geometry
} // namespace tiny3d
