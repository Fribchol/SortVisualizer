#pragma once

#include <vector>
#include <cstdint>
#include "../SortAlgorithms.hpp"

namespace Algorithms
{
    // ── Modern C++20: Explizite Standard-Typen (std::int32_t) ──
    // angepasst: const StepCallback& cb zur Vermeidung unnötiger Kopien
    void bubbleSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& metrics);
}