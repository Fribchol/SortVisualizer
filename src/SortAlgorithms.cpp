// ============================================================
// SortAlgorithms.cpp
// Dispatcher + AlgoInfo Tabelle
//
// C++20 Features:
// ┌──────────────────┬─────────────────────────────────────────┐
// │ std::array       │ Compile-Zeit Größe, kein Heap-Alloc     │
// │ std::move        │ Callback ohne Kopie weitergeben         │
// │ static const     │ Tabelle wird nur einmal angelegt        │
// │ at()             │ sicherer Zugriff mit Bounds-Check       │
// └──────────────────┴─────────────────────────────────────────┘
// ============================================================
#include "SortAlgorithms.hpp"
#include "algorithms/QuickSort.hpp"
#include "algorithms/MergeSortRec.hpp"
#include "algorithms/MergeSortIt.hpp"
#include "algorithms/HeapSort.hpp"
#include "algorithms/RadixSort.hpp"
#include "algorithms/CountingSort.hpp"
#include "algorithms/BubbleSort.hpp" // NEU: Include für BubbleSort
#include <format>
#include <array>      // std::array: Größe zur Compile-Zeit bekannt
#include <stdexcept>  // std::out_of_range

namespace SortAlgorithms
{
    // ============================================================
    // getInfo – Theoretische Infos pro Algorithmus
    // ============================================================
    [[nodiscard]] AlgoInfo getInfo(uint8_t idx)
    {
        static const std::array<AlgoInfo, 7> table
        {{
            {   // 0: QuickSort
                "QuickSort",
                "O(n log n)", "O(n log n)", "O(n^2)",
                "O(log n)", "Nein",
                "Sehr gut - schnellster Algo in der Praxis",
                "Sehr gut - Cache-freundlich (in-place)"
            },
            {   // 1: MergeSort rekursiv
                "MergeSort (rekursiv)",
                "O(n log n)", "O(n log n)", "O(n log n)",
                "O(n)", "Ja",
                "Gut - garantiert O(n log n), stabil",
                "Mittel - extra Speicher, viele Kopierops"
            },
            {   // 2: MergeSort iterativ
                "MergeSort (iterativ)",
                "O(n log n)", "O(n log n)", "O(n log n)",
                "O(n)", "Ja",
                "Gut - kein Rekursions-Overhead, stabil",
                "Mittel - extra Speicher, kein Stack-Overhead"
            },
            {   // 3: HeapSort
                "HeapSort",
                "O(n log n)", "O(n log n)", "O(n log n)",
                "O(1)", "Nein",
                "Gut - garantiert O(n log n), in-place",
                "Schlecht - Cache-unfreundlich, Sprungzugriffe"
            },
            {   // 4: RadixSort
                "RadixSort",
                "O(nk)", "O(nk)", "O(nk)",
                "O(n+k)", "Ja",
                "Sehr gut bei ganzen Zahlen, bekannter Range",
                "Mittel - linearer Durchlauf, extra Speicher"
            },
            {   // 5: CountingSort
                "CountingSort",
                "O(n+k)", "O(n+k)", "O(n+k)",
                "O(k)", "Ja",
                "Optimal bei kleinem Wertebereich k",
                "Gut - linearer Zugriff, k-grosses Count-Array"
            },
            {   // 6: BubbleSort
                "BubbleSort",
                "O(n)", "O(n^2)", "O(n^2)",
                "O(1)", "Ja",
                "Schlecht - extrem langsam bei grossen Arrays",
                "Sehr gut - rein sequenzieller Speicherzugriff"
            }
        }};

        if (idx >= table.size())
            throw std::out_of_range(
                std::format("Unbekannter Algorithmus-Index: {}", idx));

        return table[idx];
    }

    // ============================================================
    // Dispatcher – delegiert an einzelne Algorithmus-Dateien
    // ============================================================
    void quickSort(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m)
    { Algorithms::quickSort(arr, std::move(cb), m); }

    void mergeSort(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m)
    { Algorithms::mergeSortRec(arr, std::move(cb), m); }

    void mergeSortIt(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m)
    { Algorithms::mergeSortIt(arr, std::move(cb), m); }

    void heapSort(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m)
    { Algorithms::heapSort(arr, std::move(cb), m); }

    void radixSort(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m)
    { Algorithms::radixSort(arr, std::move(cb), m); }

    void countingSort(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m)
    { Algorithms::countingSort(arr, std::move(cb), m); }

    // NEU: Dispatcher leitet BubbleSort an die neue Datei weiter
    void bubbleSort(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m)
    { Algorithms::bubbleSort(arr, std::move(cb), m); }
}