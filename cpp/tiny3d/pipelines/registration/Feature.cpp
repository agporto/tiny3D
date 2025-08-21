// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "tiny3d/pipelines/registration/Feature.h"

#include <Eigen/Dense>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "tiny3d/geometry/KDTreeFlann.h"
#include "tiny3d/geometry/PointCloud.h"
#include "tiny3d/utility/Logging.h"
#include "tiny3d/utility/Parallel.h"

namespace tiny3d {
namespace pipelines {
namespace registration {

std::shared_ptr<Feature> Feature::SelectByIndex(
        const std::vector<size_t> &indices, bool invert /* = false */) const {
    auto output = std::make_shared<Feature>();

    std::vector<bool> mask = std::vector<bool>(data_.cols(), invert);
    size_t n_output_features = 0;
    for (size_t i : indices) {
        if (i < mask.size()) {
            if (mask[i] == invert) {
                mask[i] = !invert;
                n_output_features++;
            }
        } else {
            utility::LogWarning(
                    "[SelectByIndex] contains index {} that is "
                    "not within the bounds",
                    (int)i);
        }
    }
    if (invert) {
        n_output_features = data_.cols() - n_output_features;
    }

    output->Resize(data_.rows(), n_output_features);

    for (size_t i = 0, current_col_feature = 0;
         i < static_cast<size_t>(data_.cols()); i++) {
        if (mask[i]) {
            output->data_.col(current_col_feature++) = data_.col(i);
        }
    }

    utility::LogDebug(
            "[SelectByIndex] Feature group down sampled from {:d} features to "
            "{:d} features.",
            (int)data_.cols(), (int)output->data_.cols());

    return output;
}

static Eigen::Vector4d ComputePairFeatures(const Eigen::Vector3d &p1,
                                           const Eigen::Vector3d &n1,
                                           const Eigen::Vector3d &p2,
                                           const Eigen::Vector3d &n2) {
    Eigen::Vector4d result;
    Eigen::Vector3d dp2p1 = p2 - p1;
    result(3) = dp2p1.norm();
    if (result(3) == 0.0) {
        return Eigen::Vector4d::Zero();
    }
    auto n1_copy = n1;
    auto n2_copy = n2;
    double angle1 = n1_copy.dot(dp2p1) / result(3);
    double angle2 = n2_copy.dot(dp2p1) / result(3);
    if (acos(fabs(angle1)) > acos(fabs(angle2))) {
        n1_copy = n2;
        n2_copy = n1;
        dp2p1 *= -1.0;
        result(2) = -angle2;
    } else {
        result(2) = angle1;
    }
    auto v = dp2p1.cross(n1_copy);
    double v_norm = v.norm();
    if (v_norm == 0.0) {
        return Eigen::Vector4d::Zero();
    }
    v /= v_norm;
    auto w = n1_copy.cross(v);
    result(1) = v.dot(n2_copy);
    result(0) = atan2(w.dot(n2_copy), n1_copy.dot(n2_copy));
    return result;
}

static std::shared_ptr<Feature> ComputeSPFHFeature(
        const geometry::PointCloud &input,
        const geometry::KDTreeFlann &kdtree,
        const geometry::KDTreeSearchParam &search_param) {
    const size_t n_spfh = input.points_.size();
    auto feature = std::make_shared<Feature>();
    feature->Resize(33, static_cast<int>(n_spfh));

#pragma omp parallel for schedule(static) num_threads(utility::EstimateMaxThreads())
    for (int i = 0; i < static_cast<int>(n_spfh); i++) {
        const auto &point = input.points_[i];
        const auto &normal = input.normals_[i];
        std::vector<int> indices;
        std::vector<double> distance2;
        if (kdtree.Search(point, search_param, indices, distance2) > 1) {
            double hist_incr = 100.0 / static_cast<double>(indices.size() - 1);
            for (size_t k = 1; k < indices.size(); k++) {
                auto pf = ComputePairFeatures(point, normal,
                                              input.points_[indices[k]],
                                              input.normals_[indices[k]]);
                int h_index = static_cast<int>(floor(11 * (pf(0) + M_PI) / (2.0 * M_PI)));
                h_index = std::clamp(h_index, 0, 10);
                feature->data_(h_index, i) += hist_incr;

                h_index = static_cast<int>(floor(11 * (pf(1) + 1.0) * 0.5));
                h_index = std::clamp(h_index, 0, 10);
                feature->data_(h_index + 11, i) += hist_incr;

                h_index = static_cast<int>(floor(11 * (pf(2) + 1.0) * 0.5));
                h_index = std::clamp(h_index, 0, 10);
                feature->data_(h_index + 22, i) += hist_incr;
            }
        }
    }
    return feature;
}

std::shared_ptr<Feature> ComputeFPFHFeature(
        const geometry::PointCloud &input,
        const geometry::KDTreeSearchParam &search_param) {
    if (!input.HasNormals()) {
        utility::LogError("Failed because input point cloud has no normal.");
    }

    const size_t n_points = input.points_.size();
    geometry::KDTreeFlann kdtree(input);

    auto feature = std::make_shared<Feature>();
    feature->Resize(33, static_cast<int>(n_points));

    auto spfh = ComputeSPFHFeature(input, kdtree, search_param);
    if (spfh == nullptr) {
        utility::LogError("Internal error: SPFH feature is nullptr.");
    }

#pragma omp parallel for schedule(static) num_threads(utility::EstimateMaxThreads())
    for (int i = 0; i < static_cast<int>(n_points); i++) {
        int i_spfh = i;
        std::vector<int> indices;
        std::vector<double> distance2;
        kdtree.Search(input.points_[i], search_param, indices, distance2);

        if (indices.size() > 1) {
            double sum[3] = {0.0, 0.0, 0.0};
            for (size_t k = 1; k < indices.size(); k++) {
                double dist = distance2[k];
                if (dist == 0.0) continue;
                int p_index_k = indices[k];
                for (int j = 0; j < 33; j++) {
                    double val = spfh->data_(j, p_index_k) / dist;
                    sum[j / 11] += val;
                    feature->data_(j, i) += val;
                }
            }
            for (int j = 0; j < 3; j++) {
                if (sum[j] != 0.0) sum[j] = 100.0 / sum[j];
            }
            for (int j = 0; j < 33; j++) {
                feature->data_(j, i) *= sum[j / 11];
                feature->data_(j, i) += spfh->data_(j, i_spfh);
            }
        }
    }

    utility::LogDebug(
            "[ComputeFPFHFeature] Computed {:d} features from "
            "input point cloud with {:d} points.",
            static_cast<int>(feature->data_.cols()), static_cast<int>(input.points_.size()));

    return feature;
}

CorrespondenceSet CorrespondencesFromFeatures(const Feature &source_features,
                                              const Feature &target_features,
                                              bool mutual_filter,
                                              float mutual_consistent_ratio) {
    const int num_searches = mutual_filter ? 2 : 1;

    std::array<std::reference_wrapper<const Feature>, 2> features{
            std::reference_wrapper<const Feature>(source_features),
            std::reference_wrapper<const Feature>(target_features)};
    std::array<int, 2> num_pts{static_cast<int>(source_features.data_.cols()),
                               static_cast<int>(target_features.data_.cols())};
    std::vector<CorrespondenceSet> corres(num_searches);

    const int kMaxThreads = utility::EstimateMaxThreads();
    const int kOuterThreads = std::min(kMaxThreads, num_searches);
    const int kInnerThreads = std::max(kMaxThreads / num_searches, 1);
    (void)kOuterThreads;
    (void)kInnerThreads;

#pragma omp parallel for num_threads(kOuterThreads)
    for (int k = 0; k < num_searches; ++k) {
        geometry::KDTreeFlann kdtree(features[1 - k]);

        int num_pts_k = num_pts[k];
        corres[k] = CorrespondenceSet(num_pts_k);
#pragma omp parallel for num_threads(kInnerThreads)
        for (int i = 0; i < num_pts_k; i++) {
            std::vector<int> corres_tmp(1);
            std::vector<double> dist_tmp(1);

            kdtree.SearchKNN(Eigen::VectorXd(features[k].get().data_.col(i)), 1,
                             corres_tmp, dist_tmp);
            int j = corres_tmp[0];
            corres[k][i] = Eigen::Vector2i(i, j);
        }
    }

    if (!mutual_filter) return corres[0];

    CorrespondenceSet corres_mutual;
    int num_src_pts = num_pts[0];
    for (int i = 0; i < num_src_pts; ++i) {
        int j = corres[0][i](1);  // Get the target index from the first correspondence set
        if (corres[1][j](0) == i) {  // Check if the correspondence is mutual
            corres_mutual.emplace_back(i, j);
        }
    }

    if (static_cast<int>(corres_mutual.size()) >=
        static_cast<int>(mutual_consistent_ratio * num_src_pts)) {
        utility::LogDebug("{:d} correspondences remain after mutual filter",
                          static_cast<int>(corres_mutual.size()));
        return corres_mutual;
    }
    utility::LogWarning(
            "Too few correspondences ({:d}) after mutual filter, fall back to "
            "original correspondences.",
            static_cast<int>(corres_mutual.size()));
    return corres[0];
}

}  // namespace registration
}  // namespace pipelines
}  // namespace tiny3d
