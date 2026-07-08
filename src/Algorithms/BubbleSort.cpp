#include <utility>
#include <cstdint>
#include "SortAlgorithms.hpp"

namespace SortAlgorithms
{
    namespace
    {
        // --------------------------------------------------------
        // bubbleSortImpl – Kernlogik als Template
        // --------------------------------------------------------
        template <bool EnableVisuals>
        void bubbleSortImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
        {
            std::size_t bound = arr.size(); // oberer Rand des noch unsortierten Bereichs

            // Äußere Schleife läuft, solange im letzten Durchlauf noch getauscht wurde
            while (bound > 1) {
                std::size_t lastSwapIdx = 0; // Position des letzten Swaps in diesem Durchlauf

                // Innere Schleife: Hier verbringt die CPU 99 % der Zeit.
                for (std::size_t j = 0; j + 1 < bound; ++j) {
                    ++m.comparisons;
                    m.arrayAccesses += 2;

                    if constexpr (EnableVisuals) {
                        cb(arr, static_cast<std::int32_t>(j), static_cast<std::int32_t>(j + 1));
                    }

                    if (arr[j] > arr[j + 1]) {
                        std::swap(arr[j], arr[j + 1]);
                        ++m.swaps;
                        m.arrayAccesses += 4;

                        if constexpr (EnableVisuals) {
                            cb(arr, static_cast<std::int32_t>(j), static_cast<std::int32_t>(j + 1));
                        }
                        lastSwapIdx = j + 1;
                    }
                }

                // Kein Swap passiert -> Array ist bereits vollständig sortiert
                if (lastSwapIdx == 0) {
                    if constexpr (EnableVisuals) {
                        cb(arr, -1, -1);
                    }
                    break;
                }

                // Alles ab lastSwapIdx ist garantiert schon an der richtigen Stelle,
                // der nächste Durchlauf muss nur noch bis dorthin prüfen.
                bound = lastSwapIdx;
            }
        }
    } // namespace

    // --------------------------------------------------------
    // bubbleSort – Öffentliche Schnittstelle
    // --------------------------------------------------------
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