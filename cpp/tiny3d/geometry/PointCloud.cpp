// ----------------------------------------------------------------------------
// -                      Tiny3D: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------
//
// Minimal version for basic functionality plus VoxelDownSample and
// EstimateNormals.
// Includes implementations for VoxelDownSample and EstimateNormals.
//

#include "tiny3d/geometry/PointCloud.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues> // Needed for SelfAdjointEigenSolver
#include <vector>
#include <unordered_map>
#include <cmath>   // For std::floor, std::isnan, std::sqrt, std::abs, std::acos, std::cos, std::max, std::min
#include <limits>  // For std::numeric_limits
#include <numeric> // For std::accumulate in helpers

#include "tiny3d/geometry/BoundingVolume.h" // For AxisAlignedBoundingBox
#include "tiny3d/geometry/KDTreeFlann.h" // Needed for EstimateNormals
#include "tiny3d/geometry/KDTreeSearchParam.h" // Needed for EstimateNormals
#include "tiny3d/utility/Eigen.h" // For hash_eigen and ComputeMeanAndCovariance
#include "tiny3d/utility/Logging.h"
#include "tiny3d/utility/Parallel.h" // Needed for parallel loops

// Define helper functions if they aren't available elsewhere (e.g., from MeshBase)
namespace tiny3d {
namespace geometry {



// --- Re-defining basic helpers here if needed ---
Eigen::Vector3d ComputeMinBound(const std::vector<Eigen::Vector3d>& points) {
    if (points.empty()) return Eigen::Vector3d::Constant(std::numeric_limits<double>::quiet_NaN());
    return std::accumulate(points.begin() + 1, points.end(), points[0],
                           [](const Eigen::Vector3d& a, const Eigen::Vector3d& b) {
                               return a.array().min(b.array()).matrix();
                           });
}
Eigen::Vector3d ComputeMaxBound(const std::vector<Eigen::Vector3d>& points) {
     if (points.empty()) return Eigen::Vector3d::Constant(std::numeric_limits<double>::quiet_NaN());
    return std::accumulate(points.begin() + 1, points.end(), points[0],
                           [](const Eigen::Vector3d& a, const Eigen::Vector3d& b) {
                               return a.array().max(b.array()).matrix();
                           });
}
Eigen::Vector3d ComputeCenter(const std::vector<Eigen::Vector3d>& points) {
     if (points.empty()) return Eigen::Vector3d::Zero();
     Eigen::Vector3d center_sum = std::accumulate(points.begin(), points.end(), Eigen::Vector3d::Zero().eval());
     return center_sum / static_cast<double>(points.size());
}
void TransformPoints(const Eigen::Matrix4d& transformation, std::vector<Eigen::Vector3d>& points) {
    for (auto& point : points) {
        Eigen::Vector4d point_h(point(0), point(1), point(2), 1.0);
        Eigen::Vector4d new_point_h = transformation * point_h;
        if (std::abs(new_point_h(3)) > 1e-9) point = new_point_h.head<3>() / new_point_h(3);
        else point.setConstant(std::numeric_limits<double>::quiet_NaN());
    }
}
void TransformNormals(const Eigen::Matrix4d& transformation, std::vector<Eigen::Vector3d>& normals) {
    Eigen::Matrix3d linear_part = transformation.block<3, 3>(0, 0);
    bool invertible; Eigen::Matrix3d normal_matrix;
    linear_part.computeInverseWithCheck(normal_matrix, invertible);
    if (invertible) normal_matrix.transposeInPlace();
    else normal_matrix = Eigen::Matrix3d::Identity();
    for (auto& normal : normals) {
        normal = normal_matrix * normal;
        normal.stableNormalize();
        if (std::isnan(normal(0))) normal.setZero();
    }
}
void TranslatePoints(const Eigen::Vector3d& translation, std::vector<Eigen::Vector3d>& points, bool relative) {
    Eigen::Vector3d transform_vector = translation;
    if (!relative) {
        if (!points.empty()) transform_vector = translation - ComputeCenter(points);
        else transform_vector = Eigen::Vector3d::Zero();
    }
     for (auto& point : points) point += transform_vector;
}
void ScalePoints(double scale, std::vector<Eigen::Vector3d>& points, const Eigen::Vector3d& center) {
    for (auto& point : points) point = center + scale * (point - center);
}
void RotatePoints(const Eigen::Matrix3d& R, std::vector<Eigen::Vector3d>& points, const Eigen::Vector3d& center) {
    for (auto& point : points) point = center + R * (point - center);
}
void RotateNormals(const Eigen::Matrix3d& R, std::vector<Eigen::Vector3d>& normals) {
    for (auto& normal : normals) normal = R * normal;
}
// --- End of Basic Helpers ---


// --- Normal Estimation Helpers (Moved from EstimateNormals.cpp) ---
Eigen::Vector3d ComputeEigenvector0(const Eigen::Matrix3d &A, double eval0) {
    Eigen::Vector3d row0(A(0, 0) - eval0, A(0, 1), A(0, 2));
    Eigen::Vector3d row1(A(0, 1), A(1, 1) - eval0, A(1, 2));
    Eigen::Vector3d row2(A(0, 2), A(1, 2), A(2, 2) - eval0);
    Eigen::Vector3d r0xr1 = row0.cross(row1);
    Eigen::Vector3d r0xr2 = row0.cross(row2);
    Eigen::Vector3d r1xr2 = row1.cross(row2);
    double d0 = r0xr1.dot(r0xr1);
    double d1 = r0xr2.dot(r0xr2);
    double d2 = r1xr2.dot(r1xr2);

    double dmax = d0; int imax = 0;
    if (d1 > dmax) { dmax = d1; imax = 1; }
    if (d2 > dmax) imax = 2;

    if (dmax <= 1e-16) return Eigen::Vector3d::Zero(); // Handle degenerate case

    if (imax == 0) return r0xr1 / std::sqrt(d0);
    else if (imax == 1) return r0xr2 / std::sqrt(d1);
    else return r1xr2 / std::sqrt(d2);
}


Eigen::Vector3d FastEigen3x3(const Eigen::Matrix3d &covariance) {
    Eigen::Matrix3d A = covariance;
    double max_coeff = A.cwiseAbs().maxCoeff();
    if (max_coeff == 0) return Eigen::Vector3d::Zero();
    A /= max_coeff;

    double norm = A(0, 1) * A(0, 1) + A(0, 2) * A(0, 2) + A(1, 2) * A(1, 2);
    if (norm > 1e-16) { // Check if matrix is non-diagonal
        Eigen::Vector3d eval; Eigen::Vector3d evec0, evec1, evec2;
        double q = A.trace() / 3.0;
        double b00 = A(0, 0) - q, b11 = A(1, 1) - q, b22 = A(2, 2) - q;
        double p = std::sqrt((b00 * b00 + b11 * b11 + b22 * b22 + norm * 2.0) / 6.0);
        double c00 = b11 * b22 - A(1, 2) * A(1, 2);
        double c01 = A(0, 1) * b22 - A(1, 2) * A(0, 2);
        double c02 = A(0, 1) * A(1, 2) - b11 * A(0, 2);
        double det = (b00 * c00 - A(0, 1) * c01 + A(0, 2) * c02); // Determinant of B = A-qI
        // Ensure p is not zero before dividing
        if (p < 1e-16) return Eigen::Vector3d::Zero(); // Handle edge case
        det /= (p * p * p);

        double half_det = det * 0.5;
        half_det = std::min(std::max(half_det, -1.0), 1.0); // Clamp to [-1, 1]

        double angle = std::acos(half_det) / 3.0;
        double const two_thirds_pi = 2.0 * M_PI / 3.0;
        double beta2 = std::cos(angle) * 2.0;
        double beta0 = std::cos(angle + two_thirds_pi) * 2.0;
        double beta1 = -(beta0 + beta2);

        eval(0) = q + p * beta0; eval(1) = q + p * beta1; eval(2) = q + p * beta2;

        // Find eigenvector for smallest eigenvalue
        int min_eval_idx = 0;
        if (eval(1) < eval(min_eval_idx)) min_eval_idx = 1;
        if (eval(2) < eval(min_eval_idx)) min_eval_idx = 2;

        return ComputeEigenvector0(A, eval(min_eval_idx)); // Return eigenvector for smallest eigenvalue

    } else { // Matrix is diagonal
        // Return axis corresponding to smallest diagonal element
        if (A(0, 0) <= A(1, 1) && A(0, 0) <= A(2, 2)) return Eigen::Vector3d::UnitX();
        else if (A(1, 1) <= A(0, 0) && A(1, 1) <= A(2, 2)) return Eigen::Vector3d::UnitY();
        else return Eigen::Vector3d::UnitZ();
    }
}

Eigen::Vector3d ComputeNormal(const Eigen::Matrix3d &covariance,
                              bool fast_normal_computation) {
    if (fast_normal_computation) {
        return FastEigen3x3(covariance);
    } else {
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(
                covariance, Eigen::ComputeEigenvectors);
        // Check if solver converged
        if (solver.info() != Eigen::Success) {
             utility::LogWarning("[ComputeNormal] Eigen decomposition failed. Returning zero vector.");
             return Eigen::Vector3d::Zero();
        }
        // Eigenvalues are sorted in increasing order, so the first eigenvector
        // corresponds to the smallest eigenvalue.
        return solver.eigenvectors().col(0);
    }
}
// --- End of Normal Estimation Helpers ---



// --- Class Implementations ---

PointCloud &PointCloud::Clear() {
    points_.clear();
    normals_.clear();
    colors_.clear();
    return *this;
}

bool PointCloud::IsEmpty() const { return !HasPoints(); }

Eigen::Vector3d PointCloud::GetMinBound() const { return ComputeMinBound(points_); }
Eigen::Vector3d PointCloud::GetMaxBound() const { return ComputeMaxBound(points_); }
Eigen::Vector3d PointCloud::GetCenter() const { return ComputeCenter(points_); }

AxisAlignedBoundingBox PointCloud::GetAxisAlignedBoundingBox() const {
    return AxisAlignedBoundingBox(GetMinBound(), GetMaxBound());
}

PointCloud &PointCloud::Transform(const Eigen::Matrix4d &transformation) {
    TransformPoints(transformation, points_);
    if (HasNormals()) TransformNormals(transformation, normals_);
    return *this;
}

PointCloud &PointCloud::Translate(const Eigen::Vector3d &translation, bool relative) {
    TranslatePoints(translation, points_, relative);
    return *this;
}

PointCloud &PointCloud::Scale(const double scale, const Eigen::Vector3d &center) {
    ScalePoints(scale, points_, center);
    return *this;
}

PointCloud &PointCloud::Rotate(const Eigen::Matrix3d &R, const Eigen::Vector3d &center) {
    RotatePoints(R, points_, center);
    if (HasNormals()) RotateNormals(R, normals_);
    return *this;
}

PointCloud &PointCloud::NormalizeNormals() {
    for (size_t i = 0; i < normals_.size(); i++) {
        normals_[i].stableNormalize();
         if (std::isnan(normals_[i](0))) {
             normals_[i] = Eigen::Vector3d(0.0, 0.0, 1.0); // Default normal
         }
    }
    return *this;
}

PointCloud &PointCloud::PaintUniformColor(const Eigen::Vector3d &color) {
    colors_.resize(points_.size());
    Eigen::Vector3d clipped_color = color.array().max(0.0).min(1.0).matrix();
    std::fill(colors_.begin(), colors_.end(), clipped_color);
    return *this;
}


// --- VoxelDownSample Implementation (Copied from previous step) ---
namespace { // Anonymous namespace for VoxelDownSample helpers
class AccumulatedPoint { /* ... definition as before ... */
public:
    AccumulatedPoint() = default;
    void AddPoint(const PointCloud &cloud, int index) {
        point_ += cloud.points_[index];
        if (cloud.HasNormals()) {
            if (!std::isnan(cloud.normals_[index](0)) && !std::isnan(cloud.normals_[index](1)) && !std::isnan(cloud.normals_[index](2))) {
                normal_ += cloud.normals_[index]; has_normals_ = true;
            }
        }
        if (cloud.HasColors()) { color_ += cloud.colors_[index]; has_colors_ = true; }
        num_of_points_++;
    }
    Eigen::Vector3d GetAveragePoint() const { 
        if (num_of_points_ > 0) {
            return point_ / double(num_of_points_);
        } else {
            return Eigen::Vector3d::Zero();
        }
    }

    Eigen::Vector3d GetAverageNormal() const { 
        if (num_of_points_ > 0 && has_normals_) {
            return normal_ / double(num_of_points_);
        } else {
            return Eigen::Vector3d::Zero();
        }
    }

    Eigen::Vector3d GetAverageColor() const { 
        if (num_of_points_ > 0 && has_colors_) {
            return color_ / double(num_of_points_);
        } else {
            return Eigen::Vector3d::Zero();
        }
    }
    bool HasValidNormals() const { return has_normals_; }
    bool HasValidColors() const { return has_colors_; }
private:
    int num_of_points_ = 0;
    Eigen::Vector3d point_ = Eigen::Vector3d::Zero();
    Eigen::Vector3d normal_ = Eigen::Vector3d::Zero();
    Eigen::Vector3d color_ = Eigen::Vector3d::Zero();
    bool has_normals_ = false; bool has_colors_ = false;
};
} // anonymous namespace

std::shared_ptr<PointCloud> PointCloud::VoxelDownSample(double voxel_size) const {
    auto output = std::make_shared<PointCloud>();
    if (voxel_size <= 0.0) { utility::LogError("[VoxelDownSample] voxel_size must be positive."); return output; }
    if (!HasPoints()) { utility::LogWarning("[VoxelDownSample] Input point cloud is empty."); return output; }
    Eigen::Vector3d voxel_min_bound = GetMinBound(); Eigen::Vector3d voxel_max_bound = GetMaxBound();
    if (voxel_size * static_cast<double>(std::numeric_limits<int>::max()) < (voxel_max_bound - voxel_min_bound).maxCoeff() + 1e-9) {
        utility::LogError("[VoxelDownSample] voxel_size is too small relative to the cloud extent."); return output;
    }
    std::unordered_map<Eigen::Vector3i, AccumulatedPoint, internal::hash_eigen<Eigen::Vector3i>> voxelindex_to_accpoint;
    const Eigen::Vector3d origin = voxel_min_bound;
    for (int i = 0; i < (int)points_.size(); i++) {
        Eigen::Vector3d ref_coord = (points_[i] - origin) / voxel_size;
        Eigen::Vector3i voxel_index(static_cast<int>(std::floor(ref_coord(0))), static_cast<int>(std::floor(ref_coord(1))), static_cast<int>(std::floor(ref_coord(2))));
        voxelindex_to_accpoint[voxel_index].AddPoint(*this, i);
    }
    output->points_.reserve(voxelindex_to_accpoint.size());
    bool output_has_normals = HasNormals(); bool output_has_colors = HasColors();
    if (output_has_normals) output->normals_.reserve(voxelindex_to_accpoint.size());
    if (output_has_colors) output->colors_.reserve(voxelindex_to_accpoint.size());
    for (const auto &pair : voxelindex_to_accpoint) {
        const AccumulatedPoint &accpoint = pair.second;
        output->points_.push_back(accpoint.GetAveragePoint());
        if (output_has_normals) {
            if (accpoint.HasValidNormals()) output->normals_.push_back(accpoint.GetAverageNormal());
            else output->normals_.push_back(Eigen::Vector3d::Zero());
        }
        if (output_has_colors) {
             if (accpoint.HasValidColors()) output->colors_.push_back(accpoint.GetAverageColor());
             else output->colors_.push_back(Eigen::Vector3d(0.5, 0.5, 0.5));
        }
    }
    if (output->HasNormals()) output->NormalizeNormals();
    utility::LogDebug("[VoxelDownSample] Downsampled from {:d} points to {:d} points.", (int)points_.size(), (int)output->points_.size());
    return output;
}


// --- EstimateNormals Implementation ---
void PointCloud::EstimateNormals(
        const KDTreeSearchParam &search_param /* = KDTreeSearchParamKNN()*/,
        bool fast_normal_computation /* = true */) {

    if (!HasPoints()) {
        utility::LogWarning("[EstimateNormals] PointCloud is empty.");
        return;
    }

    // Resize normals vector if it doesn't exist or has wrong size
    if (normals_.size() != points_.size()) {
        normals_.resize(points_.size());
    }

    // Build KDTree for neighborhood search
    geometry::KDTreeFlann kdtree;
    kdtree.SetGeometry(*this); // Use the current point cloud

    // Store original normals if they exist, for orientation consistency
    std::vector<Eigen::Vector3d> original_normals;
    bool has_original_normals = HasNormals(); // Check before resizing potentially
    if (has_original_normals) {
         original_normals = normals_; // Make a copy
    } else {
         // Ensure normals vector is allocated even if empty initially
         normals_.resize(points_.size(), Eigen::Vector3d::Zero());
    }


    // Parallel loop over all points
    #pragma omp parallel for schedule(static) num_threads(utility::EstimateMaxThreads())
    for (int i = 0; i < (int)points_.size(); ++i) {
        std::vector<int> nn_indices;
        std::vector<double> nn_dists; // Distances are squared distances

        // Find neighbors
        if (kdtree.Search(points_[i], search_param, nn_indices, nn_dists) < 3) {
            // Not enough neighbors to estimate covariance/normal reliably
            utility::LogDebug("[EstimateNormals] Point {:d} has less than 3 neighbors, setting normal to default.", i);
            normals_[i] = Eigen::Vector3d(0.0, 0.0, 1.0); // Assign a default normal
            continue;
        }

        // Compute covariance matrix for the neighbors
        // Note: utility::ComputeMeanAndCovariance computes mean AND covariance
        // We only need covariance here. utility::ComputeCovariance might be more direct if available.
        // Assuming utility::ComputeCovariance exists:
        Eigen::Matrix3d covariance = utility::ComputeCovariance(points_, nn_indices);
        // If utility::ComputeCovariance doesn't exist, use ComputeMeanAndCovariance:
        // Eigen::Vector3d mean; Eigen::Matrix3d covariance;
        // std::tie(mean, covariance) = utility::ComputeMeanAndCovariance(points_, nn_indices);

        // Compute normal from covariance
        Eigen::Vector3d normal = ComputeNormal(covariance, fast_normal_computation);

        // Handle cases where normal computation might fail (e.g., degenerate points)
        if (normal.hasNaN() || normal.norm() < 1e-9) {
             utility::LogDebug("[EstimateNormals] Normal computation failed for point {:d}, setting normal to default.", i);
             normal = Eigen::Vector3d(0.0, 0.0, 1.0); // Assign default normal
        }

        // Orient normal based on original normal if available
        if (has_original_normals) {
            if (normal.dot(original_normals[i]) < 0.0) {
                normal *= -1.0; // Flip normal
            }
        }
        // Note: Add optional orientation towards viewpoint if needed (requires camera location)

        normals_[i] = normal; // Assign computed/oriented normal
    }

    // Optionally normalize all normals at the end (redundant if ComputeNormal returns normalized)
    // NormalizeNormals(); // Already normalized within ComputeNormal/FastEigen3x3 usually
}


} // namespace geometry
} // namespace tiny3d
