// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "pybind/pipelines/pipelines.h"

#include "pybind/tiny3d_pybind.h"
#include "pybind/pipelines/registration/registration.h"

namespace tiny3d {
namespace pipelines {

void pybind_pipelines_declarations(py::module& m) {
    py::module m_pipelines = m.def_submodule("pipelines");
    registration::pybind_registration_declarations(m_pipelines);
}

void pybind_pipelines_definitions(py::module& m) {
    auto m_pipelines = static_cast<py::module>(m.attr("pipelines"));
    registration::pybind_registration_definitions(m_pipelines);
}

}  // namespace pipelines
}  // namespace tiny3d
