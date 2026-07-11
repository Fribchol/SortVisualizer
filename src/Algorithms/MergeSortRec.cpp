#include "SortAlgorithms.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

// ==================================================================
// MergeSort (rekursiv, Top-Down) mit wiederverwendetem Hilfspuffer
// ==================================================================
// Gleicher Merge-Schritt wie in MergeSortIt.cpp (Folie 232),
// entsprechend dieselbe StepKind::Overwrite-Kennzeichnung.

namespace SortAlgorithms
{
    namespace
    {
        template <bool EnableVisuals>
        void mergeImpl(std::vector<std::int32_t>& arr,
                       std::vector<std::int32_t>& buffer,
                       std::size_t                left,
                       std::size_t                mid,
                       std::size_t                right,
                       const StepCallback&        cb,
                       LiveMetrics&               m)
        {
            const std::size_t length = right - left + 1;
            std::copy(arr.begin() + static_cast<std::ptrdiff_t>(left),
                      arr.begin() + static_cast<std::ptrdiff_t>(right) + 1,
                      buffer.begin() + static_cast<std::ptrdiff_t>(left));
            m.arrayAccesses += static_cast<std::uint64_t>(length) * 2;

            std::size_t i = left;
            std::size_t j = mid + 1;
            std::size_t k = left;

            while (i <= mid && j <= right)
            {
                ++m.comparisons;
                m.arrayAccesses += 2;

                if (buffer[i] <= buffer[j])
                {
                    arr[k] = buffer[i];
                    m.arrayAccesses += 2;
                    if constexpr (EnableVisuals) cb(arr, static_cast<std::int32_t>(k), static_cast<std::int32_t>(i), StepKind::Overwrite);
                    ++i;
                }
                else
                {
                    arr[k] = buffer[j];
                    m.arrayAccesses += 2;
                    if constexpr (EnableVisuals) cb(arr, static_cast<std::int32_t>(k), static_cast<std::int32_t>(j), StepKind::Overwrite);
                    ++j;
                }
                ++k;
            }

            while (i <= mid)
            {
                arr[k] = buffer[i];
                m.arrayAccesses += 2;
                if constexpr (EnableVisuals) cb(arr, static_cast<std::int32_t>(k), static_cast<std::int32_t>(i), StepKind::Overwrite);
                ++i; ++k;
            }
            while (j <= right)
            {
                arr[k] = buffer[j];
                m.arrayAccesses += 2;
                if constexpr (EnableVisuals) cb(arr, static_cast<std::int32_t>(k), static_cast<std::int32_t>(j), StepKind::Overwrite);
                ++j; ++k;
            }
        }

        template <bool EnableVisuals>
        void mergeSortRange(std::vector<std::int32_t>& arr,
                            std::vector<std::int32_t>& buffer,
                            std::size_t                low,
                            std::size_t                high,
                            const StepCallback&        cb,
                            LiveMetrics&               m)
        {
            if (low >= high) return;

            const std::size_t mid = low + (high - low) / 2;

            mergeSortRange<EnableVisuals>(arr, buffer, low, mid, cb, m);
            mergeSortRange<EnableVisuals>(arr, buffer, mid + 1, high, cb, m);

            mergeImpl<EnableVisuals>(arr, buffer, low, mid, high, cb, m);
        }
    } // namespace

    void mergeSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.size() < 2) return;

        std::vector<std::int32_t> buffer(arr.size());
        const std::size_t low  = 0;
        const std::size_t high = arr.size() - 1;

        if (cb) mergeSortRange<true>(arr, buffer, low, high, cb, m);
        else    mergeSortRange<false>(arr, buffer, low, high, cb, m);
    }

} // namespace SortAlgorithms