#include "SortAlgorithms.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

namespace SortAlgorithms
{
    namespace
    {
        template <bool EnableVisuals>
        void countingSortImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
        {
            const auto maxVal = *std::ranges::max_element(arr);
            const auto minVal = *std::ranges::min_element(arr);
            const auto n       = static_cast<std::int32_t>(arr.size());

            const auto range = static_cast<std::size_t>(static_cast<std::int64_t>(maxVal) - minVal + 1);

            std::vector<std::int32_t> count(range, 0);
            std::vector<std::int32_t> output(static_cast<std::size_t>(n));

            for (const auto& val : arr)
            {
                const auto offset = static_cast<std::size_t>(static_cast<std::int64_t>(val) - minVal);
                ++count[offset];
                ++m.arrayAccesses;
            }

            for (std::size_t i = 1; i < range; ++i)
            {
                count[i] += count[i - 1];
                ++m.arrayAccesses;
            }

            for (auto j = n - 1; j >= 0; --j)
            {
                const auto val    = arr[static_cast<std::size_t>(j)];
                const auto offset = static_cast<std::size_t>(static_cast<std::int64_t>(val) - minVal);
                const auto pos    = --count[offset];

                output[static_cast<std::size_t>(pos)] = val;
                m.arrayAccesses += 2;
            }

            for (auto idx = 0; idx < n; ++idx)
            {
                arr[static_cast<std::size_t>(idx)] = output[static_cast<std::size_t>(idx)];
                ++m.arrayAccesses;

                if constexpr (EnableVisuals) {
                    cb(arr, idx, -1, StepKind::Overwrite);
                }
            }
        }
    }

    void countingSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        if (cb) {
            countingSortImpl<true>(arr, cb, m);
        } else {
            countingSortImpl<false>(arr, cb, m);
        }
    }

}