// ============================================================
// MergeSortRec.hpp
// ============================================================
#pragma once

#include <vector>
#include <cstdint>
#include "../SortAlgorithms.hpp"

namespace Algorithms
{
    // ── Modern C++20: Explizite Standard-Typen (std::int32_t) ──
    void mergeSortRec(std::vector<std::int32_t>& arr, StepCallback cb, LiveMetrics& m);
}