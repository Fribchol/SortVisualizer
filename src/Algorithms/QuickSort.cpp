// ============================================================
// QuickSort.cpp – Lomuto Partition, rekursiv (Modern C++)
// ============================================================
// Modern C++20/C++23 & Data-Oriented Design Richtlinien:
// ┌───────────────────┬────────────────────────────────────────────────────────┐
// │ Cache-Lokalität   │ In-Place Partitionierung, es werden keine zusätzlichen │
// │                   │ Heap-Container allokiert. Sequenzielle Zugriffe im L1  │
// │                   │ Cache durch direkte Vektor-Indexierung.                │
// ├───────────────────┼────────────────────────────────────────────────────────┤
// │ RAII              │ Speicherbereinigung erfolgt deterministisch über       │
// │                   │ Stack-basierte Freigabe beim Verlassen des Scopes.     │
// └───────────────────┴────────────────────────────────────────────────────────┘

#include "QuickSort.hpp"
#include <algorithm>
#include <cstdint>

namespace Algorithms
{
    namespace
    {
        // --------------------------------------------------------
        // partition – Lomuto-Partition als Template
        // --------------------------------------------------------
        // Unterteilt das Array in zwei Teilstrukturen basierend auf dem Pivot-Element.
        // Elemente <= Pivot werden links einsortiert, Elemente > Pivot bleiben rechts.
        template <bool EnableVisuals>
        std::size_t partition(std::vector<std::int32_t>& arr,
                              std::size_t                low,
                              std::size_t                high,
                              const StepCallback&        cb,
                              LiveMetrics&               m)
        {
            // Das letzte Element der Sicht wird als Pivot-Element definiert
            const auto pivot = arr[high];

            // Initialisiere die Grenze für Elemente, die kleiner als das Pivot sind (-1 relativ zu low)
            auto i = static_cast<std::ptrdiff_t>(low) - 1;

            for (auto j = low; j < high; ++j)
            {
                ++m.comparisons;
                ++m.arrayAccesses;

                if constexpr (EnableVisuals) {
                    cb(arr, static_cast<std::int32_t>(j), static_cast<std::int32_t>(high));
                }

                // Wenn das Element kleiner oder gleich dem Pivot ist
                if (arr[static_cast<std::size_t>(j)] <= pivot)
                {
                    ++i;
                    std::swap(arr[static_cast<std::size_t>(i)], arr[static_cast<std::size_t>(j)]);
                    ++m.swaps;
                    m.arrayAccesses += 2;

                    if constexpr (EnableVisuals) {
                        cb(arr, static_cast<std::int32_t>(i), static_cast<std::int32_t>(j));
                    }
                }
            }

            // Setze das Pivot-Element an seine finale, sortierte Position
            std::swap(arr[static_cast<std::size_t>(i + 1)], arr[high]);
            ++m.swaps;
            m.arrayAccesses += 2;

            if constexpr (EnableVisuals) {
                cb(arr, static_cast<std::int32_t>(i + 1), static_cast<std::int32_t>(high));
            }

            return static_cast<std::size_t>(i + 1);
        }

        // --------------------------------------------------------
        // quickSortRec – Rekursiver Kern
        // --------------------------------------------------------
        // Parameter 'lowIndex' wurde zu 'low' umbenannt, um den Warnhinweis aufzuheben.
        template <bool EnableVisuals>

        void quickSortRec(std::vector<std::int32_t>& arr,
                          std::size_t                low,
                          std::size_t                high,
                          const StepCallback&        cb,
                          LiveMetrics&               m)
        {
            // Abbruchbedingung: Wenn der Startindex >= dem Endindex ist (Teilbereichslänge <= 1)
            if (low >= high) return;

            // Teile das Array in Partitionen auf und ermittle den Pivot-Index
            const auto pi = partition<EnableVisuals>(arr, low, high, cb, m);

            // Linker Abstieg (Bereich links vom Pivot-Element)
            // Verhindert Vorzeichen-Unterlauf (unsigned underflow) bei Subtraktion von size_t
            if (pi > 0 && low <= pi - 1) {
                quickSortRec<EnableVisuals>(arr, low, pi - 1, cb, m);
            }

            // Rechter Abstieg (Bereich rechts vom Pivot-Element: pi + 1 bis high)
            if (pi < high) {
                quickSortRec<EnableVisuals>(arr, pi + 1, high, cb, m); // wait -> Lokaler Abstieg benötigt korrekten Funktionsaufruf
                quickSortRec<EnableVisuals>(arr, pi + 1, high, cb, m);
            }
        }
    } // namespace

    // --------------------------------------------------------
    // quickSort – Öffentliche Schnittstelle
    // --------------------------------------------------------
    // Kapselt den Start der Algorithmus-Ausführung und schützt Schnittstellen.
    void quickSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        constexpr std::size_t startIdx = 0;
        const std::size_t endIdx = arr.size() - 1;

        if (cb) {
            quickSortRec<true>(arr, startIdx, endIdx, cb, m);
        } else {
            quickSortRec<false>(arr, startIdx, endIdx, cb, m);
        }
    }

} // namespace Algorithms
// NOLINTEND(readability-function-cognitive-complexity, bugprone-recursive-recursion, misc-no-recursion, bugprone-incorrect-roundings)