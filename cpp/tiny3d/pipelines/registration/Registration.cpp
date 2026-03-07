// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "tiny3d/pipelines/registration/Registration.h"

#include <algorithm>
#include <cmath>

#include "tiny3d/geometry/KDTreeFlann.h"
#include "tiny3d/geometry/PointCloud.h"
#include "tiny3d/pipelines/registration/Feature.h"
#include "tiny3d/utility/Logging.h"
#include "tiny3d/utility/Parallel.h"
#include "tiny3d/utility/Random.h"

namespace tiny3d {
namespace pipelines {
namespace registration {

static RegistrationResult GetRegistrationResultAndCorrespondencesTransformedSource(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        const geometry::KDTreeFlann &target_kdtree,
        double max_correspondence_distance,
        const Eigen::Matrix4d &transformation,
        bool with_correspondence_set = true) {
    RegistrationResult result(transformation);
    if (max_correspondence_distance <= 0.0) {
        return result;
    }

    const Eigen::Matrix3d R = transformation.block<3, 3>(0, 0);
    const Eigen::Vector3d t = transformation.block<3, 1>(0, 3);

    double error2 = 0.0;
    size_t correspondence_count = 0;
#pragma omp parallel
    {
        double error2_private = 0.0;
        int inlier_count_private = 0;
        CorrespondenceSet correspondence_set_private;
        std::vector<int> indices;
        std::vector<double> dists;
        indices.reserve(1);
        dists.reserve(1);
#pragma omp for nowait
        for (int i = 0; i < (int)source.points_.size(); i++) {
            const Eigen::Vector3d point = R * source.points_[i] + t;
            if (target_kdtree.SearchHybrid(point, max_correspondence_distance,
                                           1, indices, dists) > 0) {
                error2_private += dists[0];
                inlier_count_private++;
                if (with_correspondence_set) {
                    correspondence_set_private.emplace_back(i, indices[0]);
                }
            }
        }
#pragma omp critical(GetRegistrationResultAndCorrespondencesTransformedSource)
        {
            if (with_correspondence_set) {
                result.correspondence_set_.insert(
                        result.correspondence_set_.end(),
                        correspondence_set_private.begin(),
                        correspondence_set_private.end());
            }
            correspondence_count += static_cast<size_t>(inlier_count_private);
            error2 += error2_private;
        }
    }

    if (correspondence_count == 0) {
        result.fitness_ = 0.0;
        result.inlier_rmse_ = 0.0;
    } else {
        result.fitness_ = source.points_.empty()
                                  ? 0.0
                                  : static_cast<double>(correspondence_count) /
                                            static_cast<double>(source.points_.size());
        result.inlier_rmse_ =
                std::sqrt(error2 / static_cast<double>(correspondence_count));
    }
    return result;
}

static Eigen::Matrix4d ComputeTransformationPointToPointTransformedSource(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        const CorrespondenceSet &corres,
        const Eigen::Matrix4d &transformation,
        bool with_scaling) {
    if (corres.empty()) {
        return Eigen::Matrix4d::Identity();
    }

    const Eigen::Matrix3d linear = transformation.block<3, 3>(0, 0);
    const Eigen::Vector3d t = transformation.block<3, 1>(0, 3);
    const double inv_n = 1.0 / static_cast<double>(corres.size());

    Eigen::Vector3d mean_s = Eigen::Vector3d::Zero();
    Eigen::Vector3d mean_t = Eigen::Vector3d::Zero();
#pragma omp parallel
    {
        Eigen::Vector3d mean_s_private = Eigen::Vector3d::Zero();
        Eigen::Vector3d mean_t_private = Eigen::Vector3d::Zero();
#pragma omp for nowait
        for (int i = 0; i < static_cast<int>(corres.size()); ++i) {
            mean_s_private += linear * source.points_[corres[i][0]] + t;
            mean_t_private += target.points_[corres[i][1]];
        }
#pragma omp critical(ComputeTransformationPointToPointTransformedSourceMean)
        {
            mean_s += mean_s_private;
            mean_t += mean_t_private;
        }
    }
    mean_s *= inv_n;
    mean_t *= inv_n;

    Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
    double var_s = 0.0;
#pragma omp parallel
    {
        Eigen::Matrix3d cov_private = Eigen::Matrix3d::Zero();
        double var_s_private = 0.0;
#pragma omp for nowait
        for (int i = 0; i < static_cast<int>(corres.size()); ++i) {
            const Eigen::Vector3d ds =
                    linear * source.points_[corres[i][0]] + t - mean_s;
            const Eigen::Vector3d dt = target.points_[corres[i][1]] - mean_t;
            cov_private.noalias() += dt * ds.transpose();
            var_s_private += ds.squaredNorm();
        }
#pragma omp critical(ComputeTransformationPointToPointTransformedSourceCov)
        {
            cov += cov_private;
            var_s += var_s_private;
        }
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
    if (with_scaling && var_s > 0.0) {
        const Eigen::Vector3d sigma = svd.singularValues();
        scale = sigma.dot(diag) / var_s;
    }

    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<3, 3>(0, 0) = scale * R;
    T.block<3, 1>(0, 3) = mean_t - scale * R * mean_s;
    return T;
}

static Eigen::Matrix4d ComputeTransformationPointToPlaneTransformedSource(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        const CorrespondenceSet &corres,
        const Eigen::Matrix4d &transformation) {
    if (corres.empty() || !target.HasNormals()) {
        return Eigen::Matrix4d::Identity();
    }

    const Eigen::Matrix3d linear = transformation.block<3, 3>(0, 0);
    const Eigen::Vector3d t = transformation.block<3, 1>(0, 3);
    Eigen::Matrix6d JTJ = Eigen::Matrix6d::Zero();
    Eigen::Vector6d JTr = Eigen::Vector6d::Zero();

#pragma omp parallel
    {
        Eigen::Matrix6d JTJ_private = Eigen::Matrix6d::Zero();
        Eigen::Vector6d JTr_private = Eigen::Vector6d::Zero();
        Eigen::Vector6d J_r = Eigen::Vector6d::Zero();
#pragma omp for nowait
        for (int i = 0; i < static_cast<int>(corres.size()); ++i) {
            const Eigen::Vector3d vs =
                    linear * source.points_[corres[i][0]] + t;
            const Eigen::Vector3d &vt = target.points_[corres[i][1]];
            const Eigen::Vector3d &nt = target.normals_[corres[i][1]];
            const double r = (vs - vt).dot(nt);
            J_r.block<3, 1>(0, 0) = vs.cross(nt);
            J_r.block<3, 1>(3, 0) = nt;
            JTJ_private.noalias() += J_r * J_r.transpose();
            JTr_private.noalias() += J_r * r;
        }
#pragma omp critical(ComputeTransformationPointToPlaneTransformedSource)
        {
            JTJ += JTJ_private;
            JTr += JTr_private;
        }
    }

    bool is_success;
    Eigen::Matrix4d extrinsic;
    std::tie(is_success, extrinsic) =
            utility::SolveJacobianSystemAndObtainExtrinsicMatrix(JTJ, JTr);
    return is_success ? extrinsic : Eigen::Matrix4d::Identity();
}

static Eigen::Matrix4d ComputeTransformationTransformedSource(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        const CorrespondenceSet &corres,
        const Eigen::Matrix4d &transformation,
        const TransformationEstimation &estimation) {
    if (const auto *point_to_point =
                dynamic_cast<const TransformationEstimationPointToPoint *>(
                        &estimation)) {
        return ComputeTransformationPointToPointTransformedSource(
                source, target, corres, transformation,
                point_to_point->with_scaling_);
    }
    if (dynamic_cast<const TransformationEstimationPointToPlane *>(&estimation)) {
        return ComputeTransformationPointToPlaneTransformedSource(
                source, target, corres, transformation);
    }

    geometry::PointCloud transformed_source(source);
    if (!transformation.isIdentity()) {
        transformed_source.Transform(transformation);
    }
    return estimation.ComputeTransformation(transformed_source, target, corres);
}

static RegistrationResult GetRegistrationResultTransformedSourceSampled(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        const geometry::KDTreeFlann &target_kdtree,
        double max_correspondence_distance,
        const Eigen::Matrix4d &transformation,
        int max_samples) {
    RegistrationResult result(transformation);
    if (max_correspondence_distance <= 0.0 || source.points_.empty()) {
        return result;
    }
    if (max_samples <= 0) {
        max_samples = 1;
    }

    const int n_source = static_cast<int>(source.points_.size());
    const int stride = std::max(1, n_source / max_samples);
    const Eigen::Matrix3d R = transformation.block<3, 3>(0, 0);
    const Eigen::Vector3d t = transformation.block<3, 1>(0, 3);

    double error2 = 0.0;
    int inlier_count = 0;
    int sampled_count = 0;

#pragma omp parallel
    {
        double error2_private = 0.0;
        int inlier_private = 0;
        int sampled_private = 0;
        std::vector<int> indices;
        std::vector<double> dists;
        indices.reserve(1);
        dists.reserve(1);
#pragma omp for nowait
        for (int i = 0; i < n_source; i += stride) {
            sampled_private++;
            const Eigen::Vector3d point = R * source.points_[i] + t;
            if (target_kdtree.SearchHybrid(point, max_correspondence_distance,
                                           1, indices, dists) > 0) {
                error2_private += dists[0];
                inlier_private++;
            }
        }
#pragma omp critical(GetRegistrationResultTransformedSourceSampled)
        {
            error2 += error2_private;
            inlier_count += inlier_private;
            sampled_count += sampled_private;
        }
    }

    if (inlier_count > 0) {
        result.fitness_ = sampled_count > 0
                                  ? static_cast<double>(inlier_count) /
                                            static_cast<double>(sampled_count)
                                  : 0.0;
        result.inlier_rmse_ =
                std::sqrt(error2 / static_cast<double>(inlier_count));
    }
    return result;
}

static double EvaluateInlierCorrespondenceRatio(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        const CorrespondenceSet &corres,
        double max_correspondence_distance,
        const Eigen::Matrix4d &transformation) {
    if (corres.empty()) {
        return 0.0;
    }

    int inlier_corres = 0;
    int sampled_corres = 0;
    const int max_samples = 5000;
    const int stride =
            std::max(1, static_cast<int>(corres.size()) / max_samples);
    const Eigen::Matrix3d R = transformation.block<3, 3>(0, 0);
    const Eigen::Vector3d t = transformation.block<3, 1>(0, 3);
    double max_dis2 = max_correspondence_distance * max_correspondence_distance;
    for (int i = 0; i < static_cast<int>(corres.size()); i += stride) {
        const auto &c = corres[i];
        sampled_corres++;
        const Eigen::Vector3d transformed = R * source.points_[c[0]] + t;
        const double dis2 =
                (transformed - target.points_[c[1]]).squaredNorm();
        if (dis2 < max_dis2) {
            inlier_corres++;
        }
    }

    return sampled_corres > 0 ? double(inlier_corres) / double(sampled_corres)
                              : 0.0;
}

RegistrationResult EvaluateRegistration(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        double max_correspondence_distance,
        const Eigen::Matrix4d
                &transformation /* = Eigen::Matrix4d::Identity()*/) {
    geometry::KDTreeFlann kdtree;
    kdtree.SetGeometry(target);
    return GetRegistrationResultAndCorrespondencesTransformedSource(
            source, target, kdtree, max_correspondence_distance, transformation);
}

RegistrationResult RegistrationICP(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        double max_correspondence_distance,
        const Eigen::Matrix4d &init /* = Eigen::Matrix4d::Identity()*/,
        const TransformationEstimation &estimation
        /* = TransformationEstimationPointToPoint(false)*/,
        const ICPConvergenceCriteria
                &criteria /* = ICPConvergenceCriteria()*/) {
    if (max_correspondence_distance <= 0.0) {
        utility::LogError("Invalid max_correspondence_distance.");
        return RegistrationResult(init);
    }
    if (source.IsEmpty() || target.IsEmpty()) {
        utility::LogWarning("RegistrationICP skipped on empty point cloud.");
        return RegistrationResult(init);
    }

    auto [source_initialized_c, target_initialized_c] =
            estimation.InitializePointCloudsForTransformation(
                    source, target, max_correspondence_distance);

    Eigen::Matrix4d transformation = init;
    geometry::KDTreeFlann kdtree;
    const geometry::PointCloud &target_initialized = *target_initialized_c;
    kdtree.SetGeometry(target_initialized);
    RegistrationResult result;
    result = GetRegistrationResultAndCorrespondencesTransformedSource(
            *source_initialized_c, target_initialized, kdtree,
            max_correspondence_distance, transformation);
    for (int i = 0; i < criteria.max_iteration_; i++) {
        utility::LogDebug("ICP Iteration #{:d}: Fitness {:.4f}, RMSE {:.4f}", i,
                          result.fitness_, result.inlier_rmse_);
        const Eigen::Matrix4d update = ComputeTransformationTransformedSource(
                *source_initialized_c, target_initialized,
                result.correspondence_set_, transformation, estimation);
        if (!update.allFinite()) {
            utility::LogWarning(
                    "RegistrationICP encountered non-finite update at iteration {}.",
                    i);
            break;
        }
        transformation = update * transformation;
        RegistrationResult backup = result;
        result = GetRegistrationResultAndCorrespondencesTransformedSource(
                *source_initialized_c, target_initialized, kdtree,
                max_correspondence_distance, transformation);
        if (std::abs(backup.fitness_ - result.fitness_) <
                    criteria.relative_fitness_ &&
            std::abs(backup.inlier_rmse_ - result.inlier_rmse_) <
                    criteria.relative_rmse_) {
            break;
        }
    }
    return result;
}

RegistrationResult RegistrationRANSACBasedOnCorrespondence(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        const CorrespondenceSet &corres,
        double max_correspondence_distance,
        const TransformationEstimation &estimation
        /* = TransformationEstimationPointToPoint(false)*/,
        int ransac_n /* = 3*/,
        const std::vector<std::reference_wrapper<const CorrespondenceChecker>>
                &checkers /* = {}*/,
        const RANSACConvergenceCriteria &criteria
        /* = RANSACConvergenceCriteria()*/) {
    if (ransac_n < 3 || (int)corres.size() < ransac_n ||
        max_correspondence_distance <= 0.0) {
        return RegistrationResult();
    }
    if (source.IsEmpty() || target.IsEmpty()) {
        return RegistrationResult();
    }

    RegistrationResult best_result;
    geometry::KDTreeFlann kdtree(target);
    int est_k_global = criteria.max_iteration_;
    int total_validation = 0;

#pragma omp parallel
    {
        CorrespondenceSet ransac_corres(ransac_n);
        RegistrationResult best_result_local;
        int est_k_local = criteria.max_iteration_;
        utility::random::UniformIntGenerator<int> rand_gen(0,
                                                           corres.size() - 1);

#pragma omp for nowait
        for (int itr = 0; itr < criteria.max_iteration_; itr++) {
            if (itr < est_k_global) {
                int sampled = 0;
                int attempts = 0;
                const int max_attempts = std::max(32 * ransac_n, 128);
                while (sampled < ransac_n) {
                    const auto &candidate = corres[rand_gen()];
                    bool duplicated = false;
                    for (int j = 0; j < sampled; ++j) {
                        if (ransac_corres[j] == candidate) {
                            duplicated = true;
                            break;
                        }
                    }
                    if (!duplicated) {
                        ransac_corres[sampled++] = candidate;
                    }
                    if (++attempts > max_attempts) {
                        break;
                    }
                }
                if (sampled < ransac_n) {
                    continue;
                }

                Eigen::Matrix4d transformation =
                        estimation.ComputeTransformation(source, target,
                                                         ransac_corres);
                if (!transformation.allFinite()) {
                    continue;
                }

                // Check transformation: inexpensive
                bool check = true;
                for (const auto &checker : checkers) {
                    if (!checker.get().Check(source, target, ransac_corres,
                                             transformation)) {
                        check = false;
                        break;
                    }
                }
                if (!check) continue;

                // Cheap validation pass; skip obviously weak candidates.
                const auto sampled_result =
                        GetRegistrationResultTransformedSourceSampled(
                                source, target, kdtree,
                                max_correspondence_distance, transformation,
                                2048);
                if (best_result_local.fitness_ > 0.0 &&
                    sampled_result.fitness_ + 0.02 <
                            best_result_local.fitness_) {
                    continue;
                }
                if (sampled_result.fitness_ <= 0.0) {
                    continue;
                }

                // Expensive validation
                auto result =
                        GetRegistrationResultAndCorrespondencesTransformedSource(
                                source, target, kdtree,
                                max_correspondence_distance, transformation,
                                false);

                if (result.IsBetterRANSACThan(best_result_local)) {
                    result = GetRegistrationResultAndCorrespondencesTransformedSource(
                            source, target, kdtree,
                            max_correspondence_distance, transformation, true);
                } else {
                    continue;
                }

                if (result.IsBetterRANSACThan(best_result_local)) {
                    best_result_local = result;

                    double corres_inlier_ratio =
                            EvaluateInlierCorrespondenceRatio(
                                    source, target, corres,
                                    max_correspondence_distance, transformation);

                    // Update exit condition if necessary
                    double est_k_local_d = static_cast<double>(est_k_local);
                    const double inlier_prob =
                            std::pow(corres_inlier_ratio, ransac_n);
                    if (inlier_prob >= 1.0) {
                        est_k_local_d = 1.0;
                    } else if (inlier_prob > 0.0) {
                        est_k_local_d =
                                std::log(1.0 - criteria.confidence_) /
                                std::log(1.0 - inlier_prob);
                        if (est_k_local_d < 0.0 || !std::isfinite(est_k_local_d)) {
                            est_k_local_d = static_cast<double>(est_k_local);
                        }
                    }
                    est_k_local =
                            est_k_local_d < est_k_global
                                    ? static_cast<int>(std::ceil(est_k_local_d))
                                    : est_k_local;
                    utility::LogDebug(
                            "Thread {:06d}: registration fitness={:.3f}, "
                            "corres inlier ratio={:.3f}, "
                            "Est. max k = {}",
                            itr, result.fitness_, corres_inlier_ratio,
                            est_k_local_d);
                }
#pragma omp critical
                {
                    total_validation += 1;
                    if (est_k_local < est_k_global) {
                        est_k_global = est_k_local;
                    }
                }
            }  // if
        }      // for loop

#pragma omp critical(RegistrationRANSACBasedOnCorrespondence)
        {
            if (best_result_local.IsBetterRANSACThan(best_result)) {
                best_result = best_result_local;
            }
        }
    }
    utility::LogDebug(
            "RANSAC exits after {:d} validations. Best inlier ratio {:e}, "
            "RMSE {:e}",
            total_validation, best_result.fitness_, best_result.inlier_rmse_);
    return best_result;
}

RegistrationResult RegistrationRANSACBasedOnFeatureMatching(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        const Feature &source_features,
        const Feature &target_features,
        bool mutual_filter,
        double max_correspondence_distance,
        const TransformationEstimation
                &estimation /* = TransformationEstimationPointToPoint(false)*/,
        int ransac_n /* = 3*/,
        const std::vector<std::reference_wrapper<const CorrespondenceChecker>>
                &checkers /* = {}*/,
        const RANSACConvergenceCriteria
                &criteria /* = RANSACConvergenceCriteria()*/) {
    if (ransac_n < 3 || max_correspondence_distance <= 0.0) {
        return RegistrationResult();
    }

    CorrespondenceSet corres = CorrespondencesFromFeatures(
            source_features, target_features, mutual_filter);

    return RegistrationRANSACBasedOnCorrespondence(
            source, target, corres, max_correspondence_distance, estimation,
            ransac_n, checkers, criteria);
}

Eigen::Matrix6d GetInformationMatrixFromPointClouds(
        const geometry::PointCloud &source,
        const geometry::PointCloud &target,
        double max_correspondence_distance,
        const Eigen::Matrix4d &transformation) {
    geometry::KDTreeFlann target_kdtree(target);
    RegistrationResult result =
            GetRegistrationResultAndCorrespondencesTransformedSource(
                    source, target, target_kdtree,
                    max_correspondence_distance, transformation);

    // write q^*
    // see http://redwood-data.org/indoor/registration.html
    // note: I comes first in this implementation
    Eigen::Matrix6d GTG = Eigen::Matrix6d::Zero();
#pragma omp parallel
    {
        Eigen::Matrix6d GTG_private = Eigen::Matrix6d::Zero();
        Eigen::Vector6d G_r_private = Eigen::Vector6d::Zero();
#pragma omp for nowait
        for (int c = 0; c < int(result.correspondence_set_.size()); c++) {
            int t = result.correspondence_set_[c](1);
            double x = target.points_[t](0);
            double y = target.points_[t](1);
            double z = target.points_[t](2);
            G_r_private.setZero();
            G_r_private(1) = z;
            G_r_private(2) = -y;
            G_r_private(3) = 1.0;
            GTG_private.noalias() += G_r_private * G_r_private.transpose();
            G_r_private.setZero();
            G_r_private(0) = -z;
            G_r_private(2) = x;
            G_r_private(4) = 1.0;
            GTG_private.noalias() += G_r_private * G_r_private.transpose();
            G_r_private.setZero();
            G_r_private(0) = y;
            G_r_private(1) = -x;
            G_r_private(5) = 1.0;
            GTG_private.noalias() += G_r_private * G_r_private.transpose();
        }
#pragma omp critical(GetInformationMatrixFromPointClouds)
        { GTG += GTG_private; }
    }
    return GTG;
}

}  // namespace registration
}  // namespace pipelines
}  // namespace tiny3d
