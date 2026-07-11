#include "SortAlgorithms.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

namespace SortAlgorithms
{
    namespace
    {
        template <bool EnableVisuals>
        void mergeImpl(std::vector<std::int32_t>& arr,
                       std::vector<std::int32_t>& buffer,
                       std::int32_t               left,
                       std::int32_t               mid,
                       std::int32_t               right,
                       const StepCallback&        cb,
                       LiveMetrics&               m)
        {
            std::copy(arr.begin() + left, arr.begin() + right + 1, buffer.begin() + left);
            m.arrayAccesses += static_cast<std::int64_t>(right - left + 1) * 2;

            auto i = left;
            auto j = mid + 1;

            for (auto k = left; k <= right; ++k)
            {
                const bool takeFromLeft = (j > right) ||
                                           ((i <= mid) && (buffer[static_cast<std::size_t>(i)] <=
                                                            buffer[static_cast<std::size_t>(j)]));

                if (i <= mid && j <= right) ++m.comparisons;

                if (takeFromLeft)
                {
                    arr[static_cast<std::size_t>(k)] = buffer[static_cast<std::size_t>(i)];
                    ++m.arrayAccesses;
                    if constexpr (EnableVisuals) cb(arr, k, i, StepKind::Overwrite);
                    ++i;
                }
                else
                {
                    arr[static_cast<std::size_t>(k)] = buffer[static_cast<std::size_t>(j)];
                    ++m.arrayAccesses;
                    if constexpr (EnableVisuals) cb(arr, k, j, StepKind::Overwrite);
                    ++j;
                }
            }
        }

        template <bool EnableVisuals>
        void mergeSortItImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
        {
            const auto n = static_cast<std::int32_t>(arr.size());

            std::vector<std::int32_t> buffer(static_cast<std::size_t>(n));

            for (auto size = 1; size < n; size *= 2)
            {
                for (auto left = 0; left < n - size; left += 2 * size)
                {
                    const auto mid   = left + size - 1;
                    const auto right = std::min(left + 2 * size - 1, n - 1);
                    mergeImpl<EnableVisuals>(arr, buffer, left, mid, right, cb, m);
                }
            }
        }
    }

    void mergeSortIt(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        if (cb) {
            mergeSortItImpl<true>(arr, cb, m);
        } else {
            mergeSortItImpl<false>(arr, cb, m);
        }
    }

}