// ============================================================
// QuickSort.cpp – Lomuto Partition, rekursiv
//
// C++20 Features & Optimierungen (Zero-Cost Abstraction):
// ┌─────────────────────┬──────────────────────────────────────┐
// │ if constexpr        │ Compile-Time Branching! Entfernt     │
// │                     │ ungenutzten Code physisch aus dem    │
// │                     │ Assembly. 100% native Performance.   │
// │ std::format         │ Typsicheres String-Formatting        │
// │ std::swap           │ Effizientes Tauschen                 │
// └─────────────────────┴──────────────────────────────────────┘
// ============================================================
#include "QuickSort.hpp"
#include <algorithm>
#include <format>

namespace Algorithms
{
    // ============================================================
    // partition – Lomuto-Partition als Template
    // ============================================================
    template <bool EnableVisuals>
    static int32_t partition(std::vector<int32_t>& arr,
                              int32_t               low,
                              int32_t               high,
                              const StepCallback&   cb,
                              LiveMetrics&          m)
    {
        const int32_t pivot = arr[high];
        int32_t i = low - 1;

        // Data-Oriented: Enge Schleife, keine Runtime-Branches für Callbacks!
        for (int32_t j = low; j < high; ++j)
        {
            ++m.comparisons;
            ++m.arrayAccesses;

            // if constexpr wird zur Compile-Zeit ausgewertet.
            // Ist EnableVisuals = false, existiert dieser Block im Maschinencode gar nicht!
            if constexpr (EnableVisuals) {
                cb(arr, j, high, std::format(
                    "Vergleich: {} mit Pivot {} – {}",
                    arr[j], pivot,
                    arr[j] <= pivot ? "kleiner/gleich (nach links)" : "groesser (bleibt rechts)"));
            }

            if (arr[j] <= pivot)
            {
                ++i;
                std::swap(arr[i], arr[j]);
                ++m.swaps;
                m.arrayAccesses += 2;

                if constexpr (EnableVisuals) {
                    cb(arr, i, j, std::format("Tausch: {} und {}", arr[i], arr[j]));
                }
            }
        }

        std::swap(arr[i + 1], arr[high]);
        ++m.swaps;
        m.arrayAccesses += 2;

        if constexpr (EnableVisuals) {
            cb(arr, i + 1, high, std::format("Pivot {} ist fixiert an Index {}", arr[i + 1], i + 1));
        }

        return i + 1;
    }

    // ============================================================
    // quickSortRec – Rekursiver Kern als Template
    // ============================================================
    template <bool EnableVisuals>
    static void quickSortRec(std::vector<int32_t>& arr,
                              int32_t               low,
                              int32_t               high,
                              const StepCallback&   cb,
                              LiveMetrics&          m)
    {
        if (low >= high) return;

        const int32_t pi = partition<EnableVisuals>(arr, low, high, cb, m);

        quickSortRec<EnableVisuals>(arr, low, pi - 1, cb, m);
        quickSortRec<EnableVisuals>(arr, pi + 1, high, cb, m);
    }

    // ============================================================
    // quickSort – Öffentliche Schnittstelle
    // Der Dispatcher: Hier gabeln sich die Wege für GUI und CLI.
    // ============================================================
    void quickSort(std::vector<int32_t>& arr,
                   StepCallback          cb,
                   LiveMetrics&          m)
    {
        if (arr.empty()) return;

        if (cb) {
            // GUI-Modus: Compiler baut eine Funktion MIT Strings und Callbacks
            quickSortRec<true>(arr, 0, static_cast<int32_t>(arr.size()) - 1, cb, m);
        } else {
            // CLI-Modus: Compiler baut eine Funktion GANZ OHNE Strings (Max-Speed)
            quickSortRec<false>(arr, 0, static_cast<int32_t>(arr.size()) - 1, cb, m);
        }
    }

} // namespace Algorithms