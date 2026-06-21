
#pragma once

#include <vector>
#include <cstdint>
#include "../SortAlgorithms.hpp"

namespace Algorithms
{
    // Signatur angepasst: const StepCallback& cb
    void mergeSortRec(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
}