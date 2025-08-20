add_library(utility OBJECT)

target_sources(utility PRIVATE
    CPUInfo.cpp
    Eigen.cpp
    FileSystem.cpp
    Helper.cpp
    Logging.cpp
    Parallel.cpp
    ProgressBar.cpp
    Random.cpp
)

tiny3d_show_and_abort_on_warning(utility)
tiny3d_set_global_properties(utility)
tiny3d_set_tiny3d_lib_properties(utility)
tiny3d_link_3rdparty_libraries(utility)
