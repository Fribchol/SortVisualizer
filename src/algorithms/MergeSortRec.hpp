// ============================================================
// MergeSortRec.hpp
// ============================================================
#pragma once
#include "../SortAlgorithms.hpp"

namespace Algorithms
{
    void mergeSortRec(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m);
}