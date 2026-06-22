// SortAlgorithms.hpp
// Zentrale Typen, Structs und Dispatcher-Deklarationen

#pragma once

#include <cstdint>      // std::int32_t, std::uint64_t
#include <functional>   // std::function
#include <string>       // std::string
#include <vector>       // std::vector


// SortStep – Snapshot eines Sortierschritts (ohne Text-Aktion)

struct SortStep
{
    std::vector<std::int32_t> array;
    std::int32_t              indexA{-1};
    std::int32_t              indexB{-1};
};


// AlgoInfo – Theoretische Eigenschaften eines Algorithmus

struct AlgoInfo
{
    std::string name;
    std::string bestCase;
    std::string avgCase;
    std::string worstCase;
    std::string space;
    std::string stable;
    std::string quality;
    std::string communication;
};


// LiveMetrics – Live gemessene Werte während der Sortierung

struct LiveMetrics
{
    std::uint64_t comparisons   {0};
    std::uint64_t swaps         {0};
    std::uint64_t arrayAccesses {0};
    double        elapsedMs     {0.0};
};


// StepCallback – Callback-Typ für Sortierschritte (ohne Action-String)
using StepCallback = std::function<void(
    const std::vector<std::int32_t>& arr,
    std::int32_t                     a,
    std::int32_t                     b
)>;

namespace SortAlgorithms
{
    [[nodiscard]] AlgoInfo getInfo(std::uint8_t algorithmIndex);
    // Anpassung: Übergabe per const-Referenz für exaktes Signature-Matching
    void quickSort   (std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
    void mergeSort   (std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
    void mergeSortIt (std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
    void heapSort    (std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
    void radixSort   (std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
    void countingSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
    void bubbleSort  (std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m);
}