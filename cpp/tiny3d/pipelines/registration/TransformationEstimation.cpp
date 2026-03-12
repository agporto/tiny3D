// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "tiny3d/pipelines/registration/TransformationEstimation.h"

#include <Eigen/Geometry>

#include "tiny3d/geometry/PointCloud.h"
#include "tiny3d/utility/Eigen.h"
#include "tiny3d/utility/Logging.h"

namespace tiny3d {
namespace pipelines {
namespace registration {

double TransformationEstimationPointToPoint::ComputeRMSE(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        const CorrespondenceSet &corres) const {
    if (corres.empty()) return 0.0;
    double err = 0.0;
    for (const auto &c : corres) {
        err += (source.points_[c[0]] - target.points_[c[1]]).squaredNorm();
    }
    return std::sqrt(err / (double)corres.size());
}

Eigen::Matrix4d TransformationEstimationPointToPoint::ComputeTransformation(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        const CorrespondenceSet &corres) const {
    if (corres.empty()) return Eigen::Matrix4d::Identity();

    const double inv_n = 1.0 / static_cast<double>(corres.size());
    Eigen::Vector3d mean_s = Eigen::Vector3d::Zero();
    Eigen::Vector3d mean_t = Eigen::Vector3d::Zero();
    for (const auto &c : corres) {
        mean_s += source.points_[c[0]];
        mean_t += target.points_[c[1]];
    }
    mean_s *= inv_n;
    mean_t *= inv_n;

    Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
    double var_s = 0.0;
    for (const auto &c : corres) {
        const Eigen::Vector3d ds = source.points_[c[0]] - mean_s;
        const Eigen::Vector3d dt = target.points_[c[1]] - mean_t;
        cov.noalias() += dt * ds.transpose();
        var_s += ds.squaredNorm();
    }
    cov *= inv_n;
    var_s *= inv_n;

    const Eigen::JacobiSVD<Eigen::Matrix3d> svd(
            cov, Eigen::ComputeFullU | Eigen::ComputeFullV);
    const Eigen::Matrix3d U = svd.matrixU();
    const Eigen::Matrix3d V = svd.matrixV();
    if (!U.allFinite() || !V.allFinite()) {
        utility::LogDebug(
                "PointToPoint SVD failed, falling back to identity update.");
        return Eigen::Matrix4d::Identity();
    }

    Eigen::Vector3d diag = Eigen::Vector3d::Ones();
    if ((U * V.transpose()).determinant() < 0.0) {
        diag(2) = -1.0;
    }
    const Eigen::Matrix3d R = U * diag.asDiagonal() * V.transpose();

    double scale = 1.0;
    if (with_scaling_ && var_s > 0.0) {
        const Eigen::Vector3d sigma = svd.singularValues();
        scale = sigma.dot(diag) / var_s;
    }

    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<3, 3>(0, 0) = scale * R;
    T.block<3, 1>(0, 3) = mean_t - scale * R * mean_s;
    return T;
}

std::tuple<std::shared_ptr<const geometry::PointCloud>,
           std::shared_ptr<const geometry::PointCloud>>
TransformationEstimationPointToPoint::InitializePointCloudsForTransformation(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        double max_correspondence_distance) const {
    std::shared_ptr<const geometry::PointCloud> source_initialized_c(
            &source, [](const geometry::PointCloud *) {});
    std::shared_ptr<const geometry::PointCloud> target_initialized_c(
            &target, [](const geometry::PointCloud *) {});
    if (!source_initialized_c || !target_initialized_c) {
        utility::LogError(
                "Internal error: InitializePointCloudsForTransformation "
                "returns "
                "nullptr.");
    }
    return std::make_tuple(source_initialized_c, target_initialized_c);
}

double TransformationEstimationPointToPlane::ComputeRMSE(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        const CorrespondenceSet &corres) const {
    if (corres.empty() || !target.HasNormals()) return 0.0;
    double err = 0.0, r;
    for (const auto &c : corres) {
        r = (source.points_[c[0]] - target.points_[c[1]])
                    .dot(target.normals_[c[1]]);
        err += r * r;
    }
    return std::sqrt(err / (double)corres.size());
}

Eigen::Matrix4d TransformationEstimationPointToPlane::ComputeTransformation(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        const CorrespondenceSet &corres) const {
    if (corres.empty() || !target.HasNormals())
        return Eigen::Matrix4d::Identity();

    auto compute_jacobian_and_residual = [&](int i, Eigen::Vector6d &J_r,
                                             double &r, double &w) {
        const Eigen::Vector3d &vs = source.points_[corres[i][0]];
        const Eigen::Vector3d &vt = target.points_[corres[i][1]];
        const Eigen::Vector3d &nt = target.normals_[corres[i][1]];
        r = (vs - vt).dot(nt);
        w = 1.0;
        J_r.block<3, 1>(0, 0) = vs.cross(nt);
        J_r.block<3, 1>(3, 0) = nt;
    };

    Eigen::Matrix6d JTJ;
    Eigen::Vector6d JTr;
    double r2;
    std::tie(JTJ, JTr, r2) =
            utility::ComputeJTJandJTr<Eigen::Matrix6d, Eigen::Vector6d>(
                    compute_jacobian_and_residual, (int)corres.size());

    bool is_success;
    Eigen::Matrix4d extrinsic;
    std::tie(is_success, extrinsic) =
            utility::SolveJacobianSystemAndObtainExtrinsicMatrix(JTJ, JTr);

    return is_success ? extrinsic : Eigen::Matrix4d::Identity();
}

std::tuple<std::shared_ptr<const geometry::PointCloud>,
           std::shared_ptr<const geometry::PointCloud>>
TransformationEstimationPointToPlane::InitializePointCloudsForTransformation(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        double max_correspondence_distance) const {
    if (!target.HasNormals()) {
        utility::LogError(
                "PointToPlaneICP requires target pointcloud to have normals.");
    }
    std::shared_ptr<const geometry::PointCloud> source_initialized_c(
            &source, [](const geometry::PointCloud *) {});
    std::shared_ptr<const geometry::PointCloud> target_initialized_c(
            &target, [](const geometry::PointCloud *) {});
    if (!source_initialized_c || !target_initialized_c) {
        utility::LogError(
                "Internal error: InitializePointCloudsForTransformation "
                "returns "
                "nullptr.");
    }
    return std::make_tuple(source_initialized_c, target_initialized_c);
}

}  // namespace registration
}  // namespace pipelines
}  // namespace tiny3d
