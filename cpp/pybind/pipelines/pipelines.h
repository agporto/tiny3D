// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#pragma once

#include "pybind/tiny3d_pybind.h"

namespace tiny3d {
namespace pipelines {

void pybind_pipelines_declarations(py::module& m);
void pybind_pipelines_definitions(py::module& m);

}  // namespace pipelines
}  // namespace tiny3d
