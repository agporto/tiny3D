// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "tiny3d/utility/Random.h"

#include "pybind/docstring.h"
#include "pybind/tiny3d_pybind.h"

namespace tiny3d {
namespace utility {
namespace random {

void pybind_random(py::module &m) {
    py::module m_submodule = m.def_submodule("random");

    m_submodule.def("seed", &Seed, "seed"_a, "Set tiny3d global random seed.");

    docstring::FunctionDocInject(m_submodule, "seed",
                                 {{"seed", "Random seed value."}});
}

}  // namespace random
}  // namespace utility
}  // namespace tiny3d
