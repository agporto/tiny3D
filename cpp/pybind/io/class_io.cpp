// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include <string>
#include <unordered_map>


#include "tiny3d/io/FileFormatIO.h"
#include "tiny3d/io/ModelIO.h"
#include "tiny3d/io/PointCloudIO.h"
#include "tiny3d/io/TriangleMeshIO.h"
#include "pybind/docstring.h"
#include "pybind/io/io.h"


namespace tiny3d {
namespace io {

// IO functions have similar arguments, thus the arg docstrings may be shared
static const std::unordered_map<std::string, std::string>
        map_shared_argument_docstrings = {
                {"filename", "Path to file."},
                // Write options
                {"compressed",
                 "Set to ``True`` to write in compressed format."},
                {"format",
                 "The format of the input file. When not specified or set as "
                 "``auto``, the format is inferred from file extension name."},
                {"remove_nan_points",
                 "If true, all points that include a NaN are removed from "
                 "the PointCloud."},
                {"remove_infinite_points",
                 "If true, all points that include an infinite value are "
                 "removed from the PointCloud."},
                {"quality", "Quality of the output file."},
                {"write_ascii",
                 "Set to ``True`` to output in ascii format, otherwise binary "
                 "format will be used."},
                {"write_vertex_normals",
                 "Set to ``False`` to not write any vertex normals, even if "
                 "present on the mesh"},
                {"write_vertex_colors",
                 "Set to ``False`` to not write any vertex colors, even if "
                 "present on the mesh"},
                // Entities
                {"pointcloud", "The ``PointCloud`` object for I/O"},
                {"mesh", "The ``TriangleMesh`` object for I/O"},
                {"feature", "The ``Feature`` object for I/O"},
                {"print_progress",
                 "If set to true a progress bar is visualized in the console"},
};

void pybind_class_io_declarations(py::module &m_io) {
    py::enum_<FileGeometry> geom_type(m_io, "FileGeometry", py::arithmetic());
    // Trick to write docs without listing the members in the enum class again.
    geom_type.attr("__doc__") = docstring::static_property(
            py::cpp_function([](py::handle arg) -> std::string {
                return "Geometry types";
            }),
            py::none(), py::none(), "");
    geom_type.value("CONTENTS_UNKNOWN", FileGeometry::CONTENTS_UNKNOWN)
            .value("CONTAINS_POINTS", FileGeometry::CONTAINS_POINTS)
            .value("CONTAINS_LINES", FileGeometry::CONTAINS_LINES)
            .value("CONTAINS_TRIANGLES", FileGeometry::CONTAINS_TRIANGLES)
            .export_values();
}
void pybind_class_io_definitions(py::module &m_io) {
    m_io.def(
            "read_file_geometry_type", &ReadFileGeometryType,
            "Returns the type of geometry of the file. This is a faster way of "
            "determining the file type than attempting to read the file as a "
            "point cloud, mesh, or line set in turn.");


    // tiny3d::geometry::PointCloud
    m_io.def(
            "read_point_cloud",
            [](const fs::path &filename, const std::string &format,
               bool remove_nan_points, bool remove_infinite_points,
               bool print_progress) {
                py::gil_scoped_release release;
                geometry::PointCloud pcd;
                ReadPointCloud(filename.string(), pcd,
                               {format, remove_nan_points,
                                remove_infinite_points, print_progress});
                return pcd;
            },
            "Function to read PointCloud from file", "filename"_a,
            "format"_a = "auto", "remove_nan_points"_a = false,
            "remove_infinite_points"_a = false, "print_progress"_a = false);
    docstring::FunctionDocInject(m_io, "read_point_cloud",
                                 map_shared_argument_docstrings);

    m_io.def(
            "read_point_cloud_from_bytes",
            [](const py::bytes &bytes, const std::string &format,
               bool remove_nan_points, bool remove_infinite_points,
               bool print_progress) {
                const char *dataptr = PYBIND11_BYTES_AS_STRING(bytes.ptr());
                auto length = PYBIND11_BYTES_SIZE(bytes.ptr());
                auto buffer = new unsigned char[length];
                // copy before releasing GIL
                std::memcpy(buffer, dataptr, length);
                py::gil_scoped_release release;
                geometry::PointCloud pcd;
                ReadPointCloud(reinterpret_cast<const unsigned char *>(buffer),
                               length, pcd,
                               {format, remove_nan_points,
                                remove_infinite_points, print_progress});
                delete[] buffer;
                return pcd;
            },
            "Function to read PointCloud from memory", "bytes"_a,
            "format"_a = "auto", "remove_nan_points"_a = false,
            "remove_infinite_points"_a = false, "print_progress"_a = false);
    docstring::FunctionDocInject(m_io, "read_point_cloud_from_bytes",
                                 map_shared_argument_docstrings);

    m_io.def(
            "write_point_cloud",
            [](const fs::path &filename, const geometry::PointCloud &pointcloud,
               const std::string &format, bool write_ascii, bool compressed,
               bool print_progress) {
                py::gil_scoped_release release;
                return WritePointCloud(
                        filename.string(), pointcloud,
                        {format, write_ascii, compressed, print_progress});
            },
            "Function to write PointCloud to file", "filename"_a,
            "pointcloud"_a, "format"_a = "auto", "write_ascii"_a = false,
            "compressed"_a = false, "print_progress"_a = false);
    docstring::FunctionDocInject(m_io, "write_point_cloud",
                                 map_shared_argument_docstrings);

    m_io.def(
            "write_point_cloud_to_bytes",
            [](const geometry::PointCloud &pointcloud,
               const std::string &format, bool write_ascii, bool compressed,
               bool print_progress) {
                py::gil_scoped_release release;
                size_t len = 0;
                unsigned char *buffer = nullptr;
                bool wrote = WritePointCloud(
                        buffer, len, pointcloud,
                        {format, write_ascii, compressed, print_progress});
                py::gil_scoped_acquire acquire;
                if (!wrote) {
                    return py::bytes();
                }
                auto ret =
                        py::bytes(reinterpret_cast<const char *>(buffer), len);
                delete[] buffer;
                return ret;
            },
            "Function to write PointCloud to memory", "pointcloud"_a,
            "format"_a = "auto", "write_ascii"_a = false,
            "compressed"_a = false, "print_progress"_a = false);
    docstring::FunctionDocInject(m_io, "write_point_cloud_to_bytes",
                                 map_shared_argument_docstrings);

    // tiny3d::geometry::TriangleMesh
    m_io.def(
            "read_triangle_mesh",
            [](const fs::path &filename, bool enable_post_processing,
               bool print_progress) {
                py::gil_scoped_release release;
                geometry::TriangleMesh mesh;
                ReadTriangleMeshOptions opt;
                opt.enable_post_processing = enable_post_processing;
                opt.print_progress = print_progress;
                ReadTriangleMesh(filename.string(), mesh, opt);
                return mesh;
            },
            "Function to read TriangleMesh from file", "filename"_a,
            "enable_post_processing"_a = false, "print_progress"_a = false);
    docstring::FunctionDocInject(m_io, "read_triangle_mesh",
                                 map_shared_argument_docstrings);

    m_io.def(
            "write_triangle_mesh",
            [](const fs::path &filename, const geometry::TriangleMesh &mesh,
               bool write_ascii, bool compressed, bool write_vertex_normals,
               bool write_vertex_colors, bool write_triangle_uvs,
               bool print_progress) {
                py::gil_scoped_release release;
                return WriteTriangleMesh(filename.string(), mesh, write_ascii,
                                         compressed, write_vertex_normals,
                                         write_vertex_colors,
                                         write_triangle_uvs, print_progress);
            },
            "Function to write TriangleMesh to file", "filename"_a, "mesh"_a,
            "write_ascii"_a = false, "compressed"_a = false,
            "write_vertex_normals"_a = true, "write_vertex_colors"_a = false,
            "write_triangle_uvs"_a = false, "print_progress"_a = false);
    docstring::FunctionDocInject(m_io, "write_triangle_mesh",
                                 map_shared_argument_docstrings);



}

}  // namespace io
}  // namespace tiny3d
