// ----------------------------------------------------------------------------
// -                        tiny3d: www.tiny3d.org                            -
// ----------------------------------------------------------------------------
// Copyright (c) 2018-2024 www.tiny3d.org
// SPDX-License-Identifier: MIT
// ----------------------------------------------------------------------------

#pragma once

namespace tiny3d {
namespace utility {

/// Estimate the maximum number of threads to be used in a parallel region.
int EstimateMaxThreads();

/// Returns true if in an parallel section.
bool InParallel();

}  // namespace utility
}  // namespace tiny3d
