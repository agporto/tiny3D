add_library(pipelines OBJECT)

target_sources(pipelines PRIVATE
    registration/CorrespondenceChecker.cpp
    registration/Feature.cpp
    registration/Registration.cpp
    registration/TransformationEstimation.cpp
)

tiny3d_show_and_abort_on_warning(pipelines)
tiny3d_set_global_properties(pipelines)
tiny3d_set_tiny3d_lib_properties(pipelines)
tiny3d_link_3rdparty_libraries(pipelines)
