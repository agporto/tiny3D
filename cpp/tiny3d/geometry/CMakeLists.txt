add_library(geometry OBJECT)

target_sources(geometry PRIVATE
    BoundingVolume.cpp
    Geometry3D.cpp
    KDTreeFlann.cpp
    MeshBase.cpp
    PointCloud.cpp
    TriangleMesh.cpp
    VoxelGrid.cpp
)

tiny3d_show_and_abort_on_warning(geometry)
tiny3d_set_global_properties(geometry)
tiny3d_set_tiny3d_lib_properties(geometry)
tiny3d_link_3rdparty_libraries(geometry)
