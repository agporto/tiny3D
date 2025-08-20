// ----------------------------------------------------------------------------
// -                      Tiny3D: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------
//
// Modified version: Implements VoxelGrid based on the modified header
// (voxelgrid_header_no_color). Removed color dependency in creation
// and removed merge operators. Includes implementations for
// CreateFromPointCloud* methods.
//

#include "tiny3d/geometry/VoxelGrid.h"

#include <cmath>         // For std::floor
#include <limits>        // For std::numeric_limits
#include <memory>        // For std::shared_ptr, std::make_shared
#include <unordered_set> // For occupancy tracking during creation
#include <vector>        // For std::vector

#include "tiny3d/geometry/BoundingVolume.h"
#include "tiny3d/geometry/PointCloud.h" // Needed for CreateFromPointCloud*
#include "tiny3d/utility/Eigen.h" // Ensure Eigen types and hash are available
#include "tiny3d/utility/Logging.h"
// #include "tiny3d/utility/Helper.h" // Include if needed

namespace tiny3d {
namespace geometry {

// --- Constructors and Basic Operations ---

VoxelGrid::VoxelGrid(const VoxelGrid &src_voxel_grid)
    : Geometry3D(Geometry::GeometryType::VoxelGrid),
      voxel_size_(src_voxel_grid.voxel_size_),
      origin_(src_voxel_grid.origin_),
      voxels_(src_voxel_grid.voxels_) {}

VoxelGrid &VoxelGrid::Clear() {
    voxel_size_ = 0.0;
    origin_ = Eigen::Vector3d::Zero();
    voxels_.clear();
    return *this;
}

bool VoxelGrid::IsEmpty() const {
    // Delegate to HasVoxels for consistency
    return !HasVoxels();
}

// --- Bound Calculation ---

Eigen::Vector3d VoxelGrid::GetMinBound() const {
    if (!HasVoxels()) {
        // Return origin or perhaps an invalid bound if preferred
        return origin_;
    } else {
        // Initialize with the first voxel's index
        Eigen::Vector3i min_grid_index = voxels_.begin()->first;
        // Iterate through all voxels to find the minimum index components
        for (const auto &it : voxels_) {
            // Use Eigen's component-wise min operation
            min_grid_index = min_grid_index.array()
                                     .min(it.first.array())
                                     .matrix();
        }
        // Convert grid index back to world coordinates
        return origin_ + min_grid_index.cast<double>() * voxel_size_;
    }
}

Eigen::Vector3d VoxelGrid::GetMaxBound() const {
    if (!HasVoxels()) {
        // Return origin or perhaps an invalid bound if preferred
        return origin_;
    } else {
        // Initialize with the first voxel's index
        Eigen::Vector3i max_grid_index = voxels_.begin()->first;
        // Iterate through all voxels to find the maximum index components
        for (const auto &it : voxels_) {
            // Use Eigen's component-wise max operation
            max_grid_index = max_grid_index.array()
                                     .max(it.first.array())
                                     .matrix();
        }
        // Convert grid index back to world coordinates. Add 1 to include the
        // full extent of the max index voxel.
        return origin_ + ((max_grid_index.cast<double>().array() + 1.0) * voxel_size_).matrix();
    }
}

Eigen::Vector3d VoxelGrid::GetCenter() const {
    if (!HasVoxels()) {
        return Eigen::Vector3d::Zero(); // Or return origin_
    }
    Eigen::Vector3d center_sum = Eigen::Vector3d::Zero();
    const Eigen::Vector3d half_voxel(0.5 * voxel_size_, 0.5 * voxel_size_,
                                     0.5 * voxel_size_);
    // Sum the centers of all voxels
    for (const auto &it : voxels_) {
        // Calculate center of the current voxel in world coordinates
        center_sum += origin_ + it.first.cast<double>() * voxel_size_ + half_voxel;
    }
    // Average the centers
    return center_sum / double(voxels_.size());
}

// --- Bounding Box Creation ---

AxisAlignedBoundingBox VoxelGrid::GetAxisAlignedBoundingBox() const {
    return AxisAlignedBoundingBox(GetMinBound(), GetMaxBound());
}


// --- Transformations (Implementations likely needed but not shown) ---

VoxelGrid &VoxelGrid::Transform(const Eigen::Matrix4d &transformation) {
    // Implementation would involve transforming origin and potentially
    // re-inserting voxels if rotation/scaling changes their indices.
    // This can be complex. Often, voxel grids are rebuilt after transformation.
    utility::LogWarning("VoxelGrid::Transform is not fully implemented.");
    // Simple transformation of origin (incomplete for general transforms)
    origin_ = (transformation * origin_.homogeneous()).head<3>();
    // Voxel data needs careful handling, especially with rotation/scaling
    return *this;
}

VoxelGrid &VoxelGrid::Translate(const Eigen::Vector3d &translation,
                                bool relative) {
    if (relative) {
        origin_ += translation;
    } else {
        origin_ = translation; // Sets new origin, implies grid structure moves
    }
    return *this;
}

VoxelGrid &VoxelGrid::Scale(const double scale, const Eigen::Vector3d &center) {
    // Scaling changes voxel size and origin relative to center.
    // Requires careful recalculation or rebuilding.
    utility::LogWarning("VoxelGrid::Scale is not fully implemented.");
    origin_ = center + scale * (origin_ - center);
    voxel_size_ *= scale;
    // Voxel indices might need updating or grid rebuild
    return *this;
}

VoxelGrid &VoxelGrid::Rotate(const Eigen::Matrix3d &R,
                             const Eigen::Vector3d &center) {
    // Rotation changes origin relative to center. Voxel indices change.
    // Requires careful recalculation or rebuilding.
    utility::LogWarning("VoxelGrid::Rotate is not fully implemented.");
    origin_ = center + R * (origin_ - center);
    // Voxel data needs careful handling
    return *this;
}


// --- Merge Operators (Removed as requested) ---
// VoxelGrid &VoxelGrid::operator+=(const VoxelGrid &voxelgrid) { ... }
// VoxelGrid VoxelGrid::operator+(const VoxelGrid &voxelgrid) const { ... }


// --- Voxel Access ---

std::vector<Voxel> VoxelGrid::GetVoxels() const {
    std::vector<Voxel> result;
    result.reserve(voxels_.size());
    for (const auto &keyval : voxels_) {
        result.push_back(keyval.second);
    }
    return result;
}

// Helper function to add a voxel (might need to be declared in header if public)
//void VoxelGrid::AddVoxel(const Voxel& voxel) {
//    voxels_[voxel.grid_index_] = voxel;
//}


// --- Static Creation Methods (Implementations from previous steps) ---

std::shared_ptr<VoxelGrid> VoxelGrid::CreateFromPointCloudWithinBounds(
        const PointCloud &input,
        double voxel_size,
        const Eigen::Vector3d &min_bound,
        const Eigen::Vector3d &max_bound) {
    auto output = std::make_shared<VoxelGrid>();
    if (voxel_size <= 0.0) {
        utility::LogError("voxel_size must be positive.");
        return output; // Return empty grid
    }

    double max_extent = (max_bound - min_bound).maxCoeff();
    if (max_extent / voxel_size > static_cast<double>(std::numeric_limits<int>::max())) {
         utility::LogError("voxel_size is potentially too small for the given bounds, may lead to integer overflow in indices.");
         return output; // Return empty grid
    }

    output->voxel_size_ = voxel_size;
    output->origin_ = min_bound;

    // Use an unordered_set to store unique occupied voxel indices.
    std::unordered_set<Eigen::Vector3i, internal::hash_eigen<Eigen::Vector3i>>
            occupied_voxel_indices;

    Eigen::Vector3d ref_coord;
    Eigen::Vector3i voxel_index;

    // Iterate through each point in the input PointCloud
    for (size_t i = 0; i < input.points_.size(); ++i) {
        const Eigen::Vector3d &point = input.points_[i];

        // Optional: Explicit check if point is within bounds before proceeding
         if ((point.array() < min_bound.array()).any() || (point.array() >= max_bound.array()).any()) {
             continue; // Skip points strictly outside the bounds
         }

        // Calculate the point's coordinates relative to the origin and voxel size
        ref_coord = (point - min_bound) / voxel_size;

        // Compute the integer voxel index using floor
        voxel_index(0) = static_cast<int>(std::floor(ref_coord(0)));
        voxel_index(1) = static_cast<int>(std::floor(ref_coord(1)));
        voxel_index(2) = static_cast<int>(std::floor(ref_coord(2)));

        // Insert the calculated voxel index into the set.
        occupied_voxel_indices.insert(voxel_index);
    }

    // Define a default color for the voxels (e.g., grey)
    const Eigen::Vector3d default_color(0.5, 0.5, 0.5);

    // Reserve space in the map for efficiency
    output->voxels_.reserve(occupied_voxel_indices.size());

    // Iterate through the set of unique occupied voxel indices
    for (const auto &grid_index : occupied_voxel_indices) {
        // Add a voxel to the output VoxelGrid with the index and the default color.
        // Using direct map insertion which implicitly calls AddVoxel logic here.
         output->voxels_.emplace(grid_index, Voxel(grid_index, default_color));
        // Or if AddVoxel is preferred:
        // output->AddVoxel(geometry::Voxel(grid_index, default_color));
    }

    utility::LogDebug(
            "Pointcloud is voxelized from {:d} points to {:d} voxels (color "
            "information ignored).",
            (int)input.points_.size(), (int)output->voxels_.size());

    return output;
}

std::shared_ptr<VoxelGrid> VoxelGrid::CreateFromPointCloud(
        const PointCloud &input,
        double voxel_size) {
    if (input.IsEmpty()) {
        utility::LogWarning("Input point cloud is empty for VoxelGrid creation.");
        return std::make_shared<VoxelGrid>(); // Return empty grid
    }
    if (voxel_size <= 0.0) {
        utility::LogError("voxel_size must be positive for VoxelGrid creation.");
        return std::make_shared<VoxelGrid>(); // Return empty grid
    }

    // Calculate bounds based on the point cloud extents
    Eigen::Vector3d min_bound = input.GetMinBound();
    Eigen::Vector3d max_bound = input.GetMaxBound();
    // Add a small buffer (half voxel size) to ensure points near edges are included
    // Note: Original code added/subtracted full voxel_size3 * 0.5. Be consistent.
    const Eigen::Vector3d half_voxel(0.5 * voxel_size, 0.5 * voxel_size, 0.5 * voxel_size);
    min_bound -= half_voxel;
    max_bound += half_voxel;


    // Call the modified CreateFromPointCloudWithinBounds function
    return CreateFromPointCloudWithinBounds(input, voxel_size, min_bound,
                                            max_bound);
}


} // namespace geometry
} // namespace tiny3d
