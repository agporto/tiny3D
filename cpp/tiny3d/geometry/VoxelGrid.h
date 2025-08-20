// ----------------------------------------------------------------------------
// -                      Tiny3D: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------
//
// Modified version: Removed VoxelPoolingMode and color dependency
// in the CreateFromPointCloud* methods. Voxels will be created
// based on occupancy only, with a default color assigned in the
// implementation (.cpp file).
// Also removed the operator+= and operator+ declarations.
//

#pragma once

#include <Eigen/Core>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional> // for std::hash

#include "tiny3d/geometry/Geometry3D.h"
#include "tiny3d/utility/Logging.h"

namespace tiny3d {

namespace geometry {

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

class PointCloud;
class AxisAlignedBoundingBox;

class Voxel {
public:
    Voxel() {}
    Voxel(const Eigen::Vector3i &grid_index) : grid_index_(grid_index) {}
    Voxel(const Eigen::Vector3i &grid_index, const Eigen::Vector3d &color)
        : grid_index_(grid_index), color_(color) {}
    ~Voxel() {}

public:
    Eigen::Vector3i grid_index_ = Eigen::Vector3i(0, 0, 0);
    Eigen::Vector3d color_ = Eigen::Vector3d(0, 0, 0);
};

class VoxelGrid : public Geometry3D {
public:
    VoxelGrid() : Geometry3D(Geometry::GeometryType::VoxelGrid) {}
    VoxelGrid(const VoxelGrid &src_voxel_grid);
    ~VoxelGrid() override {}

    VoxelGrid &Clear() override;
    bool IsEmpty() const override;
    Eigen::Vector3d GetMinBound() const override;
    Eigen::Vector3d GetMaxBound() const override;
    Eigen::Vector3d GetCenter() const override;
    AxisAlignedBoundingBox GetAxisAlignedBoundingBox() const override;

    VoxelGrid &Transform(const Eigen::Matrix4d &transformation) override;
    VoxelGrid &Translate(const Eigen::Vector3d &translation,
                         bool relative = true) override;
    VoxelGrid &Scale(const double scale,
                     const Eigen::Vector3d &center) override;
    VoxelGrid &Rotate(const Eigen::Matrix3d &R,
                      const Eigen::Vector3d &center) override;

    bool HasVoxels() const { return !voxels_.empty(); }
    bool HasColors() const { return true; }

    static std::shared_ptr<VoxelGrid> CreateFromPointCloud(
            const PointCloud &input, double voxel_size);

    static std::shared_ptr<VoxelGrid> CreateFromPointCloudWithinBounds(
            const PointCloud &input,
            double voxel_size,
            const Eigen::Vector3d &min_bound,
            const Eigen::Vector3d &max_bound);

    std::vector<Voxel> GetVoxels() const;

public:
    double voxel_size_ = 0.0;
    Eigen::Vector3d origin_ = Eigen::Vector3d::Zero();
    std::unordered_map<Eigen::Vector3i,
                       Voxel,
                       internal::hash_eigen<Eigen::Vector3i>> voxels_;
};

} // namespace geometry
} // namespace tiny3d

