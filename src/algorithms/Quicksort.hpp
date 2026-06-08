// ============================================================
// QuickSort.hpp
// ============================================================
#pragma once
#include "../SortAlgorithms.hpp"

namespace Algorithms
{
    // ── [[nodiscard]] nicht nötig hier ───────────────────────────
    // void-Funktionen haben keinen Rückgabewert → kein nodiscard
    void quickSort(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m);
}