// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "tiny3d/geometry/TriangleMesh.h"
#include <fmt/format.h>

#include "pybind/docstring.h"
#include "pybind/geometry/geometry.h"
#include "pybind/geometry/geometry_trampoline.h"

namespace tiny3d {
namespace geometry {

void pybind_trianglemesh_declarations(py::module &m) {
    py::class_<TriangleMesh, PyGeometry3D<TriangleMesh>,
               std::shared_ptr<TriangleMesh>, MeshBase>
            trianglemesh(m, "TriangleMesh",
                         "TriangleMesh class. A triangle mesh consists of vertices "
                         "and triangles (faces). Optionally, it may contain normals.");
}

void pybind_trianglemesh_definitions(py::module &m) {
    auto trianglemesh = static_cast<py::class_<TriangleMesh, PyGeometry3D<TriangleMesh>,
                                               std::shared_ptr<TriangleMesh>, MeshBase>>(
        m.attr("TriangleMesh"));

    py::detail::bind_default_constructor<TriangleMesh>(trianglemesh);
    py::detail::bind_copy_functions<TriangleMesh>(trianglemesh);

    trianglemesh
        .def(py::init<const std::vector<Eigen::Vector3d>&,
                      const std::vector<Eigen::Vector3i>&>(),
             "Create a TriangleMesh from vertices and triangle indices.",
             "vertices"_a, "triangles"_a)
        .def("__repr__",
             [](const TriangleMesh &mesh) {
                 return fmt::format("TriangleMesh with {} vertices and {} triangles.",
                                    mesh.vertices_.size(), mesh.triangles_.size());
             })
        .def("compute_triangle_normals", &TriangleMesh::ComputeTriangleNormals,
             "Compute per-triangle normals.", "normalized"_a = true)
        .def("compute_vertex_normals", &TriangleMesh::ComputeVertexNormals,
             "Compute per-vertex normals.", "normalized"_a = true)
        .def("has_vertices", &TriangleMesh::HasVertices,
             "Returns ``True`` if the mesh contains vertices.")
        .def("has_triangles", &TriangleMesh::HasTriangles,
             "Returns ``True`` if the mesh contains triangles.")
        .def("has_vertex_normals", &TriangleMesh::HasVertexNormals,
             "Returns ``True`` if the mesh contains vertex normals.")
        .def("has_triangle_normals", &TriangleMesh::HasTriangleNormals,
             "Returns ``True`` if the mesh contains triangle normals.")
        .def("normalize_normals", &TriangleMesh::NormalizeNormals,
             "Normalize all vertex and triangle normals to unit length.")
        .def("paint_uniform_color", &TriangleMesh::PaintUniformColor,
             "Assign a uniform RGB color to all vertices.", "color"_a)
        .def_readwrite("vertices", &TriangleMesh::vertices_,
             "``float64`` array of shape ``(num_vertices, 3)``: Vertex coordinates.")
        .def_readwrite("vertex_normals", &TriangleMesh::vertex_normals_,
             "``float64`` array of shape ``(num_vertices, 3)``: Vertex normals.")
        .def_readwrite("vertex_colors", &TriangleMesh::vertex_colors_,
             "``float64`` array of shape ``(num_vertices, 3)`` in range [0, 1]: Vertex RGB colors.")
        .def_readwrite("triangles", &TriangleMesh::triangles_,
             "``int`` array of shape ``(num_triangles, 3)``: Triangle vertex indices.")
        .def_readwrite("triangle_normals", &TriangleMesh::triangle_normals_,
             "``float64`` array of shape ``(num_triangles, 3)``: Triangle normals.");
}

}  // namespace geometry
}  // namespace tiny3d
