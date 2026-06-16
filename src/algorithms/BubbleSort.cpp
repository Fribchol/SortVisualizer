// ============================================================
// BubbleSort.cpp
//
// C++20 Features & Modernisierungen:
// ┌─────────────────────┬──────────────────────────────────────┐
// │ Anonymer Namespace  │ Verhindert Linker-Konflikte          │
// │ if constexpr        │ Compile-Time Branching               │
// │ std::int32_t        │ Explizite Typen aus <cstdint>        │
// │ std::format         │ Typsicheres String-Formatting        │
// └─────────────────────┴──────────────────────────────────────┘
// ============================================================
#include "BubbleSort.hpp"
#include <format>
#include <utility>
#include <cstdint>

namespace Algorithms
{
    namespace
    {
        // ============================================================
        // bubbleSortImpl – Kernlogik als Template
        // ============================================================
        template <bool EnableVisuals>
        void bubbleSortImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
        {
            const auto n = static_cast<std::int32_t>(arr.size());

            // Äußere Schleife: Reduziert den zu sortierenden Bereich schrittweise
            for (auto i = 0; i < n - 1; ++i) {
                bool swapped = false;

                // Innere Schleife: Hier verbringt die CPU 99% der Zeit.
                for (auto j = 0; j < n - i - 1; ++j) {
                    ++m.comparisons;
                    m.arrayAccesses += 2;

                    if constexpr (EnableVisuals) {
                        cb(arr, j, j + 1, std::format("Vergleiche {} und {}", arr[static_cast<std::size_t>(j)], arr[static_cast<std::size_t>(j) + 1]));
                    }

                    if (arr[static_cast<std::size_t>(j)] > arr[static_cast<std::size_t>(j) + 1]) {
                        std::swap(arr[static_cast<std::size_t>(j)], arr[static_cast<std::size_t>(j) + 1]);
                        ++m.swaps;
                        m.arrayAccesses += 4;

                        if constexpr (EnableVisuals) {
                            cb(arr, j, j + 1, std::format("Tausche {} und {}", arr[static_cast<std::size_t>(j)], arr[static_cast<std::size_t>(j) + 1]));
                        }
                        swapped = true;
                    }
                }

                if (!swapped) {
                    if constexpr (EnableVisuals) {
                        cb(arr, -1, -1, "Keine Tauschvorgänge mehr nötig, Array ist sortiert!");
                    }
                    break;
                }
            }
        }
    }

    // ============================================================
    // bubbleSort – Öffentliche Schnittstelle
    // ============================================================
    void bubbleSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        if (cb) {
            bubbleSortImpl<true>(arr, cb, m);
        } else {
            bubbleSortImpl<false>(arr, cb, m);
        }
    }

} // namespace Algorithms