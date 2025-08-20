// ----------------------------------------------------------------------------
// -                        Tiny3D: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "pybind/tiny3d_pybind.h"

#include "tiny3d/utility/Logging.h"
#include "pybind/geometry/geometry.h"
#include "pybind/io/io.h"
#include "pybind/pipelines/pipelines.h"
#include "pybind/utility/utility.h"

namespace tiny3d {

PYBIND11_MODULE(pybind, m) {
    utility::Logger::GetInstance().SetPrintFunction([](const std::string& msg) {
        py::gil_scoped_acquire acquire;
        py::print(msg);
    });

    m.doc() = "Python binding of Tiny3D";

    // Check Tiny3D CXX11_ABI with
    // import tiny3d as t3d; print(t3d.tiny3d_pybind._GLIBCXX_USE_CXX11_ABI)
    m.add_object("_GLIBCXX_USE_CXX11_ABI",
                 _GLIBCXX_USE_CXX11_ABI ? Py_True : Py_False);

    // The binding order matters: if a class haven't been binded, binding the
    // user of this class will result in "could not convert default argument
    // into a Python object" error.
    utility::pybind_utility_declarations(m);
    geometry::pybind_geometry_declarations(m);
    io::pybind_io_declarations(m);
    pipelines::pybind_pipelines_declarations(m);

    utility::pybind_utility_definitions(m);
    geometry::pybind_geometry_definitions(m);
    io::pybind_io_definitions(m);
    pipelines::pybind_pipelines_definitions(m);

    // pybind11 will internally manage the lifetime of default arguments for
    // function bindings. Since these objects will live longer than the memory
    // manager statistics, the latter will report leaks. Reset the statistics to
    // ignore them and transfer the responsibility to pybind11.
}

}  // namespace tiny3d
