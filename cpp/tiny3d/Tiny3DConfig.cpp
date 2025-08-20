// ----------------------------------------------------------------------------
// -                        Tiny3D: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#include "tiny3d/Tiny3DConfig.h"

#include "tiny3d/utility/Logging.h"

namespace tiny3d {

void PrintTiny3DVersion() { utility::LogInfo("Tiny3D {}", TINY3D_VERSION); }

}  // namespace tiny3d
