// ============================================================
// RadixSort.cpp – LSD RadixSort, Basis 10
// ============================================================
#include "RadixSort.hpp"
#include <algorithm>
#include <format>
#include <array>
#include <string>
#include <cstdint>

namespace Algorithms
{
    namespace
    {
        // ============================================================
        // radixSortImpl – Kernlogik als Template
        // ============================================================
        template <bool EnableVisuals>
        void radixSortImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
        {
            const auto maxVal = std::ranges::max(arr);
            const auto n      = static_cast<std::int32_t>(arr.size());

            std::vector<std::int32_t> output(static_cast<std::size_t>(n));
            std::array<std::int32_t, 10> count{};

            for (auto exp = 1; maxVal / exp > 0; exp *= 10)
            {
                std::ranges::fill(count, 0);

                for (auto i = 0; i < n; ++i)
                {
                    count[static_cast<std::size_t>((arr[static_cast<std::size_t>(i)] / exp) % 10)]++;
                    ++m.arrayAccesses;
                }

                for (auto i = 1; i < 10; ++i)
                {
                    count[static_cast<std::size_t>(i)] += count[static_cast<std::size_t>(i - 1)];
                }

                for (auto i = n - 1; i >= 0; --i)
                {
                    auto digit = (arr[static_cast<std::size_t>(i)] / exp) % 10;
                    output[static_cast<std::size_t>(--count[static_cast<std::size_t>(digit)])] = arr[static_cast<std::size_t>(i)];
                    ++m.arrayAccesses;
                }

                for (auto i = 0; i < n; ++i)
                {
                    arr[static_cast<std::size_t>(i)] = output[static_cast<std::size_t>(i)];
                    ++m.swaps;
                    ++m.arrayAccesses;

                    if constexpr (EnableVisuals) {
                        std::string stelle = (exp == 1)   ? "Einerstelle"
                                           : (exp == 10)  ? "Zehnerstelle"
                                           : (exp == 100) ? "Hunderterstelle"
                                           :                std::format("{}er-Stelle", exp);

                        cb(arr, i, -1, std::format(
                            "Sortiere nach {}: {} hat Ziffer {} und kommt an Position {}",
                            stelle, arr[static_cast<std::size_t>(i)], (arr[static_cast<std::size_t>(i)] / exp) % 10, i));
                    }
                }
            }
        }
    }

    // ============================================================
    // radixSort – Öffentliche Schnittstelle
    // ============================================================
    void radixSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        if (cb) {
            radixSortImpl<true>(arr, cb, m);
        } else {
            radixSortImpl<false>(arr, cb, m);
        }
    }

} // namespace Algorithms