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
namespace registration {

void pybind_registration_declarations(py::module &m);
void pybind_feature_declarations(py::module &m_registration);

void pybind_registration_definitions(py::module &m);
void pybind_feature_definitions(py::module &m_registration);

}  // namespace registration
}  // namespace pipelines
}  // namespace tiny3d
