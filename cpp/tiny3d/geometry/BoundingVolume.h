// ----------------------------------------------------------------------------
// -                      Tiny3D: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------
//
// Minimal version containing only AxisAlignedBoundingBox.
// OrientedBoundingBox class removed.
//

#pragma once

#include <Eigen/Core>
#include <vector>
#include <string> // For GetPrintInfo

#include "tiny3d/geometry/Geometry3D.h"

namespace tiny3d {
namespace geometry {

// Forward declare OrientedBoundingBox ONLY if AxisAlignedBoundingBox needs it
// (which it doesn't in this simplified version)
// class OrientedBoundingBox;

/// \class AxisAlignedBoundingBox
///
/// \brief A bounding box that is aligned along the coordinate axes.
/// Basic version.
class AxisAlignedBoundingBox : public Geometry3D {
public:
    /// \brief Default constructor.
    AxisAlignedBoundingBox()
        : Geometry3D(Geometry::GeometryType::AxisAlignedBoundingBox),
          min_bound_(0, 0, 0),
          max_bound_(0, 0, 0),
          color_(1, 1, 1) {} // Initialize color
    /// \brief Parameterized constructor.
    AxisAlignedBoundingBox(const Eigen::Vector3d& min_bound,
                           const Eigen::Vector3d& max_bound);
    ~AxisAlignedBoundingBox() override {}

public:
    AxisAlignedBoundingBox& Clear() override;
    bool IsEmpty() const override;
    Eigen::Vector3d GetMinBound() const override;
    Eigen::Vector3d GetMaxBound() const override;
    Eigen::Vector3d GetCenter() const override;

    AxisAlignedBoundingBox GetAxisAlignedBoundingBox() const override; // Returns self

    // --- OBB Methods Removed ---
    // virtual OrientedBoundingBox GetOrientedBoundingBox(bool robust = false) const override;
    // virtual OrientedBoundingBox GetMinimalOrientedBoundingBox(bool robust = false) const override;

    AxisAlignedBoundingBox& Transform(const Eigen::Matrix4d& transformation) override; // Logs error
    AxisAlignedBoundingBox& Translate(const Eigen::Vector3d& translation, bool relative = true) override;
    AxisAlignedBoundingBox& Scale(const double scale, const Eigen::Vector3d& center) override;
    AxisAlignedBoundingBox& Rotate(const Eigen::Matrix3d& R, const Eigen::Vector3d& center) override; // Logs error

    AxisAlignedBoundingBox& operator+=(const AxisAlignedBoundingBox& other);

    Eigen::Vector3d GetExtent() const { return (max_bound_ - min_bound_); }
    Eigen::Vector3d GetHalfExtent() const { return GetExtent() * 0.5; }
    double GetMaxExtent() const { return GetExtent().maxCoeff(); }

    // --- Percentage Methods Removed (Optional) ---
    // double GetXPercentage(double x) const;
    // double GetYPercentage(double y) const;
    // double GetZPercentage(double z) const;

    double Volume() const;
    std::vector<Eigen::Vector3d> GetBoxPoints() const;
    std::vector<size_t> GetPointIndicesWithinBoundingBox(
            const std::vector<Eigen::Vector3d>& points) const;
    std::string GetPrintInfo() const;

    /// Creates the bounding box that encloses the set of points.
    static AxisAlignedBoundingBox CreateFromPoints(
            const std::vector<Eigen::Vector3d>& points);

public:
    Eigen::Vector3d min_bound_;
    Eigen::Vector3d max_bound_;
    Eigen::Vector3d color_;
};

// --- OrientedBoundingBox Class Removed ---

} // namespace geometry
} // namespace tiny3d
