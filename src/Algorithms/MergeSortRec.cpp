
#include "SortAlgorithms.hpp"
#include <span>
#include <vector>
#include <cstdint>
#include <algorithm>

namespace SortAlgorithms
{
    // Hilfsfunktionen direkt hier, ohne anonymen Namespace,
    // damit sie im gesamten Linker-Kontext sauber auflösbar sind.

    template <bool EnableVisuals>
    void mergeImpl(std::vector<std::int32_t>& arr,
                   std::span<std::int32_t>    bufferView,
                   std::size_t                left,
                   std::size_t                mid,
                   std::size_t                right,
                   const StepCallback&        cb,
                   LiveMetrics&               m)
    {
        const std::size_t length = right - left + 1;
        std::copy(arr.begin() + static_cast<std::ptrdiff_t>(left),
                  arr.begin() + static_cast<std::ptrdiff_t>(right) + 1,
                  bufferView.begin() + static_cast<std::ptrdiff_t>(left));
        m.arrayAccesses += static_cast<std::int64_t>(length) * 2;

        std::size_t i = left;
        std::size_t j = mid + 1;
        std::size_t k = left;

        while (i <= mid && j <= right)
        {
            ++m.comparisons;
            if (bufferView[i] <= bufferView[j])
            {
                arr[static_cast<std::size_t>(k)] = bufferView[i++];
                ++m.arrayAccesses;
                if constexpr (EnableVisuals) cb(arr, static_cast<std::int32_t>(k), static_cast<std::int32_t>(i - 1));
            }
            else
            {
                arr[static_cast<std::size_t>(k)] = bufferView[j++];
                ++m.arrayAccesses;
                if constexpr (EnableVisuals) cb(arr, static_cast<std::int32_t>(k), static_cast<std::int32_t>(j - 1));
            }
            ++k;
        }

        while (i <= mid)
        {
            arr[static_cast<std::size_t>(k++)] = bufferView[i++];
            ++m.arrayAccesses;
        }
        while (j <= right)
        {
            arr[static_cast<std::size_t>(k++)] = bufferView[j++];
            ++m.arrayAccesses;
        }
    }

    template <bool EnableVisuals>
    void mergeSortRange(std::vector<std::int32_t>& arr,
                        std::span<std::int32_t>    fullBufferView,
                        std::span<std::int32_t>    subArrayView,
                        const StepCallback&        cb,
                        LiveMetrics&               m)
    {
        if (subArrayView.size() <= 1) return;

        const std::size_t midOffset = subArrayView.size() / 2;
        std::span<std::int32_t> leftSubView  = subArrayView.first(midOffset);
        std::span<std::int32_t> rightSubView = subArrayView.last(subArrayView.size() - midOffset);

        mergeSortRange<EnableVisuals>(arr, fullBufferView, leftSubView, cb, m);
        mergeSortRange<EnableVisuals>(arr, fullBufferView, rightSubView, cb, m);

        const std::size_t absStartIdx = static_cast<std::size_t>(subArrayView.data() - arr.data());
        mergeImpl<EnableVisuals>(arr, fullBufferView, absStartIdx, absStartIdx + midOffset - 1, absStartIdx + subArrayView.size() - 1, cb, m);
    }

    // WICHTIG: Hier muss "mergeSort" stehen, wie im Header deklariert!
    void mergeSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.size() < 2) return;

        std::vector<std::int32_t> buffer(arr.size());
        std::span<std::int32_t>   bufferView(buffer);
        std::span<std::int32_t>   mainView(arr);

        if (cb) mergeSortRange<true>(arr, bufferView, mainView, cb, m);
        else    mergeSortRange<false>(arr, bufferView, mainView, cb, m);
    }

} // namespace SortAlgorithms