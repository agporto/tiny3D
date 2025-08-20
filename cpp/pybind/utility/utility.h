// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#pragma once

#include "pybind/tiny3d_pybind.h"

namespace tiny3d {
namespace utility {

void pybind_utility_declarations(py::module &m);
void pybind_eigen_declarations(py::module &m);
void pybind_logging_declarations(py::module &m);
void pybind_utility_definitions(py::module &m);
void pybind_eigen_definitions(py::module &m);
void pybind_logging_definitions(py::module &m);

namespace random {
void pybind_random(py::module &m);
}

}  // namespace utility
}  // namespace tiny3d
