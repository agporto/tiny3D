// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "tiny3d/geometry/BoundingVolume.h"

#include <sstream>

#include "pybind/docstring.h"
#include "pybind/geometry/geometry.h"
#include "pybind/geometry/geometry_trampoline.h"

namespace tiny3d {
namespace geometry {

void pybind_boundingvolume_declarations(py::module &m) {
    py::class_<AxisAlignedBoundingBox, PyGeometry3D<AxisAlignedBoundingBox>,
               std::shared_ptr<AxisAlignedBoundingBox>, Geometry3D>
            axis_aligned_bounding_box(m, "AxisAlignedBoundingBox",
                "Class that defines an axis-aligned box computed from 3D geometries.");
}

void pybind_boundingvolume_definitions(py::module &m) {
    auto axis_aligned_bounding_box = static_cast<py::class_<
            AxisAlignedBoundingBox, PyGeometry3D<AxisAlignedBoundingBox>,
            std::shared_ptr<AxisAlignedBoundingBox>, Geometry3D>>(
            m.attr("AxisAlignedBoundingBox"));

    py::detail::bind_default_constructor<AxisAlignedBoundingBox>(axis_aligned_bounding_box);
    py::detail::bind_copy_functions<AxisAlignedBoundingBox>(axis_aligned_bounding_box);

    axis_aligned_bounding_box
        .def(py::init<const Eigen::Vector3d&, const Eigen::Vector3d&>(),
             "Creates an AxisAlignedBoundingBox from min and max bounds.", "min_bound"_a, "max_bound"_a)
        .def("__repr__",
             [](const AxisAlignedBoundingBox &box) {
                 std::stringstream s;
                 auto mn = box.min_bound_;
                 auto mx = box.max_bound_;
                 s << "AxisAlignedBoundingBox: min: (" << mn.x() << ", "
                   << mn.y() << ", " << mn.z() << "), max: (" << mx.x()
                   << ", " << mx.y() << ", " << mx.z() << ")";
                 return s.str();
             })
        .def(py::self += py::self)
        .def("volume", &AxisAlignedBoundingBox::Volume, "Returns the volume of the bounding box.")
        .def("get_box_points", &AxisAlignedBoundingBox::GetBoxPoints, "Returns the eight points of the box.")
        .def("get_extent", &AxisAlignedBoundingBox::GetExtent, "Returns the XYZ extent of the box.")
        .def("get_half_extent", &AxisAlignedBoundingBox::GetHalfExtent, "Returns half the extent.")
        .def("get_max_extent", &AxisAlignedBoundingBox::GetMaxExtent, "Returns the maximum extent value.")
        .def("get_point_indices_within_bounding_box",
             &AxisAlignedBoundingBox::GetPointIndicesWithinBoundingBox,
             "Returns indices of points within the bounding box.", "points"_a)
        .def("get_print_info", &AxisAlignedBoundingBox::GetPrintInfo, "Returns a human-readable description.")
        .def_static("create_from_points", &AxisAlignedBoundingBox::CreateFromPoints,
                    "Creates an AxisAlignedBoundingBox from a set of points.", "points"_a)
        .def_readwrite("min_bound", &AxisAlignedBoundingBox::min_bound_, "Minimum XYZ bound of the box.")
        .def_readwrite("max_bound", &AxisAlignedBoundingBox::max_bound_, "Maximum XYZ bound of the box.")
        .def_readwrite("color", &AxisAlignedBoundingBox::color_, "Color of the box (for visualization).");

    docstring::ClassMethodDocInject(m, "AxisAlignedBoundingBox", "volume");
    docstring::ClassMethodDocInject(m, "AxisAlignedBoundingBox", "get_box_points");
    docstring::ClassMethodDocInject(m, "AxisAlignedBoundingBox", "get_extent");
    docstring::ClassMethodDocInject(m, "AxisAlignedBoundingBox", "get_half_extent");
    docstring::ClassMethodDocInject(m, "AxisAlignedBoundingBox", "get_max_extent");
    docstring::ClassMethodDocInject(m, "AxisAlignedBoundingBox", "get_point_indices_within_bounding_box",
                                    {{"points", "A list of points."}});
    docstring::ClassMethodDocInject(m, "AxisAlignedBoundingBox", "get_print_info");
    docstring::ClassMethodDocInject(m, "AxisAlignedBoundingBox", "create_from_points",
                                    {{"points", "A list of points."}});
}

}  // namespace geometry
}  // namespace tiny3d
