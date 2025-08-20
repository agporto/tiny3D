#include "tiny3d/geometry/VoxelGrid.h"
#include "tiny3d/geometry/PointCloud.h"  // <-- ADD THIS LINE

#include <sstream>

#include "pybind/docstring.h"
#include "pybind/geometry/geometry.h"
#include "pybind/geometry/geometry_trampoline.h"

namespace tiny3d {
namespace geometry {

void pybind_voxelgrid_declarations(py::module &m) {
    py::class_<Voxel, std::shared_ptr<Voxel>> voxel(
            m, "Voxel", "Base Voxel class, containing grid id and color");
    py::class_<VoxelGrid, PyGeometry3D<VoxelGrid>, std::shared_ptr<VoxelGrid>,
               Geometry3D>
            voxelgrid(m, "VoxelGrid",
                      "VoxelGrid is a collection of voxels aligned in a regular grid.");
}

void pybind_voxelgrid_definitions(py::module &m) {
    auto voxel = static_cast<py::class_<Voxel, std::shared_ptr<Voxel>>>(m.attr("Voxel"));
    py::detail::bind_default_constructor<Voxel>(voxel);
    py::detail::bind_copy_functions<Voxel>(voxel);
    voxel.def("__repr__",
              [](const Voxel &v) {
                  std::ostringstream repr;
                  repr << "Voxel(grid_index=" << v.grid_index_.transpose()
                       << ", color=" << v.color_.transpose() << ")";
                  return repr.str();
              })
          .def(py::init([](const Eigen::Vector3i &grid_index) {
                   return new Voxel(grid_index);
               }), "grid_index"_a)
          .def(py::init([](const Eigen::Vector3i &grid_index, const Eigen::Vector3d &color) {
                   return new Voxel(grid_index, color);
               }), "grid_index"_a, "color"_a)
          .def_readwrite("grid_index", &Voxel::grid_index_,
                         "``int`` numpy array of shape (3,): Voxel grid coordinates.")
          .def_readwrite("color", &Voxel::color_,
                         "``float64`` numpy array of shape (3,): RGB color.");

    auto voxelgrid = static_cast<py::class_<VoxelGrid, PyGeometry3D<VoxelGrid>,
                                            std::shared_ptr<VoxelGrid>, Geometry3D>>(m.attr("VoxelGrid"));
    py::detail::bind_default_constructor<VoxelGrid>(voxelgrid);
    py::detail::bind_copy_functions<VoxelGrid>(voxelgrid);
    voxelgrid
        .def("__repr__",
             [](const VoxelGrid &vg) {
                 return fmt::format("VoxelGrid with {} voxels.", vg.voxels_.size());
             })
        .def("get_voxels", &VoxelGrid::GetVoxels,
             "Returns list of voxels contained in the VoxelGrid.")
        .def("has_voxels", &VoxelGrid::HasVoxels,
             "Returns ``True`` if the VoxelGrid contains voxels.")
        .def("has_colors", &VoxelGrid::HasColors,
             "Returns ``True`` if the VoxelGrid voxels have colors.")
        .def_static("create_from_point_cloud", &VoxelGrid::CreateFromPointCloud,
                    "Create VoxelGrid from a PointCloud.", "input"_a, "voxel_size"_a)
        .def_static("create_from_point_cloud_within_bounds",
                    &VoxelGrid::CreateFromPointCloudWithinBounds,
                    "Create VoxelGrid from a PointCloud within bounds.",
                    "input"_a, "voxel_size"_a, "min_bound"_a, "max_bound"_a)
        .def_readwrite("origin", &VoxelGrid::origin_,
                       "Origin of the VoxelGrid.")
        .def_readwrite("voxel_size", &VoxelGrid::voxel_size_,
                       "Voxel size.");
}

}  // namespace geometry
}  // namespace tiny3d
