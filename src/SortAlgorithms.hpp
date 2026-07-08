#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

struct SortStep {
    std::vector<std::int32_t> array;
    std::int32_t              indexA{-1};
    std::int32_t              indexB{-1};
};

namespace SortAlgorithms
{
    // AlgoInfo jetzt INNERHALB des Namespaces
    struct AlgoInfo {
        std::string name;
        std::string bestCase;
        std::string avgCase;
        std::string worstCase;
        std::string space;
        std::string stable;
        std::string quality;
        std::string communication;
    };

    // LiveMetrics jetzt INNERHALB des Namespaces
    struct LiveMetrics {
        std::uint64_t comparisons   {0};
        std::uint64_t swaps         {0};
        std::uint64_t arrayAccesses {0};
        double        elapsedMs     {0.0};
    };

    using StepCallback = std::function<void(const std::vector<std::int32_t>&, std::int32_t, std::int32_t)>;

    [[nodiscard]] AlgoInfo getInfo(std::uint8_t algorithmIndex);

    void quickSort   (std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
    void mergeSort   (std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
    void mergeSortIt (std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
    void heapSort    (std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
    void radixSort   (std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
    void countingSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
    void bubbleSort  (std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
}