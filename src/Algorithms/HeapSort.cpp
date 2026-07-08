// HeapSort.cpp – in-place Max-Heap


#include <algorithm>
#include <cstdint>
#include "SortAlgorithms.hpp"

namespace SortAlgorithms
{
    namespace
    {

        // heapify – Iterativ, um Rekursion zu vermeiden

        template <bool EnableVisuals>
        void heapify(std::vector<std::int32_t>& arr,
                     std::int32_t               n,
                     std::int32_t               i,
                     const StepCallback&        cb,
                     LiveMetrics&               m)
        {
            auto current = i;

            while (true) {
                auto largest = current;
                const auto left = 2 * current + 1;
                const auto right = 2 * current + 2;

                if (left < n) { ++m.comparisons; ++m.arrayAccesses; }
                if (right < n) { ++m.comparisons; ++m.arrayAccesses; }

                if (left < n && arr[static_cast<std::size_t>(left)] > arr[static_cast<std::size_t>(largest)])
                    largest = left;
                if (right < n && arr[static_cast<std::size_t>(right)] > arr[static_cast<std::size_t>(largest)])
                    largest = right;

                if (largest != current) {
                    std::swap(arr[static_cast<std::size_t>(current)], arr[static_cast<std::size_t>(largest)]);
                    ++m.swaps;
                    m.arrayAccesses += 2;

                    if constexpr (EnableVisuals) {
                        cb(arr, current, largest);
                    }

                    // Nächste Iteration statt Rekursion
                    current = largest;
                } else {
                    break;
                }
            }
        }


        // heapSortImpl – Kernlogik

        template <bool EnableVisuals>
        void heapSortImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
        {
            const auto n = static_cast<std::int32_t>(arr.size());

            for (auto i = n / 2 - 1; i >= 0; --i) {
                heapify<EnableVisuals>(arr, n, i, cb, m);
            }

            for (auto i = n - 1; i > 0; --i)
            {
                std::swap(arr[0], arr[static_cast<std::size_t>(i)]);
                ++m.swaps;
                m.arrayAccesses += 2;

                if constexpr (EnableVisuals) {
                    cb(arr, 0, i);
                }

                heapify<EnableVisuals>(arr, i, 0, cb, m);
            }
        }
    }

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