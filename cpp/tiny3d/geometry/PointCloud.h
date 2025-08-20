// ----------------------------------------------------------------------------
// -                      Tiny3D: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------
//
// Minimal version for basic functionality plus VoxelDownSample and
// EstimateNormals.
//

#pragma once

#include <Eigen/Core>
#include <memory>
#include <vector>
#include <cmath> // For std::isnan

#include "tiny3d/geometry/Geometry3D.h"
#include "tiny3d/geometry/BoundingVolume.h" // For AxisAlignedBoundingBox
// Forward declare KDTreeSearchParam instead of including the full header
namespace tiny3d { namespace geometry { class KDTreeSearchParam; } }
// If KDTreeSearchParamKNN() default is needed, include might be required:
#include "tiny3d/geometry/KDTreeSearchParam.h"


namespace tiny3d {


// Define a local hasher for Eigen::Vector3i
namespace internal {
template <typename Derived>
struct hash_eigen {
    std::size_t operator()(const Eigen::MatrixBase<Derived>& v) const {
        std::size_t seed = 0;
        for (int i = 0; i < v.size(); ++i) {
            seed ^= std::hash<typename Derived::Scalar>()(v(i)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};
}  // namespace internal

namespace geometry {

/// \class PointCloud
///
/// \brief A point cloud consists of point coordinates, and optionally point
/// colors and point normals. Basic version with normal estimation.
class PointCloud : public Geometry3D {
public:
    /// \brief Default Constructor.
    PointCloud() : Geometry3D(Geometry::GeometryType::PointCloud) {}
    /// \brief Parameterized Constructor.
    ///
    /// \param points Points coordinates.
    PointCloud(const std::vector<Eigen::Vector3d> &points)
        : Geometry3D(Geometry::GeometryType::PointCloud), points_(points) {}
    ~PointCloud() override {}

public:
    // --- Core Methods ---
    PointCloud &Clear() override;
    bool IsEmpty() const override;
    Eigen::Vector3d GetMinBound() const override;
    Eigen::Vector3d GetMaxBound() const override;
    Eigen::Vector3d GetCenter() const override;
    AxisAlignedBoundingBox GetAxisAlignedBoundingBox() const override;

    // --- Transformations ---
    PointCloud &Transform(const Eigen::Matrix4d &transformation) override;
    PointCloud &Translate(const Eigen::Vector3d &translation,
                          bool relative = true) override;
    PointCloud &Scale(const double scale,
                      const Eigen::Vector3d &center) override;
    PointCloud &Rotate(const Eigen::Matrix3d &R,
                       const Eigen::Vector3d &center) override;

    // --- Basic Properties ---
    bool HasPoints() const { return !points_.empty(); }
    bool HasNormals() const {
        return HasPoints() && normals_.size() == points_.size();
    }
    bool HasColors() const {
        return HasPoints() && colors_.size() == points_.size();
    }

    // --- Utility Methods ---
    PointCloud &NormalizeNormals(); // Implementation in cpp
    PointCloud &PaintUniformColor(const Eigen::Vector3d &color); // Implementation in cpp

    // --- Downsampling ---
    std::shared_ptr<PointCloud> VoxelDownSample(double voxel_size) const;

    // --- Normal Estimation ---
    /// \brief Function to compute the normals of a point cloud by
    /// analyzing eigenvalues/vectors of the covariance matrix of
    /// neighboring points.
    ///
    /// \param search_param The KDTree search parameters for neighborhood
    /// search (e.g., KDTreeSearchParamKNN, KDTreeSearchParamRadius).
    /// \param fast_normal_computation If true, uses a faster method for
    /// eigenvector computation which might be less stable for planar cases.
    void EstimateNormals(
            const KDTreeSearchParam &search_param = KDTreeSearchParamKNN(),
            bool fast_normal_computation = true);

public:
    /// Points coordinates.
    std::vector<Eigen::Vector3d> points_;
    /// Points normals. Size should match points_.
    std::vector<Eigen::Vector3d> normals_;
    /// RGB colors of points. Size should match points_.
    std::vector<Eigen::Vector3d> colors_;
};

} // namespace geometry
} // namespace tiny3d
