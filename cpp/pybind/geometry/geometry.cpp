// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "tiny3d/geometry/Geometry.h"

#include "pybind/docstring.h"
#include "pybind/geometry/geometry.h"
#include "pybind/geometry/geometry_trampoline.h"

namespace tiny3d {
namespace geometry {

void pybind_geometry_classes_declarations(py::module &m) {
    py::class_<Geometry, PyGeometry<Geometry>, std::shared_ptr<Geometry>>
            geometry(m, "Geometry", "The base geometry class.");
    py::enum_<Geometry::GeometryType> geometry_type(geometry, "Type",
                                                    py::arithmetic());
    // Trick to write docs without listing the members in the enum class again.
    geometry_type.attr("__doc__") = docstring::static_property(
            py::cpp_function([](py::handle arg) -> std::string {
                return "Enum class for Geometry types.";
            }),
            py::none(), py::none(), "");

    geometry_type.value("Unspecified", Geometry::GeometryType::Unspecified)
            .value("PointCloud", Geometry::GeometryType::PointCloud)
            .value("VoxelGrid", Geometry::GeometryType::VoxelGrid)
            .value("TriangleMesh", Geometry::GeometryType::TriangleMesh)
            .export_values();
    py::class_<Geometry3D, PyGeometry3D<Geometry3D>,
               std::shared_ptr<Geometry3D>, Geometry>
            geometry3d(m, "Geometry3D",
                       "The base geometry class for 3D geometries.");
    py::class_<Geometry2D, PyGeometry2D<Geometry2D>,
               std::shared_ptr<Geometry2D>, Geometry>
            geometry2d(m, "Geometry2D",
                       "The base geometry class for 2D geometries.");

    m.attr("m") = py::module_::import("typing").attr("TypeVar")("m");
    m.attr("n") = py::module_::import("typing").attr("TypeVar")("n");
}
void pybind_geometry_classes_definitions(py::module &m) {
    // tiny3d.geometry functions
    m.def("get_rotation_matrix_from_xyz", &Geometry3D::GetRotationMatrixFromXYZ,
          "rotation"_a);
    m.def("get_rotation_matrix_from_yzx", &Geometry3D::GetRotationMatrixFromYZX,
          "rotation"_a);
    m.def("get_rotation_matrix_from_zxy", &Geometry3D::GetRotationMatrixFromZXY,
          "rotation"_a);
    m.def("get_rotation_matrix_from_xzy", &Geometry3D::GetRotationMatrixFromXZY,
          "rotation"_a);
    m.def("get_rotation_matrix_from_zyx", &Geometry3D::GetRotationMatrixFromZYX,
          "rotation"_a);
    m.def("get_rotation_matrix_from_yxz", &Geometry3D::GetRotationMatrixFromYXZ,
          "rotation"_a);
    m.def("get_rotation_matrix_from_axis_angle",
          &Geometry3D::GetRotationMatrixFromAxisAngle, "rotation"_a);
    m.def("get_rotation_matrix_from_quaternion",
          &Geometry3D::GetRotationMatrixFromQuaternion, "rotation"_a);

    // tiny3d.geometry.Geometry
    auto geometry = static_cast<py::class_<Geometry, PyGeometry<Geometry>,
                                           std::shared_ptr<Geometry>>>(
            m.attr("Geometry"));
    geometry.def("clear", &Geometry::Clear,
                 "Clear all elements in the geometry.")
            .def("is_empty", &Geometry::IsEmpty,
                 "Returns ``True`` iff the geometry is empty.")
            .def("get_geometry_type", &Geometry::GetGeometryType,
                 "Returns one of registered geometry types.")
            .def("dimension", &Geometry::Dimension,
                 "Returns whether the geometry is 2D or 3D.");
    docstring::ClassMethodDocInject(m, "Geometry", "clear");
    docstring::ClassMethodDocInject(m, "Geometry", "is_empty");
    docstring::ClassMethodDocInject(m, "Geometry", "get_geometry_type");
    docstring::ClassMethodDocInject(m, "Geometry", "dimension");
    // tiny3d.geometry.Geometry3D
    auto geometry3d =
            static_cast<py::class_<Geometry3D, PyGeometry3D<Geometry3D>,
                                   std::shared_ptr<Geometry3D>, Geometry>>(
                    m.attr("Geometry3D"));
    geometry3d
            .def("get_min_bound", &Geometry3D::GetMinBound,
                 "Returns min bounds for geometry coordinates.")
            .def("get_max_bound", &Geometry3D::GetMaxBound,
                 "Returns max bounds for geometry coordinates.")
            .def("get_center", &Geometry3D::GetCenter,
                 "Returns the center of the geometry coordinates.")
            .def("get_axis_aligned_bounding_box",
                 &Geometry3D::GetAxisAlignedBoundingBox,
                 "Returns an axis-aligned bounding box of the geometry.")
            .def("transform", &Geometry3D::Transform,
                 "Apply transformation (4x4 matrix) to the geometry "
                 "coordinates.")
            .def("translate", &Geometry3D::Translate,
                 "Apply translation to the geometry coordinates.",
                 "translation"_a, "relative"_a = true)
            .def("scale",
                 (Geometry3D &
                  (Geometry3D::*)(const double, const Eigen::Vector3d &)) &
                         Geometry3D::Scale,
                 "Apply scaling to the geometry coordinates.", "scale"_a,
                 "center"_a)
            .def("scale", &Geometry3D::Scale,
                 "Apply scaling to the geometry coordinates.", "scale"_a,
                 "center"_a)
            .def("rotate",
                 py::overload_cast<const Eigen::Matrix3d &>(
                         &Geometry3D::Rotate),
                 "Apply rotation to the geometry coordinates and normals.",
                 "R"_a)
            .def("rotate",
                 py::overload_cast<const Eigen::Matrix3d &,
                                   const Eigen::Vector3d &>(
                         &Geometry3D::Rotate),
                 "Apply rotation to the geometry coordinates and normals.",
                 "R"_a, "center"_a)
            .def_static("get_rotation_matrix_from_xyz",
                        &Geometry3D::GetRotationMatrixFromXYZ, "rotation"_a)
            .def_static("get_rotation_matrix_from_yzx",
                        &Geometry3D::GetRotationMatrixFromYZX, "rotation"_a)
            .def_static("get_rotation_matrix_from_zxy",
                        &Geometry3D::GetRotationMatrixFromZXY, "rotation"_a)
            .def_static("get_rotation_matrix_from_xzy",
                        &Geometry3D::GetRotationMatrixFromXZY, "rotation"_a)
            .def_static("get_rotation_matrix_from_zyx",
                        &Geometry3D::GetRotationMatrixFromZYX, "rotation"_a)
            .def_static("get_rotation_matrix_from_yxz",
                        &Geometry3D::GetRotationMatrixFromYXZ, "rotation"_a)
            .def_static("get_rotation_matrix_from_axis_angle",
                        &Geometry3D::GetRotationMatrixFromAxisAngle,
                        "rotation"_a)
            .def_static("get_rotation_matrix_from_quaternion",
                        &Geometry3D::GetRotationMatrixFromQuaternion,
                        "rotation"_a);
    docstring::ClassMethodDocInject(m, "Geometry3D", "get_min_bound");
    docstring::ClassMethodDocInject(m, "Geometry3D", "get_max_bound");
    docstring::ClassMethodDocInject(m, "Geometry3D", "get_center");
    docstring::ClassMethodDocInject(m, "Geometry3D",
                                    "get_axis_aligned_bounding_box");
    docstring::ClassMethodDocInject(m, "Geometry3D", "transform");
    docstring::ClassMethodDocInject(
            m, "Geometry3D", "translate",
            {{"translation", "A 3D vector to transform the geometry"},
             {"relative",
              "If true, the translation vector is directly added to the "
              "geometry "
              "coordinates. Otherwise, the center is moved to the translation "
              "vector."}});
    docstring::ClassMethodDocInject(
            m, "Geometry3D", "scale",
            {{"scale",
              "The scale parameter that is multiplied to the points/vertices "
              "of the geometry."},
             {"center", "Scale center used for transformation."}});
    docstring::ClassMethodDocInject(
            m, "Geometry3D", "rotate",
            {{"R", "The rotation matrix"},
             {"center", "Rotation center used for transformation."}});

    // tiny3d.geometry.Geometry2D
    auto geometry2d =
            static_cast<py::class_<Geometry2D, PyGeometry2D<Geometry2D>,
                                   std::shared_ptr<Geometry2D>, Geometry>>(
                    m.attr("Geometry2D"));
    geometry2d
            .def("get_min_bound", &Geometry2D::GetMinBound,
                 "Returns min bounds for geometry coordinates.")
            .def("get_max_bound", &Geometry2D::GetMaxBound,
                 "Returns max bounds for geometry coordinates.");
    docstring::ClassMethodDocInject(m, "Geometry2D", "get_min_bound");
    docstring::ClassMethodDocInject(m, "Geometry2D", "get_max_bound");
}

void pybind_geometry_declarations(py::module &m) {
    py::module m_geometry = m.def_submodule("geometry");
    pybind_geometry_classes_declarations(m_geometry);
    pybind_kdtreeflann_declarations(m_geometry);
    pybind_pointcloud_declarations(m_geometry);
    pybind_voxelgrid_declarations(m_geometry);
    pybind_meshbase_declarations(m_geometry);
    pybind_trianglemesh_declarations(m_geometry);
    pybind_boundingvolume_declarations(m_geometry);
}

void pybind_geometry_definitions(py::module &m) {
    auto m_geometry = static_cast<py::module>(m.attr("geometry"));
    pybind_geometry_classes_definitions(m_geometry);
    pybind_kdtreeflann_definitions(m_geometry);
    pybind_pointcloud_definitions(m_geometry);
    pybind_voxelgrid_definitions(m_geometry);
    pybind_meshbase_definitions(m_geometry);
    pybind_trianglemesh_definitions(m_geometry);
    pybind_boundingvolume_definitions(m_geometry);
}

}  // namespace geometry
}  // namespace tiny3d
