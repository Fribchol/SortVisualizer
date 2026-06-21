
// QuickSort.cpp – Lomuto Partition, rekursiv


#include "QuickSort.hpp"
#include <algorithm>
#include <cstdint>

namespace Algorithms
{
    namespace
    {

        // partition – Lomuto-Partition als Template

        template <bool EnableVisuals>
        std::size_t partition(std::vector<std::int32_t>& arr,
                              std::size_t                low,
                              std::size_t                high,
                              const StepCallback&        cb,
                              LiveMetrics&               m)
        {
            const auto pivot = arr[high];
            auto i = static_cast<std::ptrdiff_t>(low) - 1;

            for (auto j = low; j < high; ++j)
            {
                ++m.comparisons;
                ++m.arrayAccesses;

                if constexpr (EnableVisuals) {
                    cb(arr, static_cast<std::int32_t>(j), static_cast<std::int32_t>(high));
                }

                if (arr[static_cast<std::size_t>(j)] <= pivot)
                {
                    ++i;
                    std::swap(arr[static_cast<std::size_t>(i)], arr[j]);
                    ++m.swaps;
                    m.arrayAccesses += 2;

                    if constexpr (EnableVisuals) {
                        cb(arr, static_cast<std::int32_t>(i), static_cast<std::int32_t>(j));
                    }
                }
            }

            std::swap(arr[static_cast<std::size_t>(i + 1)], arr[high]);
            ++m.swaps;
            m.arrayAccesses += 2;

            if constexpr (EnableVisuals) {
                cb(arr, static_cast<std::int32_t>(i + 1), static_cast<std::int32_t>(high));
            }

            return static_cast<std::size_t>(i + 1);
        }


        // quickSortRec – Rekursiver Kern

        template <bool EnableVisuals>
        void quickSortRec(std::vector<std::int32_t>& arr,
                          std::size_t                lowIndex,
                          std::size_t                high,
                          const StepCallback&        cb,
                          LiveMetrics&               m)
        {
            if (lowIndex >= high) return;

            const auto pi = partition<EnableVisuals>(arr, lowIndex, high, cb, m);

            if (pi > 0) quickSortRec<EnableVisuals>(arr, lowIndex, pi - 1, cb, m);
            quickSortRec<EnableVisuals>(arr, pi + 1, high, cb, m);
        }
    }


    // quickSort – Öffentliche Schnittstelle

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