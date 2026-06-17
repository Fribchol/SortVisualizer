// ============================================================
// MergeSortRec.cpp – Top-Down MergeSort, rekursiv
// ============================================================
#include "MergeSortRec.hpp"
#include <algorithm>
#include <cstdint>

namespace Algorithms
{
    namespace
    {
        // ============================================================
        // mergeImpl – Zwei sortierte Hälften zusammenführen
        // ============================================================
        template <bool EnableVisuals>
        void mergeImpl(std::vector<std::int32_t>& arr,
                       std::vector<std::int32_t>& buffer,
                       std::size_t                left,
                       std::size_t                mid,
                       std::size_t                right,
                       const StepCallback&        cb,
                       LiveMetrics&               m)
        {
            std::copy(arr.begin() + static_cast<std::ptrdiff_t>(left),
                      arr.begin() + static_cast<std::ptrdiff_t>(right) + 1,
                      buffer.begin() + static_cast<std::ptrdiff_t>(left));
            m.arrayAccesses += static_cast<std::int64_t>(right - left + 1) * 2;

            auto i = left;
            auto j = mid + 1;
            auto k = left;

            while (i <= mid && j <= right)
            {
                ++m.comparisons;
                if (buffer[i] <= buffer[j])
                {
                    arr[k] = buffer[i++];
                    ++m.arrayAccesses;

                    if constexpr (EnableVisuals) {
                        cb(arr, static_cast<std::int32_t>(k), -1);
                    }
                }
                else
                {
                    arr[k] = buffer[j++];
                    ++m.arrayAccesses;

                    if constexpr (EnableVisuals) {
                        cb(arr, static_cast<std::int32_t>(k), -1);
                    }
                }
                ++k;
            }

            while (i <= mid) {
                arr[k++] = buffer[i++];
                ++m.arrayAccesses;
            }
            while (j <= right) {
                arr[k++] = buffer[j++];
                ++m.arrayAccesses;
            }
        }

        // ============================================================
        // mergeSortRecHelper – Rekursiver Kern als Template
        // ============================================================
        // NOLINTBEGIN(readability-function-cognitive-complexity, bugprone-recursive-recursion, misc-no-recursion)
        template <bool EnableVisuals>
        void mergeSortRecHelper(std::vector<std::int32_t>& arr,
                                std::vector<std::int32_t>& buffer,
                                std::size_t                left,
                                std::size_t                right,
                                const StepCallback&        cb,
                                LiveMetrics&               m)
        {
            if (left >= right) return;

            const auto mid = left + (right - left) / 2;

            mergeSortRecHelper<EnableVisuals>(arr, buffer, left, mid, cb, m);
            mergeSortRecHelper<EnableVisuals>(arr, buffer, mid + 1, right, cb, m);

            mergeImpl<EnableVisuals>(arr, buffer, left, mid, right, cb, m);
        }
        // NOLINTEND(readability-function-cognitive-complexity, bugprone-recursive-recursion, misc-no-recursion)
    }

    // ============================================================
    // mergeSortRec – Öffentliche Schnittstelle
    // ============================================================
    void mergeSortRec(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.size() < 2) return;

        std::vector<std::int32_t> buffer(arr.size());

        // NOLINTNEXTLINE(readability-static-accessed-through-instance)
        if (cb) {
            mergeSortRecHelper<true>(arr, buffer, 0, arr.size() - 1, cb, m);
        } else {
            mergeSortRecHelper<false>(arr, buffer, 0, arr.size() - 1, cb, m);
        }
    }

} // namespace Algorithms