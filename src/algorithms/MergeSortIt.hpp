// ============================================================
// MergeSortIt.hpp
// ============================================================
#pragma once

#include <vector>
#include <cstdint>
#include "../SortAlgorithms.hpp"

namespace Algorithms
{
    // Signatur auf const StepCallback& angepasst
    void mergeSortIt(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
}