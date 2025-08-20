// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "tiny3d/geometry/PointCloud.h"

#include <vector>

#include "pybind/docstring.h"
#include "pybind/geometry/geometry.h"
#include "pybind/geometry/geometry_trampoline.h"

namespace tiny3d {
namespace geometry {

void pybind_pointcloud_declarations(py::module &m) {
    py::class_<PointCloud, PyGeometry3D<PointCloud>,
               std::shared_ptr<PointCloud>, Geometry3D>
            pointcloud(m, "PointCloud",
                       "PointCloud class. A point cloud consists of point "
                       "coordinates, and optionally point colors and point "
                       "normals.");
}

void pybind_pointcloud_definitions(py::module &m) {
    auto pointcloud =
            static_cast<py::class_<PointCloud, PyGeometry3D<PointCloud>,
                                   std::shared_ptr<PointCloud>, Geometry3D>>(
                    m.attr("PointCloud"));
    py::detail::bind_default_constructor<PointCloud>(pointcloud);
    py::detail::bind_copy_functions<PointCloud>(pointcloud);

    pointcloud
        .def(py::init<const std::vector<Eigen::Vector3d> &>(),
             "Create a PointCloud from points", "points"_a)
        .def("__repr__",
             [](const PointCloud &pcd) {
                 return std::string("PointCloud with ") +
                        std::to_string(pcd.points_.size()) + " points.";
             })
        .def("has_points", &PointCloud::HasPoints)
        .def("has_normals", &PointCloud::HasNormals)
        .def("has_colors", &PointCloud::HasColors)
        .def("normalize_normals", &PointCloud::NormalizeNormals)
        .def("paint_uniform_color", &PointCloud::PaintUniformColor, "color"_a)
        .def("voxel_down_sample", &PointCloud::VoxelDownSample, "voxel_size"_a)
        .def("estimate_normals", &PointCloud::EstimateNormals,
             "search_param"_a = KDTreeSearchParamKNN(),
             "fast_normal_computation"_a = true)
        .def_readwrite("points", &PointCloud::points_)
        .def_readwrite("normals", &PointCloud::normals_)
        .def_readwrite("colors", &PointCloud::colors_);

    docstring::ClassMethodDocInject(
        m, "PointCloud", "has_points",
        {{"", "Returns ``True`` if the point cloud contains any points."}});

    docstring::ClassMethodDocInject(
        m, "PointCloud", "has_normals",
        {{"", "Returns ``True`` if the point cloud contains point normals."}});

    docstring::ClassMethodDocInject(
        m, "PointCloud", "has_colors",
        {{"", "Returns ``True`` if the point cloud contains point colors."}});

    docstring::ClassMethodDocInject(
        m, "PointCloud", "normalize_normals",
        {{"", "Normalize all point normals to have unit length."}});

    docstring::ClassMethodDocInject(
        m, "PointCloud", "paint_uniform_color",
        {{"color", "RGB color to assign to all points. Each component must be between 0 and 1."}});

    docstring::ClassMethodDocInject(
        m, "PointCloud", "voxel_down_sample",
        {{"voxel_size", "The edge length of each voxel. Points within a voxel are averaged into one output point."}});

    docstring::ClassMethodDocInject(
        m, "PointCloud", "estimate_normals",
        {{"search_param", "Search parameters for finding neighboring points."},
         {"fast_normal_computation", "If ``True``, uses a faster approximate method for normal estimation. If ``False``, uses full eigen decomposition."}});
}

}  // namespace geometry
}  // namespace tiny3d
