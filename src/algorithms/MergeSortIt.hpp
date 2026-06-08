// ============================================================
// MergeSortIt.hpp
// ============================================================
#pragma once
#include "../SortAlgorithms.hpp"

namespace Algorithms
{
    void mergeSortIt(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m);
}