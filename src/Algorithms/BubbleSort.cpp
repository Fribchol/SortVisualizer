#include "SortAlgorithms.hpp"
#include <cstdint>
#include <utility>
#include <vector>

// ==================================================================
// Bubblesort (optimiert: schrumpfende Grenze + Früh-Abbruch)
// ==================================================================
// Siehe Kommentar zu StepKind in SortAlgorithms.hpp: Compare markiert
// den reinen Vergleich zweier Nachbarn, Swap den tatsächlichen Tausch,
// Done den Früh-Abbruch (Array war schon vor Ende des Durchlaufs
// vollständig sortiert) - vorher nicht von "Init" unterscheidbar.

namespace SortAlgorithms
{
    namespace
    {
        template <bool EnableVisuals>
        void bubbleSortImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
        {
            std::size_t bound = arr.size();

            while (bound > 1)
            {
                std::size_t lastSwapIdx = 0;

                for (std::size_t j = 0; j + 1 < bound; ++j)
                {
                    ++m.comparisons;
                    m.arrayAccesses += 2;

                    if constexpr (EnableVisuals) {
                        cb(arr, static_cast<std::int32_t>(j), static_cast<std::int32_t>(j + 1), StepKind::Compare);
                    }

                    if (arr[j] > arr[j + 1])
                    {
                        std::swap(arr[j], arr[j + 1]);
                        ++m.swaps;
                        m.arrayAccesses += 2;

                        if constexpr (EnableVisuals) {
                            cb(arr, static_cast<std::int32_t>(j), static_cast<std::int32_t>(j + 1), StepKind::Swap);
                        }
                        lastSwapIdx = j + 1;
                    }
                }

                if (lastSwapIdx == 0)
                {
                    if constexpr (EnableVisuals) {
                        cb(arr, -1, -1, StepKind::Done);
                    }
                    break;
                }

                bound = lastSwapIdx;
            }
        }
    } // namespace

    void bubbleSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        if (cb) {
            bubbleSortImpl<true>(arr, cb, m);
        } else {
            bubbleSortImpl<false>(arr, cb, m);
        }
    }

} // namespace SortAlgorithms