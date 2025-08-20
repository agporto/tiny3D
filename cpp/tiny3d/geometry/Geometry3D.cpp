// ----------------------------------------------------------------------------
// -                      Tiny3D: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------
//
// Minimal version for basic functionality. Implements non-virtual helpers
// declared in the basic Geometry3D.h. Removed implementations for
// covariance transformations.
//

#include "tiny3d/geometry/Geometry3D.h"

#include <Eigen/Dense>
#include <numeric> // For std::accumulate
#include <vector>

#include "tiny3d/utility/Logging.h"
// Include headers for helper functions if they are not inline/part of Eigen.h
#include "tiny3d/utility/Eigen.h"

namespace tiny3d {
namespace geometry {

// --- Non-Virtual Method Implementations ---

// Default rotation implementation: calls the virtual Rotate with center.
Geometry3D& Geometry3D::Rotate(const Eigen::Matrix3d& R) {
    // Call the virtual Rotate function, using GetCenter() as the center point
    // Derived classes must implement GetCenter()
    return Rotate(R, GetCenter());
}

// --- Protected Helper Function Implementations ---

Eigen::Vector3d Geometry3D::ComputeMinBound(
        const std::vector<Eigen::Vector3d>& points) const {
    if (points.empty()) {
        // Return zero or potentially NaN/infinity depending on desired behavior
        return Eigen::Vector3d::Constant(
                std::numeric_limits<double>::quiet_NaN());
    }
    // Use std::accumulate for finding the minimum component-wise
    return std::accumulate(
            points.begin() + 1, points.end(), points[0], // Start with first point
            [](const Eigen::Vector3d& a, const Eigen::Vector3d& b) {
                // Use Eigen's array operations for component-wise min
                return a.array().min(b.array()).matrix();
            });
}

Eigen::Vector3d Geometry3D::ComputeMaxBound(
        const std::vector<Eigen::Vector3d>& points) const {
    if (points.empty()) {
         // Return zero or potentially NaN/infinity depending on desired behavior
        return Eigen::Vector3d::Constant(
                std::numeric_limits<double>::quiet_NaN());
    }
     // Use std::accumulate for finding the maximum component-wise
    return std::accumulate(
            points.begin() + 1, points.end(), points[0], // Start with first point
            [](const Eigen::Vector3d& a, const Eigen::Vector3d& b) {
                 // Use Eigen's array operations for component-wise max
                return a.array().max(b.array()).matrix();
            });
}

Eigen::Vector3d Geometry3D::ComputeCenter(
        const std::vector<Eigen::Vector3d>& points) const {
    Eigen::Vector3d center = Eigen::Vector3d::Zero();
    if (points.empty()) {
        return center; // Return zero vector for empty input
    }
    // Sum all points using std::accumulate
    center = std::accumulate(points.begin(), points.end(), center);
    // Divide by the number of points to get the average (center)
    center /= static_cast<double>(points.size());
    return center;
}

void Geometry3D::ResizeAndPaintUniformColor(
        std::vector<Eigen::Vector3d>& colors,
        const size_t size,
        const Eigen::Vector3d& color) const {
    colors.resize(size);
    // Clip color values to be within [0, 1] range
    Eigen::Vector3d clipped_color = color.array().max(0.0).min(1.0).matrix();
    if ((color - clipped_color).norm() > 1e-6) { // Check if clipping occurred
        utility::LogWarning(
                "[ResizeAndPaintUniformColor] Color value clipped to [0, 1].");
    }
    // Fill the vector with the clipped color
    std::fill(colors.begin(), colors.end(), clipped_color);
}

void Geometry3D::TransformPoints(const Eigen::Matrix4d& transformation,
                                 std::vector<Eigen::Vector3d>& points) const {
    // Apply transformation using homogeneous coordinates
    for (auto& point : points) {
        Eigen::Vector4d point_h(point(0), point(1), point(2), 1.0);
        Eigen::Vector4d new_point_h = transformation * point_h;
        // Avoid division by zero if w is close to zero (though unlikely for typical transformations)
        if (std::abs(new_point_h(3)) > 1e-9) {
            point = new_point_h.head<3>() / new_point_h(3);
        } else {
             // Handle perspective projection case where w -> 0 (point at infinity)
             // Assigning NaN or infinity might be appropriate, or logging a warning.
             point.setConstant(std::numeric_limits<double>::quiet_NaN());
             utility::LogWarning("[TransformPoints] Transformation resulted in near-zero w component.");
        }
    }
}

void Geometry3D::TransformNormals(const Eigen::Matrix4d& transformation,
                                  std::vector<Eigen::Vector3d>& normals) const {
    // Normals transform with the inverse transpose of the linear part (3x3)
    // Extract the 3x3 linear part
    Eigen::Matrix3d linear_part = transformation.block<3, 3>(0, 0);
    // Compute inverse transpose (handle potential non-invertibility)
    bool invertible;
    Eigen::Matrix3d normal_matrix;
    linear_part.computeInverseWithCheck(normal_matrix, invertible);
    if (invertible) {
         normal_matrix.transposeInPlace();
    } else {
         // Handle non-invertible transformation (e.g., scaling to zero)
         utility::LogWarning("[TransformNormals] Transformation matrix is not invertible. Using identity for normal transformation.");
         normal_matrix = Eigen::Matrix3d::Identity();
    }

    for (auto& normal : normals) {
        normal = normal_matrix * normal;
        // Renormalize after transformation
        normal.stableNormalize();
        // Optional: Check for NaN after normalization
        if (std::isnan(normal(0))) {
            normal.setZero(); // Or assign default normal
        }
    }
}

// --- Covariance Transformations Removed ---
// void Geometry3D::TransformCovariances(...) const { ... }

void Geometry3D::TranslatePoints(const Eigen::Vector3d& translation,
                                 std::vector<Eigen::Vector3d>& points,
                                 bool relative) const {
    Eigen::Vector3d transform_vector = translation;
    if (!relative) {
        // Move the center of the points to the target translation
        if (!points.empty()) {
            transform_vector = translation - ComputeCenter(points);
        } else {
            transform_vector = Eigen::Vector3d::Zero(); // Cannot compute center
        }
    }
    // Apply the calculated translation vector to all points
    for (auto& point : points) {
        point += transform_vector;
    }
}

void Geometry3D::ScalePoints(const double scale,
                             std::vector<Eigen::Vector3d>& points,
                             const Eigen::Vector3d& center) const {
    // Apply scaling relative to the specified center
    for (auto& point : points) {
        point = center + scale * (point - center);
    }
}

void Geometry3D::RotatePoints(const Eigen::Matrix3d& R,
                              std::vector<Eigen::Vector3d>& points,
                              const Eigen::Vector3d& center) const {
    // Apply rotation relative to the specified center
    for (auto& point : points) {
        point = center + R * (point - center);
    }
}

void Geometry3D::RotateNormals(const Eigen::Matrix3d& R,
                               std::vector<Eigen::Vector3d>& normals) const {
    // Rotate normals directly (assumes R is a pure rotation matrix)
    for (auto& normal : normals) {
        normal = R * normal;
        // No need to renormalize if R is orthogonal (pure rotation)
    }
}

// --- Covariance Rotations Removed ---
// void Geometry3D::RotateCovariances(...) const { ... }


// --- Static Rotation Matrix Helper Implementations ---

Eigen::Matrix3d Geometry3D::GetRotationMatrixFromXYZ(
        const Eigen::Vector3d& rotation) {
    // Ensure utility functions are available (either via include or definition)
    return utility::RotationMatrixX(rotation(0)) *
           utility::RotationMatrixY(rotation(1)) *
           utility::RotationMatrixZ(rotation(2));
}

Eigen::Matrix3d Geometry3D::GetRotationMatrixFromYZX(
        const Eigen::Vector3d& rotation) {
    return utility::RotationMatrixY(rotation(0)) *
           utility::RotationMatrixZ(rotation(1)) *
           utility::RotationMatrixX(rotation(2));
}

Eigen::Matrix3d Geometry3D::GetRotationMatrixFromZXY(
        const Eigen::Vector3d& rotation) {
    return utility::RotationMatrixZ(rotation(0)) *
           utility::RotationMatrixX(rotation(1)) *
           utility::RotationMatrixY(rotation(2));
}

Eigen::Matrix3d Geometry3D::GetRotationMatrixFromXZY(
        const Eigen::Vector3d& rotation) {
    return utility::RotationMatrixX(rotation(0)) *
           utility::RotationMatrixZ(rotation(1)) *
           utility::RotationMatrixY(rotation(2));
}

Eigen::Matrix3d Geometry3D::GetRotationMatrixFromZYX(
        const Eigen::Vector3d& rotation) {
    return utility::RotationMatrixZ(rotation(0)) *
           utility::RotationMatrixY(rotation(1)) *
           utility::RotationMatrixX(rotation(2));
}

Eigen::Matrix3d Geometry3D::GetRotationMatrixFromYXZ(
        const Eigen::Vector3d& rotation) {
    return utility::RotationMatrixY(rotation(0)) *
           utility::RotationMatrixX(rotation(1)) *
           utility::RotationMatrixZ(rotation(2));
}

Eigen::Matrix3d Geometry3D::GetRotationMatrixFromAxisAngle(
        const Eigen::Vector3d& rotation) {
    const double angle = rotation.norm();
    // Use a small epsilon to avoid division by zero for near-zero angles
    if (angle > 1e-12) {
        // Use Eigen::AngleAxisd for robust conversion
        return Eigen::AngleAxisd(angle, rotation / angle).toRotationMatrix();
    } else {
        // Return identity for zero rotation
        return Eigen::Matrix3d::Identity();
    }
}

Eigen::Matrix3d Geometry3D::GetRotationMatrixFromQuaternion(
        const Eigen::Vector4d& rotation) {
    // Ensure the quaternion is normalized
    Eigen::Quaterniond q(rotation(0), rotation(1), rotation(2), rotation(3));
    q.normalize();
    return q.toRotationMatrix();
}

} // namespace geometry
} // namespace tiny3d
