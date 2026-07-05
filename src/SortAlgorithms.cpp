#include "SortAlgorithms.hpp"
#include "Algorithms/QuickSort.hpp"
#include "Algorithms/MergeSortRec.hpp"
#include "Algorithms/MergeSortIt.hpp"
#include "Algorithms/HeapSort.hpp"
#include "Algorithms/RadixSort.hpp"
#include "Algorithms/CountingSort.hpp"
#include "Algorithms/BubbleSort.hpp"
#include <format>
#include <array>
#include <stdexcept>
#include <cstdint>

// Namensraum zur Kapselung der algorithmischen Schnittstellen
namespace SortAlgorithms
{
    // Anonymer Namespace für interne Verlinkung (Internal Linkage)
    namespace
    {
        // const statt constexpr, da std::string (intern in AlgoInfo) Heap-Speicher
        // verwendet, der nicht zur Kompilierzeit ausgewertet werden kann.
        const std::array<AlgoInfo, 7> infoTable
        {{
            {"QuickSort", "O(n log n)", "O(n log n)", "O(n^2)", "O(log n)", "Nein", "Sehr gut - schnellster Algo in der Praxis", "Sehr gut - Cache-freundlich (in-place)"},
            {"MergeSort (rekursiv)", "O(n log n)", "O(n log n)", "O(n log n)", "O(n)", "Ja", "Gut - garantiert O(n log n), stabil", "Mittel - extra Speicher, viele Kopier-Ops"},
            {"MergeSort (iterativ)", "O(n log n)", "O(n log n)", "O(n log n)", "O(n)", "Ja", "Gut - kein Rekursions-Overhead, stabil", "Mittel - extra Speicher, kein Stack-Overhead"},
            {"HeapSort", "O(n log n)", "O(n log n)", "O(n log n)", "O(1)", "Nein", "Gut - garantiert O(n log n), in-place", "Schlecht - Cache-unfreundlich, Sprungzugriffe"},
            {"RadixSort", "O(nk)", "O(nk)", "O(nk)", "O(n+k)", "Ja", "Sehr gut bei ganzen Zahlen, bekannter Range", "Mittel - linearer Durchlauf, extra Speicher"},
            {"CountingSort", "O(n+k)", "O(n+k)", "O(n+k)", "O(k)", "Ja", "Optimal bei kleinem Wertebereich k", "Gut - linearer Zugriff, k-großes Count-Array"},
            {"BubbleSort", "O(n)", "O(n^2)", "O(n^2)", "O(1)", "Ja", "Schlecht - extrem langsam bei großen Arrays", "Sehr gut - rein sequenzieller Speicherzugriff"}
        }};
    } // namespace

    // Abrufen der Algorithmisierungs-Informationen mit typsicherer Bereichsprüfung
    // [[nodiscard]] erzwingt die Auswertung des Rückgabewerts.
    [[nodiscard]] AlgoInfo getInfo(const std::uint8_t idx)
    {
        // Sicherer Zugriff ohne Pointer-Dereferenzierung. Löst Exception bei Bereichsüberschreitung aus.
        if (idx >= infoTable.size())
        {
            throw std::out_of_range(std::format("Unbekannter Algorithmus-Index: {}", idx));
        }
        return infoTable.at(idx);
    }

    // Die Dispatcher-Funktionen – leiten die linearen Vektorstrukturen an den Sub-Namespace weiter.
    // Die Definitionen stimmen nun exakt mit den Spezifikationen in der Header-Datei überein.

    void quickSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        Algorithms::quickSort(arr, cb, m);
    }

    void mergeSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        Algorithms::mergeSortRec(arr, cb, m);
    }

    void mergeSortIt(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        Algorithms::mergeSortIt(arr, cb, m);
    }

    void heapSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        Algorithms::heapSort(arr, cb, m);
    }

    void radixSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        Algorithms::radixSort(arr, cb, m);
    }

    void countingSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        Algorithms::countingSort(arr, cb, m);
    }

    void bubbleSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        Algorithms::bubbleSort(arr, cb, m);
    }

} // namespace SortAlgorithms