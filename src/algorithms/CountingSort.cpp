
// CountingSort.cpp – O(n+k), stabil

#include "CountingSort.hpp"
#include <cstdint>

namespace Algorithms
{
    namespace
    {
        template <bool EnableVisuals>
        void countingSortImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
        {
            const auto maxVal = std::ranges::max(arr);
            const auto minVal = std::ranges::min(arr);
            // Sicherer Cast der Differenz zur Vermeidung von Precision-Loss
            const auto range  = static_cast<std::size_t>(static_cast<std::int64_t>(maxVal) - minVal + 1);

            std::vector<std::int32_t> count(range, 0);

            // Phase 1: Häufigkeiten zählen
            for (const auto& val : arr)
            {
                // Hier explizit erst nach int64 konvertieren für die Subtraktion
                const auto offset = static_cast<std::size_t>(static_cast<std::int64_t>(val) - minVal);
                count[offset]++;
                ++m.arrayAccesses;
            }

            // Phase 2: Array rekonstruieren
            for (std::size_t i = 0, idx = 0; i < range; ++i)
            {
                while (count[i]-- > 0)
                {
                    const auto val = static_cast<std::int32_t>(static_cast<std::int64_t>(i) + minVal);
                    arr[idx] = val;
                    ++m.swaps;
                    ++m.arrayAccesses;

                    if constexpr (EnableVisuals) {
                        cb(arr, static_cast<std::int32_t>(idx), -1);
                    }
                    ++idx;
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

} // namespace Algorithms