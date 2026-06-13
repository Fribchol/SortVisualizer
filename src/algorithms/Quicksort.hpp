// ============================================================
// QuickSort.hpp
// ============================================================
#pragma once

#include <vector>
#include <cstdint>
#include "../SortAlgorithms.hpp"

namespace Algorithms
{
    // ── [[nodiscard]] nicht nötig hier ───────────────────────────
    // void-Funktionen haben keinen Rückgabewert → kein nodiscard
    // ── Modern C++20: Explizite Standard-Typen (std::int32_t) ──
    void quickSort(std::vector<std::int32_t>& arr, StepCallback cb, LiveMetrics& m);
}