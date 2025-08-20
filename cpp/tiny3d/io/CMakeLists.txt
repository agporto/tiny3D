add_library(io OBJECT)

target_sources(io PRIVATE
    FileFormatIO.cpp
    ModelIO.cpp
    PointCloudIO.cpp
    TriangleMeshIO.cpp
)

target_sources(io PRIVATE
    file_format/FilePLY.cpp
    file_format/FileXYZ.cpp
)

tiny3d_show_and_abort_on_warning(io)
tiny3d_set_global_properties(io)
tiny3d_set_tiny3d_lib_properties(io)
tiny3d_link_3rdparty_libraries(io)
