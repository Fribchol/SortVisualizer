// ============================================================
// QuickSort.cpp – Lomuto Partition, rekursiv
//
// C++20 Features & Modernisierungen:
// ┌─────────────────────┬──────────────────────────────────────┐
// │ Anonymer Namespace  │ Verhindert Linker-Konflikte sauber   │
// │ if constexpr        │ Compile-Time Branching! Entfernt     │
// │                     │ ungenutzten Code physisch aus dem    │
// │                     │ Assembly. 100% native Performance.   │
// │ std::int32_t        │ Explizite Typen aus <cstdint>        │
// │ std::format         │ Typsicheres String-Formatting        │
// │ std::swap           │ Effizientes Tauschen                 │
// └─────────────────────┴──────────────────────────────────────┘
//
// Data-Oriented Design (DOD) Notiz:
// QuickSort ist in der Praxis extrem schnell, da das
// Partitionieren sequenziell über das Array iteriert. Dies
// maximiert Cache-Hits (Spatial Locality), was moderne CPUs
// weitaus effizienter verarbeiten als Sprünge im Speicher.
// ============================================================
#include "QuickSort.hpp"
#include <algorithm>
#include <format>
#include <cstdint> // C++ Standard für feste Integer-Breiten

namespace Algorithms
{
    // ── Anonymer Namespace ──────────────────────────────────────
    // Ersetzt 'static' für interne Funktionen. Alles hierin
    // ist strikt nur in dieser Translation Unit (.cpp) sichtbar.
    namespace
    {
        // ============================================================
        // partition – Lomuto-Partition als Template
        // ============================================================
        template <bool EnableVisuals>
        std::int32_t partition(std::vector<std::int32_t>& arr,
                               std::int32_t               low,
                               std::int32_t               high,
                               const StepCallback&        cb,
                               LiveMetrics&               m)
        {
            const std::int32_t pivot = arr[high];
            std::int32_t i = low - 1;

            // Data-Oriented: Enge Schleife, maximaler Durchsatz für den L1-Cache
            for (std::int32_t j = low; j < high; ++j)
            {
                ++m.comparisons;
                ++m.arrayAccesses;

                // Compile-Time Verzweigung: Im CLI-Modus komplett eliminiert!
                if constexpr (EnableVisuals) {
                    cb(arr, j, high, std::format(
                        "Vergleich: {} mit Pivot {} – {}",
                        arr[j], pivot,
                        arr[j] <= pivot ? "kleiner/gleich (nach links)" : "größer (bleibt rechts)"));
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
        void quickSortRec(std::vector<std::int32_t>& arr,
                          std::int32_t               low,
                          std::int32_t               high,
                          const StepCallback&        cb,
                          LiveMetrics&               m)
        {
            if (low >= high) return;

            const std::int32_t pi = partition<EnableVisuals>(arr, low, high, cb, m);

            quickSortRec<EnableVisuals>(arr, low, pi - 1, cb, m);
            quickSortRec<EnableVisuals>(arr, pi + 1, high, cb, m);
        }
    } // Ende des anonymen Namespaces

    // ============================================================
    // quickSort – Öffentliche Schnittstelle
    // Der Dispatcher: Hier gabeln sich die Wege für GUI und CLI.
    // ============================================================
    void quickSort(std::vector<std::int32_t>& arr,
                   StepCallback          cb,
                   LiveMetrics&          m)
    {
        if (arr.empty()) return;

        if (cb) {
            // GUI-Modus: Compiler baut eine Funktion MIT Strings und Callbacks
            quickSortRec<true>(arr, 0, static_cast<std::int32_t>(arr.size()) - 1, cb, m);
        } else {
            // CLI-Modus: Compiler baut eine Funktion GANZ OHNE Strings (Max-Speed)
            quickSortRec<false>(arr, 0, static_cast<std::int32_t>(arr.size()) - 1, cb, m);
        }
    }

} // namespace Algorithms