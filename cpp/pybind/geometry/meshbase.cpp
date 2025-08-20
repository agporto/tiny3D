// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "tiny3d/geometry/MeshBase.h"

#include "tiny3d/geometry/PointCloud.h"
#include "pybind/docstring.h"
#include "pybind/geometry/geometry.h"
#include "pybind/geometry/geometry_trampoline.h"

namespace tiny3d {
namespace geometry {

void pybind_meshbase_declarations(py::module &m) {
    py::class_<MeshBase, PyGeometry3D<MeshBase>, std::shared_ptr<MeshBase>,
               Geometry3D>
            meshbase(m, "MeshBase",
                     "MeshBase class. Triangle mesh contains vertices. "
                     "Optionally, the mesh "
                     "may also contain vertex normals and vertex colors.");
}

void pybind_meshbase_definitions(py::module &m) {
    auto meshbase =
            static_cast<py::class_<MeshBase, PyGeometry3D<MeshBase>,
                                   std::shared_ptr<MeshBase>, Geometry3D>>(
                    m.attr("MeshBase"));
    py::detail::bind_default_constructor<MeshBase>(meshbase);
    py::detail::bind_copy_functions<MeshBase>(meshbase);

    meshbase.def("__repr__",
                 [](const MeshBase &mesh) {
                     return std::string("MeshBase with ") +
                            std::to_string(mesh.vertices_.size()) + " points";
                 })
            .def("has_vertices", &MeshBase::HasVertices,
                 "Returns ``True`` if the mesh contains vertices.")
            .def("has_vertex_normals", &MeshBase::HasVertexNormals,
                 "Returns ``True`` if the mesh contains vertex normals.")
            .def("has_vertex_colors", &MeshBase::HasVertexColors,
                 "Returns ``True`` if the mesh contains vertex colors.")
            .def("normalize_normals", &MeshBase::NormalizeNormals,
                 "Normalize vertex normals to length 1.")
            .def("paint_uniform_color", &MeshBase::PaintUniformColor,
                 "Assigns each vertex in the MeshBase the same color.",
                 "color"_a)
            .def_readwrite("vertices", &MeshBase::vertices_,
                           "``float64`` array of shape ``(num_vertices, 3)``, "
                           "use ``numpy.asarray()`` to access data: Vertex "
                           "coordinates.")
            .def_readwrite("vertex_normals", &MeshBase::vertex_normals_,
                           "``float64`` array of shape ``(num_vertices, 3)``, "
                           "use ``numpy.asarray()`` to access data: Vertex "
                           "normals.")
            .def_readwrite(
                    "vertex_colors", &MeshBase::vertex_colors_,
                    "``float64`` array of shape ``(num_vertices, 3)``, "
                    "range ``[0, 1]`` , use ``numpy.asarray()`` to access "
                    "data: RGB colors of vertices.");
    docstring::ClassMethodDocInject(m, "MeshBase", "has_vertex_colors");
    docstring::ClassMethodDocInject(
            m, "MeshBase", "has_vertex_normals",
            {{"normalized",
              "Set to ``True`` to normalize the normal to length 1."}});
    docstring::ClassMethodDocInject(m, "MeshBase", "has_vertices");
    docstring::ClassMethodDocInject(m, "MeshBase", "normalize_normals");
    docstring::ClassMethodDocInject(m, "MeshBase", "paint_uniform_color",
                                    {{"color", "RGB colors of vertices."}});
}

}  // namespace geometry
}  // namespace tiny3d
