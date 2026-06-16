// ============================================================
// CountingSort.hpp
// ============================================================
#pragma once

#include <vector>
#include <cstdint>
#include "../SortAlgorithms.hpp"

namespace Algorithms
{
    // Konsequent mit const StepCallback& zur Vermeidung von Kopien
    void countingSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
}