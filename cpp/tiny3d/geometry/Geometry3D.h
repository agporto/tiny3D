// ----------------------------------------------------------------------------
// -                      Tiny3D: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------
//
// Minimal version for basic functionality. Removed oriented bounding boxes
// and covariance transformation helpers.
//

#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry> // For AngleAxisd, Quaterniond
#include <vector>
#include <memory> // For std::shared_ptr in derived classes potentially

#include "tiny3d/geometry/Geometry.h"
#include "tiny3d/utility/Eigen.h" // For utility::RotationMatrix* helpers

namespace tiny3d {

// Forward declare utility functions if needed and not included via Eigen.h
namespace utility {
Eigen::Matrix3d RotationMatrixX(double angle);
Eigen::Matrix3d RotationMatrixY(double angle);
Eigen::Matrix3d RotationMatrixZ(double angle);
} // namespace utility

namespace geometry {

class AxisAlignedBoundingBox;
// class OrientedBoundingBox; // Removed

/// \class Geometry3D
///
/// \brief The base geometry class for 3D geometries.
///
/// Defines the basic interface for 3D geometries including bounds,
/// transformations, and common helper functions.
class Geometry3D : public Geometry {
public:
    ~Geometry3D() override {}

protected:
    /// \brief Protected Constructor.
    ///
    /// \param type type of object based on GeometryType.
    Geometry3D(GeometryType type) : Geometry(type, 3) {}

public:
    // --- Pure Virtual Functions (Interface for Derived Classes) ---
    Geometry3D& Clear() override = 0;
    bool IsEmpty() const override = 0;
    /// Returns min bounds for geometry coordinates.
    virtual Eigen::Vector3d GetMinBound() const = 0;
    /// Returns max bounds for geometry coordinates.
    virtual Eigen::Vector3d GetMaxBound() const = 0;
    /// Returns the center of the geometry coordinates.
    virtual Eigen::Vector3d GetCenter() const = 0;

    /// Creates the axis-aligned bounding box around the points/vertices.
    virtual AxisAlignedBoundingBox GetAxisAlignedBoundingBox() const = 0;

    // --- Oriented Bounding Boxes Removed ---
    // virtual OrientedBoundingBox GetOrientedBoundingBox(
    //         bool robust = false) const = 0;
    // virtual OrientedBoundingBox GetMinimalOrientedBoundingBox(
    //         bool robust = false) const = 0;

    /// \brief Apply transformation (4x4 matrix) to the geometry coordinates.
    virtual Geometry3D& Transform(const Eigen::Matrix4d& transformation) = 0;

    /// \brief Apply translation to the geometry coordinates.
    virtual Geometry3D& Translate(const Eigen::Vector3d& translation,
                                  bool relative = true) = 0;
    /// \brief Apply scaling to the geometry coordinates.
    virtual Geometry3D& Scale(const double scale,
                              const Eigen::Vector3d& center) = 0;

    /// \brief Apply rotation to the geometry coordinates and normals.
    virtual Geometry3D& Rotate(const Eigen::Matrix3d& R,
                               const Eigen::Vector3d& center) = 0;

    /// \brief Apply rotation to the geometry coordinates and normals around the center.
    virtual Geometry3D& Rotate(const Eigen::Matrix3d& R); // Non-virtual overload

    // --- Static Rotation Matrix Helpers (General Utility) ---
    static Eigen::Matrix3d GetRotationMatrixFromXYZ(
            const Eigen::Vector3d& rotation);
    static Eigen::Matrix3d GetRotationMatrixFromYZX(
            const Eigen::Vector3d& rotation);
    static Eigen::Matrix3d GetRotationMatrixFromZXY(
            const Eigen::Vector3d& rotation);
    static Eigen::Matrix3d GetRotationMatrixFromXZY(
            const Eigen::Vector3d& rotation);
    static Eigen::Matrix3d GetRotationMatrixFromZYX(
            const Eigen::Vector3d& rotation);
    static Eigen::Matrix3d GetRotationMatrixFromYXZ(
            const Eigen::Vector3d& rotation);
    static Eigen::Matrix3d GetRotationMatrixFromAxisAngle(
            const Eigen::Vector3d& rotation);
    static Eigen::Matrix3d GetRotationMatrixFromQuaternion(
            const Eigen::Vector4d& rotation);

protected:
    // --- Protected Helper Functions (Used by Derived Classes) ---

    /// Compute min bound of a list points.
    Eigen::Vector3d ComputeMinBound(
            const std::vector<Eigen::Vector3d>& points) const;
    /// Compute max bound of a list points.
    Eigen::Vector3d ComputeMaxBound(
            const std::vector<Eigen::Vector3d>& points) const;
    /// Computer center of a list of points.
    Eigen::Vector3d ComputeCenter(
            const std::vector<Eigen::Vector3d>& points) const;

    /// \brief Resizes the colors vector and paints a uniform color.
    void ResizeAndPaintUniformColor(std::vector<Eigen::Vector3d>& colors,
                                    const size_t size,
                                    const Eigen::Vector3d& color) const;

    /// \brief Transforms all points with the transformation matrix.
    void TransformPoints(const Eigen::Matrix4d& transformation,
                         std::vector<Eigen::Vector3d>& points) const;

    /// \brief Transforms the normals with the transformation matrix.
    void TransformNormals(const Eigen::Matrix4d& transformation,
                          std::vector<Eigen::Vector3d>& normals) const;

    // --- Covariance Transformations Removed ---
    // void TransformCovariances(...) const;

    /// \brief Apply translation to points.
    void TranslatePoints(const Eigen::Vector3d& translation,
                         std::vector<Eigen::Vector3d>& points,
                         bool relative) const;

    /// \brief Scale points.
    void ScalePoints(const double scale,
                     std::vector<Eigen::Vector3d>& points,
                     const Eigen::Vector3d& center) const;

    /// \brief Rotate points.
    void RotatePoints(const Eigen::Matrix3d& R,
                      std::vector<Eigen::Vector3d>& points,
                      const Eigen::Vector3d& center) const;

    /// \brief Rotate normals.
    void RotateNormals(const Eigen::Matrix3d& R,
                       std::vector<Eigen::Vector3d>& normals) const;

    // --- Covariance Rotations Removed ---
    // void RotateCovariances(...) const;
};

} // namespace geometry
} // namespace tiny3d
