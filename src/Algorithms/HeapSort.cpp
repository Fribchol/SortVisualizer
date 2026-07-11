// HeapSort.cpp – in-place Max-Heap

#include "SortAlgorithms.hpp"
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

// ==================================================================
// Heapsort (in-place, Max-Heap)
// ==================================================================
// Zwei unterschiedliche Tausch-Arten, jetzt auch semantisch getrennt
// (siehe StepKind in SortAlgorithms.hpp):
//   - HeapifySink:   "Versickern" (Folie 240 im Skript) - ein Element
//                     wird mit seinem größeren Kind getauscht, bis die
//                     Heap-Eigenschaft wiederhergestellt ist.
//   - HeapExtract:   Wurzel (Maximum) wird mit dem letzten Element des
//                     aktiven Heap-Bereichs getauscht (Folie 241) -
//                     das ist die eigentliche "Sortier"-Aktion, bei
//                     der ein Wert seine endgültige Position erhält.
// Beide sahen für die Visualisierung vorher identisch aus (nur zwei
// vertauschte Indizes); jetzt lässt sich das im Lerntext klar trennen.

namespace SortAlgorithms
{
    namespace
    {
        template <bool EnableVisuals>
        void heapify(std::vector<std::int32_t>& arr,
                     std::int32_t               n,
                     std::int32_t               i,
                     const StepCallback&        cb,
                     LiveMetrics&               m)
        {
            auto current = i;

            while (true)
            {
                auto largest     = current;
                const auto left  = 2 * current + 1;
                const auto right = 2 * current + 2;

                if (left < n)
                {
                    ++m.comparisons;
                    m.arrayAccesses += 2;
                    if (arr[static_cast<std::size_t>(left)] > arr[static_cast<std::size_t>(largest)])
                        largest = left;
                }

                if (right < n)
                {
                    ++m.comparisons;
                    m.arrayAccesses += 2;
                    if (arr[static_cast<std::size_t>(right)] > arr[static_cast<std::size_t>(largest)])
                        largest = right;
                }

                if (largest != current)
                {
                    std::swap(arr[static_cast<std::size_t>(current)], arr[static_cast<std::size_t>(largest)]);
                    ++m.swaps;
                    m.arrayAccesses += 2;

                    if constexpr (EnableVisuals) cb(arr, current, largest, StepKind::HeapifySink);

                    current = largest;
                }
                else
                {
                    break;
                }
            }
        }

        template <bool EnableVisuals>
        void heapSortImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
        {
            const auto n = static_cast<std::int32_t>(arr.size());

            for (auto i = n / 2 - 1; i >= 0; --i)
            {
                heapify<EnableVisuals>(arr, n, i, cb, m);
            }

            for (auto i = n - 1; i > 0; --i)
            {
                std::swap(arr[0], arr[static_cast<std::size_t>(i)]);
                ++m.swaps;
                m.arrayAccesses += 2;

                if constexpr (EnableVisuals) cb(arr, 0, i, StepKind::HeapExtract);

                heapify<EnableVisuals>(arr, i, 0, cb, m);
            }
        }
    } // namespace

    void heapSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        if (cb) {
            heapSortImpl<true>(arr, cb, m);
        } else {
            heapSortImpl<false>(arr, cb, m);
        }
    }

} // namespace SortAlgorithms