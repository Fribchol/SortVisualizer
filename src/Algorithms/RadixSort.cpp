#include "SortAlgorithms.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>

// ==================================================================
// Radixsort (LSD, Least Significant Digit first), Basis 10
// ==================================================================
// Die "copy back"-Phase pro Ziffernposition (Folie 283 im Skript)
// ist wie bei Countingsort ein Schreiben in das Array -> Overwrite.

namespace SortAlgorithms
{
    namespace
    {
        template <bool EnableVisuals>
        void radixSortImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
        {
            if (!std::ranges::all_of(arr, [](std::int32_t v) { return v >= 0; })) {
                throw std::invalid_argument(
                    "Radixsort (Basis 10) ist hier nur fuer nichtnegative Werte definiert.");
            }

            const auto maxVal = std::ranges::max(arr);
            const auto n      = static_cast<std::int32_t>(arr.size());

            std::vector<std::int32_t> output(static_cast<std::size_t>(n));
            std::array<std::int32_t, 10> count{};

            for (auto exp = 1; maxVal / exp > 0; exp *= 10)
            {
                std::ranges::fill(count, 0);
                for (auto i = 0; i < n; ++i)
                {
                    const auto digit = (arr[static_cast<std::size_t>(i)] / exp) % 10;
                    ++count[static_cast<std::size_t>(digit)];
                    ++m.arrayAccesses;
                }

                for (auto i = 1; i < 10; ++i)
                {
                    count[static_cast<std::size_t>(i)] += count[static_cast<std::size_t>(i - 1)];
                }

                for (auto i = n - 1; i >= 0; --i)
                {
                    const auto digit = (arr[static_cast<std::size_t>(i)] / exp) % 10;
                    const auto pos   = --count[static_cast<std::size_t>(digit)];
                    output[static_cast<std::size_t>(pos)] = arr[static_cast<std::size_t>(i)];
                    ++m.arrayAccesses;
                }

                for (auto i = 0; i < n; ++i)
                {
                    arr[static_cast<std::size_t>(i)] = output[static_cast<std::size_t>(i)];
                    ++m.arrayAccesses;

                    if constexpr (EnableVisuals) {
                        cb(arr, i, -1, StepKind::Overwrite);
                    }
                }
            }
        }
    } // namespace

    void radixSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        if (cb) {
            radixSortImpl<true>(arr, cb, m);
        } else {
            radixSortImpl<false>(arr, cb, m);
        }
    }

} // namespace SortAlgorithms