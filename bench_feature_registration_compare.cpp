#include <Eigen/Core>
#include <Eigen/Geometry>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include "tiny3d/geometry/KDTreeSearchParam.h"
#include "tiny3d/geometry/PointCloud.h"
#include "tiny3d/pipelines/registration/Feature.h"
#include "tiny3d/pipelines/registration/CorrespondenceChecker.h"
#include "tiny3d/pipelines/registration/Registration.h"
#include "tiny3d/pipelines/registration/TransformationEstimation.h"
#include "tiny3d/utility/Random.h"

namespace {

constexpr double kPi = 3.14159265358979323846;

using Clock = std::chrono::steady_clock;
using tiny3d::geometry::KDTreeSearchParamHybrid;
using tiny3d::geometry::PointCloud;
using tiny3d::pipelines::registration::ComputeFPFHFeature;
using tiny3d::pipelines::registration::CorrespondenceChecker;
using tiny3d::pipelines::registration::CorrespondenceCheckerBasedOnDistance;
using tiny3d::pipelines::registration::CorrespondenceCheckerBasedOnEdgeLength;
using tiny3d::pipelines::registration::CorrespondenceCheckerBasedOnNormal;
using tiny3d::pipelines::registration::CorrespondenceSet;
using tiny3d::pipelines::registration::Feature;
using tiny3d::pipelines::registration::ICPConvergenceCriteria;
using tiny3d::pipelines::registration::RANSACConvergenceCriteria;
using tiny3d::pipelines::registration::RegistrationICP;
using tiny3d::pipelines::registration::RegistrationRANSACBasedOnFeatureMatching;
using tiny3d::pipelines::registration::RegistrationResult;
using tiny3d::pipelines::registration::TransformationEstimationPointToPoint;

struct Scenario {
    std::string name;
    PointCloud source;
    PointCloud target;
    Eigen::Matrix4d ground_truth = Eigen::Matrix4d::Identity();
    Eigen::Matrix4d icp_init = Eigen::Matrix4d::Identity();
};

struct ScenarioConfig {
    std::string name;
    int seed = 7;
    int nx = 68;
    int ny = 58;
    double jitter = 0.004;
    double noise_std = 0.0015;
    double outlier_ratio = 1.0 / 12.0;
    double overlap_keep_ratio = 1.0;
    double rot_z = 0.18;
    double rot_y = -0.11;
    double rot_x = 0.07;
    Eigen::Vector3d translation = Eigen::Vector3d(0.11, -0.06, 0.04);
    double icp_rot_z = 0.03;
    double icp_rot_y = -0.02;
    double icp_rot_x = 0.015;
    Eigen::Vector3d icp_translation_delta =
            Eigen::Vector3d(0.008, -0.006, 0.004);
    double feature_radius = 0.28;
    int feature_max_nn = 64;
    double max_correspondence_distance = 0.08;
    int fpfh_runs = 5;
    int correspondence_runs = 12;
    int ransac_runs = 4;
    int icp_runs = 8;
    int ransac_max_iteration = 40000;
    double ransac_confidence = 0.999;
};

Eigen::Vector3d NormalizeOrFallback(const Eigen::Vector3d &v) {
    const double norm = v.norm();
    if (norm > 0.0) {
        return v / norm;
    }
    return Eigen::Vector3d(0.0, 0.0, 1.0);
}

double RotationErrorDeg(const Eigen::Matrix4d &estimate,
                        const Eigen::Matrix4d &ground_truth) {
    const Eigen::Matrix3d delta =
            estimate.block<3, 3>(0, 0) * ground_truth.block<3, 3>(0, 0).transpose();
    const double cos_theta =
            std::clamp((delta.trace() - 1.0) * 0.5, -1.0, 1.0);
    return std::acos(cos_theta) * 180.0 / kPi;
}

double TranslationError(const Eigen::Matrix4d &estimate,
                        const Eigen::Matrix4d &ground_truth) {
    return (estimate.block<3, 1>(0, 3) - ground_truth.block<3, 1>(0, 3)).norm();
}

template <typename Fn>
double MeanMs(int runs, Fn &&fn) {
    fn();
    double total_ms = 0.0;
    for (int i = 0; i < runs; ++i) {
        const auto start = Clock::now();
        fn();
        const auto stop = Clock::now();
        total_ms += std::chrono::duration<double, std::milli>(stop - start).count();
    }
    return total_ms / static_cast<double>(runs);
}

Scenario MakeScenario(const ScenarioConfig &config) {
    Scenario scenario;
    scenario.name = config.name;
    std::mt19937 rng(config.seed);
    std::uniform_real_distribution<double> jitter(-config.jitter, config.jitter);
    std::normal_distribution<double> noise(0.0, config.noise_std);
    std::uniform_real_distribution<double> outlier_xy(-1.6, 1.6);
    std::uniform_real_distribution<double> outlier_z(-0.6, 0.6);
    std::uniform_real_distribution<double> normal_component(-1.0, 1.0);

    const int nx = config.nx;
    const int ny = config.ny;
    scenario.source.points_.reserve(nx * ny);
    scenario.source.normals_.reserve(nx * ny);

    for (int iy = 0; iy < ny; ++iy) {
        for (int ix = 0; ix < nx; ++ix) {
            const double x =
                    -1.1 + 2.2 * static_cast<double>(ix) / static_cast<double>(nx - 1) +
                    jitter(rng);
            const double y =
                    -0.9 + 1.8 * static_cast<double>(iy) / static_cast<double>(ny - 1) +
                    jitter(rng);
            const double z = 0.24 * std::sin(2.7 * x) + 0.17 * std::cos(3.1 * y) +
                             0.08 * std::sin(1.4 * x + 0.8 * y) + 0.03 * x * y;
            const double dzdx = 0.24 * 2.7 * std::cos(2.7 * x) +
                                0.08 * 1.4 * std::cos(1.4 * x + 0.8 * y) + 0.03 * y;
            const double dzdy = -0.17 * 3.1 * std::sin(3.1 * y) +
                                0.08 * 0.8 * std::cos(1.4 * x + 0.8 * y) + 0.03 * x;
            scenario.source.points_.emplace_back(x, y, z);
            scenario.source.normals_.push_back(
                    NormalizeOrFallback(Eigen::Vector3d(-dzdx, -dzdy, 1.0)));
        }
    }

    const Eigen::Matrix3d rotation =
            (Eigen::AngleAxisd(config.rot_z, Eigen::Vector3d::UnitZ()) *
             Eigen::AngleAxisd(config.rot_y, Eigen::Vector3d::UnitY()) *
             Eigen::AngleAxisd(config.rot_x, Eigen::Vector3d::UnitX()))
                    .toRotationMatrix();
    scenario.ground_truth.setIdentity();
    scenario.ground_truth.block<3, 3>(0, 0) = rotation;
    scenario.ground_truth.block<3, 1>(0, 3) = config.translation;

    scenario.target = scenario.source;
    scenario.target.Transform(scenario.ground_truth);
    for (auto &p : scenario.target.points_) {
        p.x() += noise(rng);
        p.y() += noise(rng);
        p.z() += noise(rng);
    }
    scenario.target.NormalizeNormals();

    if (config.overlap_keep_ratio < 1.0) {
        std::vector<Eigen::Vector3d> filtered_points;
        std::vector<Eigen::Vector3d> filtered_normals;
        filtered_points.reserve(scenario.target.points_.size());
        filtered_normals.reserve(scenario.target.normals_.size());

        double min_x = std::numeric_limits<double>::infinity();
        double max_x = -std::numeric_limits<double>::infinity();
        for (const auto &p : scenario.target.points_) {
            min_x = std::min(min_x, p.x());
            max_x = std::max(max_x, p.x());
        }
        const double cutoff = max_x - config.overlap_keep_ratio * (max_x - min_x);
        for (size_t i = 0; i < scenario.target.points_.size(); ++i) {
            if (scenario.target.points_[i].x() >= cutoff) {
                filtered_points.push_back(scenario.target.points_[i]);
                filtered_normals.push_back(scenario.target.normals_[i]);
            }
        }
        scenario.target.points_ = std::move(filtered_points);
        scenario.target.normals_ = std::move(filtered_normals);
    }

    const int num_outliers = static_cast<int>(
            scenario.target.points_.size() * config.outlier_ratio);
    for (int i = 0; i < num_outliers; ++i) {
        scenario.target.points_.emplace_back(
                outlier_xy(rng), outlier_xy(rng), outlier_z(rng));
        scenario.target.normals_.push_back(NormalizeOrFallback(Eigen::Vector3d(
                normal_component(rng), normal_component(rng), normal_component(rng))));
    }

    const Eigen::Matrix3d icp_delta =
            (Eigen::AngleAxisd(config.icp_rot_z, Eigen::Vector3d::UnitZ()) *
             Eigen::AngleAxisd(config.icp_rot_y, Eigen::Vector3d::UnitY()) *
             Eigen::AngleAxisd(config.icp_rot_x, Eigen::Vector3d::UnitX()))
                    .toRotationMatrix();
    scenario.icp_init = scenario.ground_truth;
    scenario.icp_init.block<3, 3>(0, 0) =
            icp_delta * scenario.icp_init.block<3, 3>(0, 0);
    scenario.icp_init.block<3, 1>(0, 3) += config.icp_translation_delta;

    return scenario;
}

void PrintMetric(const std::string &key, double value) {
    std::cout << key << "=" << std::setprecision(17) << value << "\n";
}

void PrintMetric(const std::string &key, size_t value) {
    std::cout << key << "=" << value << "\n";
}

void PrintScenarioMetric(const std::string &scenario_name,
                         const std::string &key,
                         double value) {
    PrintMetric(scenario_name + "." + key, value);
}

void PrintScenarioMetric(const std::string &scenario_name,
                         const std::string &key,
                         size_t value) {
    PrintMetric(scenario_name + "." + key, value);
}

void RunScenario(const ScenarioConfig &config) {
    const Scenario scenario = MakeScenario(config);
    const KDTreeSearchParamHybrid feature_search(config.feature_radius,
                                                 config.feature_max_nn);
    const double max_correspondence_distance = config.max_correspondence_distance;

    std::shared_ptr<Feature> source_feature;
    std::shared_ptr<Feature> target_feature;
    const double fpfh_ms = MeanMs(config.fpfh_runs, [&]() {
        source_feature = ComputeFPFHFeature(scenario.source, feature_search);
        target_feature = ComputeFPFHFeature(scenario.target, feature_search);
    });

    CorrespondenceSet correspondences;
    const double correspondence_ms = MeanMs(config.correspondence_runs, [&]() {
        correspondences = tiny3d::pipelines::registration::CorrespondencesFromFeatures(
                *source_feature, *target_feature, true, 0.1f);
    });

    CorrespondenceCheckerBasedOnEdgeLength edge_checker(0.9);
    CorrespondenceCheckerBasedOnDistance distance_checker(max_correspondence_distance);
    CorrespondenceCheckerBasedOnNormal normal_checker(0.35);
    std::vector<std::reference_wrapper<const CorrespondenceChecker>> checkers{
            edge_checker, distance_checker, normal_checker};

    RegistrationResult ransac_result;
    const double ransac_ms = MeanMs(config.ransac_runs, [&]() {
        tiny3d::utility::random::Seed(12345);
        ransac_result = RegistrationRANSACBasedOnFeatureMatching(
                scenario.source, scenario.target, *source_feature, *target_feature, true,
                max_correspondence_distance, TransformationEstimationPointToPoint(false), 4,
                checkers, RANSACConvergenceCriteria(config.ransac_max_iteration,
                                                   config.ransac_confidence));
    });

    RegistrationResult icp_result;
    const double icp_ms = MeanMs(config.icp_runs, [&]() {
        icp_result = RegistrationICP(
                scenario.source, scenario.target, max_correspondence_distance,
                scenario.icp_init, TransformationEstimationPointToPoint(false),
                ICPConvergenceCriteria(1e-7, 1e-7, 60));
    });

    PrintScenarioMetric(config.name, "source_points", scenario.source.points_.size());
    PrintScenarioMetric(config.name, "target_points", scenario.target.points_.size());
    PrintScenarioMetric(config.name, "fpfh_mean_ms", fpfh_ms);
    PrintScenarioMetric(config.name, "correspondence_mean_ms", correspondence_ms);
    PrintScenarioMetric(config.name, "ransac_mean_ms", ransac_ms);
    PrintScenarioMetric(config.name, "icp_mean_ms", icp_ms);
    PrintScenarioMetric(config.name, "source_feature_sum", source_feature->data_.sum());
    PrintScenarioMetric(config.name, "target_feature_sum", target_feature->data_.sum());
    PrintScenarioMetric(config.name, "source_feature_sqnorm",
                        source_feature->data_.squaredNorm());
    PrintScenarioMetric(config.name, "target_feature_sqnorm",
                        target_feature->data_.squaredNorm());
    PrintScenarioMetric(config.name, "correspondence_count", correspondences.size());
    PrintScenarioMetric(config.name, "ransac_fitness", ransac_result.fitness_);
    PrintScenarioMetric(config.name, "ransac_rmse", ransac_result.inlier_rmse_);
    PrintScenarioMetric(config.name, "ransac_rotation_error_deg",
                        RotationErrorDeg(ransac_result.transformation_,
                                         scenario.ground_truth));
    PrintScenarioMetric(config.name, "ransac_translation_error",
                        TranslationError(ransac_result.transformation_,
                                         scenario.ground_truth));
    PrintScenarioMetric(config.name, "icp_fitness", icp_result.fitness_);
    PrintScenarioMetric(config.name, "icp_rmse", icp_result.inlier_rmse_);
    PrintScenarioMetric(config.name, "icp_rotation_error_deg",
                        RotationErrorDeg(icp_result.transformation_,
                                         scenario.ground_truth));
    PrintScenarioMetric(config.name, "icp_translation_error",
                        TranslationError(icp_result.transformation_,
                                         scenario.ground_truth));
}

}  // namespace

int main() {
    std::vector<ScenarioConfig> scenarios;

    ScenarioConfig baseline;
    baseline.name = "baseline";
    baseline.seed = 7;
    baseline.nx = 68;
    baseline.ny = 58;
    baseline.noise_std = 0.0015;
    baseline.outlier_ratio = 1.0 / 12.0;
    baseline.overlap_keep_ratio = 1.0;
    scenarios.push_back(baseline);

    ScenarioConfig large_cloud;
    large_cloud.name = "large_cloud";
    large_cloud.seed = 11;
    large_cloud.nx = 120;
    large_cloud.ny = 100;
    large_cloud.noise_std = 0.0015;
    large_cloud.outlier_ratio = 1.0 / 12.0;
    large_cloud.overlap_keep_ratio = 1.0;
    large_cloud.fpfh_runs = 3;
    large_cloud.correspondence_runs = 8;
    large_cloud.ransac_runs = 3;
    large_cloud.icp_runs = 5;
    scenarios.push_back(large_cloud);

    ScenarioConfig low_overlap;
    low_overlap.name = "low_overlap";
    low_overlap.seed = 19;
    low_overlap.nx = 80;
    low_overlap.ny = 64;
    low_overlap.noise_std = 0.0015;
    low_overlap.outlier_ratio = 1.0 / 10.0;
    low_overlap.overlap_keep_ratio = 0.58;
    low_overlap.translation = Eigen::Vector3d(0.18, -0.08, 0.05);
    low_overlap.max_correspondence_distance = 0.1;
    low_overlap.ransac_runs = 3;
    low_overlap.icp_runs = 5;
    scenarios.push_back(low_overlap);

    ScenarioConfig high_outlier;
    high_outlier.name = "high_outlier";
    high_outlier.seed = 29;
    high_outlier.nx = 76;
    high_outlier.ny = 64;
    high_outlier.noise_std = 0.0025;
    high_outlier.outlier_ratio = 0.35;
    high_outlier.overlap_keep_ratio = 1.0;
    high_outlier.translation = Eigen::Vector3d(0.12, -0.03, 0.06);
    high_outlier.max_correspondence_distance = 0.09;
    high_outlier.ransac_runs = 3;
    high_outlier.icp_runs = 5;
    scenarios.push_back(high_outlier);

    for (const auto &scenario : scenarios) {
        RunScenario(scenario);
    }

    return 0;
}
