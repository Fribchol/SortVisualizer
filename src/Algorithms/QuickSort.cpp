#include <algorithm>
#include <cstdint>
#include <utility>
#include "SortAlgorithms.hpp"

namespace SortAlgorithms
{
    namespace
    {
        template <bool EnableVisuals>
        void medianOfThreeIndex(std::vector<std::int32_t>& arr,
                                 std::size_t                low,
                                 std::size_t                high,
                                 const StepCallback&        cb,
                                 LiveMetrics&                m)
        {
            const std::size_t mid = low + (high - low) / 2;

            ++m.comparisons;
            m.arrayAccesses += 2;
            if (arr[low] > arr[mid]) {
                std::swap(arr[low], arr[mid]);
                ++m.swaps;
                m.arrayAccesses += 2;
            }

            ++m.comparisons;
            m.arrayAccesses += 2;
            if (arr[low] > arr[high]) {
                std::swap(arr[low], arr[high]);
                ++m.swaps;
                m.arrayAccesses += 2;
            }

            ++m.comparisons;
            m.arrayAccesses += 2;
            if (arr[mid] > arr[high]) {
                std::swap(arr[mid], arr[high]);
                ++m.swaps;
                m.arrayAccesses += 2;
            }

            std::swap(arr[mid], arr[high]);
            ++m.swaps;
            m.arrayAccesses += 2;

            if constexpr (EnableVisuals) {
                cb(arr, static_cast<std::int32_t>(mid), static_cast<std::int32_t>(high), StepKind::PivotChosen);
            }
        }

        template <bool EnableVisuals>
        [[nodiscard]] std::size_t partition(std::vector<std::int32_t>& arr,
                                             std::size_t                low,
                                             std::size_t                high,
                                             const StepCallback&        cb,
                                             LiveMetrics&                m)
        {
            medianOfThreeIndex<EnableVisuals>(arr, low, high, cb, m);
            const std::int32_t pivot = arr[high];

            auto i = static_cast<std::ptrdiff_t>(low) - 1;

            for (std::size_t j = low; j < high; ++j)
            {
                ++m.comparisons;
                ++m.arrayAccesses;

                if constexpr (EnableVisuals) {
                    cb(arr, static_cast<std::int32_t>(j), static_cast<std::int32_t>(high), StepKind::Compare);
                }

                if (arr[j] <= pivot)
                {
                    ++i;
                    std::swap(arr[static_cast<std::size_t>(i)], arr[j]);
                    ++m.swaps;
                    m.arrayAccesses += 2;

                    if constexpr (EnableVisuals) {
                        cb(arr, static_cast<std::int32_t>(i), static_cast<std::int32_t>(j), StepKind::PartitionSwap);
                    }
                }
            }

            const auto pivotFinalIndex = static_cast<std::size_t>(i + 1);
            std::swap(arr[pivotFinalIndex], arr[high]);
            ++m.swaps;
            m.arrayAccesses += 2;

            if constexpr (EnableVisuals) {
                cb(arr, static_cast<std::int32_t>(pivotFinalIndex), static_cast<std::int32_t>(high), StepKind::PivotPlaced);
            }

            return pivotFinalIndex;
        }

        template <bool EnableVisuals>
        void quickSortRec(std::vector<std::int32_t>& arr,
                           std::size_t                low,
                           std::size_t                high,
                           const StepCallback&        cb,
                           LiveMetrics&                m)
        {

            while (low < high)
            {
                const std::size_t pivotIndex = partition<EnableVisuals>(arr, low, high, cb, m);

                const std::size_t leftSize  = pivotIndex - low;
                const std::size_t rightSize = high - pivotIndex;

                if (leftSize < rightSize)
                {
                    if (pivotIndex > 0 && low <= pivotIndex - 1) {
                        quickSortRec<EnableVisuals>(arr, low, pivotIndex - 1, cb, m);
                    }
                    low = pivotIndex + 1; // größere rechte Hälfte -> iterativ
                }
                else
                {
                    if (pivotIndex < high) {
                        quickSortRec<EnableVisuals>(arr, pivotIndex + 1, high, cb, m);
                    }
                    if (pivotIndex == 0) break; // Schutz vor size_t-Unterlauf
                    high = pivotIndex - 1; // größere linke Hälfte -> iterativ
                }
            }
        }
    }

    void quickSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        constexpr std::size_t startIdx = 0;
        const std::size_t     endIdx   = arr.size() - 1;

        if (cb) {
            quickSortRec<true>(arr, startIdx, endIdx, cb, m);
        } else {
            quickSortRec<false>(arr, startIdx, endIdx, cb, m);
        }
    }

}