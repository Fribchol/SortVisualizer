#include "BubbleSort.hpp"
#include <format>
#include <utility>

namespace Algorithms
{
    void bubbleSort(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m)
    {
        int32_t n = arr.size();
        bool swapped;
        for (int32_t i = 0; i < n - 1; ++i) {
            swapped = false;
            for (int32_t j = 0; j < n - i - 1; ++j) {
                m.comparisons++;
                m.arrayAccesses += 2;
                cb(arr, j, j + 1, std::format("Vergleiche {} und {}", arr[j], arr[j+1]));
                
                if (arr[j] > arr[j + 1]) {
                    std::swap(arr[j], arr[j + 1]);
                    m.swaps++;
                    m.arrayAccesses += 4;
                    cb(arr, j, j + 1, std::format("Tausche {} und {}", arr[j], arr[j+1]));
                    swapped = true;
                }
            }
            // Frühzeitiger Abbruch, wenn das Array bereits sortiert ist
            if (!swapped) {
                break;
            }
        }
    }
}