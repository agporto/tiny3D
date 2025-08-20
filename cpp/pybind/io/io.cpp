// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "pybind/io/io.h"

#include "pybind/tiny3d_pybind.h"

namespace tiny3d {
namespace io {

void pybind_io_declarations(py::module &m) {
    py::module m_io = m.def_submodule("io");
    pybind_class_io_declarations(m_io);

}

void pybind_io_definitions(py::module &m) {
    auto m_io = static_cast<py::module>(m.attr("io"));
    pybind_class_io_definitions(m_io);

}

}  // namespace io
}  // namespace tiny3d
