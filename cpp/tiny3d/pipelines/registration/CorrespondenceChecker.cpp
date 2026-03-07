// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "tiny3d/pipelines/registration/CorrespondenceChecker.h"

#include <Eigen/Dense>

#include "tiny3d/geometry/PointCloud.h"
#include "tiny3d/utility/Logging.h"

namespace tiny3d {
namespace pipelines {
namespace registration {

bool CorrespondenceCheckerBasedOnEdgeLength::Check(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        const CorrespondenceSet &corres,
        const Eigen::Matrix4d & /*transformation*/) const {
    const double similarity_threshold2 =
            similarity_threshold_ * similarity_threshold_;
    for (size_t i = 0; i < corres.size(); i++) {
        for (size_t j = i + 1; j < corres.size(); j++) {
            // check edge ij
            const double dis_source2 =
                    (source.points_[corres[i](0)] - source.points_[corres[j](0)])
                            .squaredNorm();
            const double dis_target2 =
                    (target.points_[corres[i](1)] - target.points_[corres[j](1)])
                            .squaredNorm();
            if (dis_source2 < dis_target2 * similarity_threshold2 ||
                dis_target2 < dis_source2 * similarity_threshold2) {
                return false;
            }
        }
    }
    return true;
}

bool CorrespondenceCheckerBasedOnDistance::Check(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        const CorrespondenceSet &corres,
        const Eigen::Matrix4d &transformation) const {
    const Eigen::Matrix3d R = transformation.block<3, 3>(0, 0);
    const Eigen::Vector3d t = transformation.block<3, 1>(0, 3);
    const double distance_threshold2 = distance_threshold_ * distance_threshold_;
    for (const auto &c : corres) {
        const Eigen::Vector3d pt_trans = R * source.points_[c(0)] + t;
        if ((target.points_[c(1)] - pt_trans).squaredNorm() >
            distance_threshold2) {
            return false;
        }
    }
    return true;
}

bool CorrespondenceCheckerBasedOnNormal::Check(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        const CorrespondenceSet &corres,
        const Eigen::Matrix4d &transformation) const {
    if (!source.HasNormals() || !target.HasNormals()) {
        utility::LogWarning(
                "[CorrespondenceCheckerBasedOnNormal::Check] Pointcloud has no "
                "normals.");
        return true;
    }
    const Eigen::Matrix3d R = transformation.block<3, 3>(0, 0);
    const double cos_normal_angle_threshold = std::cos(normal_angle_threshold_);
    for (const auto &c : corres) {
        const Eigen::Vector3d normal_trans = R * source.normals_[c(0)];
        if (target.normals_[c(1)].dot(normal_trans) <
            cos_normal_angle_threshold) {
            return false;
        }
    }
    return true;
}

}  // namespace registration
}  // namespace pipelines
}  // namespace tiny3d
