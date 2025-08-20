// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#pragma once

#include "pybind/tiny3d_pybind.h"

namespace tiny3d {
namespace io {

void pybind_io_declarations(py::module& m);
void pybind_class_io_declarations(py::module& m);

void pybind_io_definitions(py::module& m);
void pybind_class_io_definitions(py::module& m);



}  // namespace io
}  // namespace tiny3d
