#pragma once

#include <vector>
#include <cstdint>
#include "../SortAlgorithms.hpp"

namespace Algorithms
{
    // Signatur auf const StepCallback& cb angepasst
    void radixSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
}