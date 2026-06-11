#pragma once

#include <vector>
#include <cstdint>
#include "../SortAlgorithms.hpp"

namespace Algorithms
{
    void bubbleSort(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& metrics);
}