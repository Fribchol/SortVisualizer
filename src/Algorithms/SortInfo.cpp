#include "SortAlgorithms.hpp"
#include <array>
#include <cstdint>

namespace SortAlgorithms
{
    namespace
    {
        // Die eigentliche Datentabelle. Reihenfolge der Zeilen =
        // Algorithmus-Index (0 = QuickSort, 1 = MergeSort rek., ...).
        inline const std::array<AlgoInfo, 7> kAlgorithmTable{ {
            {"QuickSort",       "O(n log n)", "O(n log n)", "O(n^2)",     "O(log n)", "Nein", "Sehr schnell",       "Lomuto Partition"},
            {"MergeSort (rek)", "O(n log n)", "O(n log n)", "O(n log n)", "O(n)",     "Ja",   "Stabil",             "Teile und Herrsche"},
            {"MergeSort (it)",  "O(n log n)", "O(n log n)", "O(n log n)", "O(n)",     "Ja",   "Stabil",             "Bottom-Up"},
            {"HeapSort",        "O(n log n)", "O(n log n)", "O(n log n)", "O(1)",     "Nein", "Sehr effizient",     "Binär-Heap"},
            {"RadixSort",       "O(nk)",      "O(nk)",      "O(nk)",      "O(n+k)",   "Ja",   "Nicht-vergleichend", "Digit-basiert"},
            {"CountingSort",    "O(n+k)",     "O(n+k)",     "O(n+k)",     "O(k)",     "Ja",   "Linear",             "Frequenz-Array"},
            {"BubbleSort",      "O(n)",       "O(n^2)",     "O(n^2)",     "O(1)",     "Ja",   "Sehr langsam",       "Austausch-basiert"},
        } };

        // Rückgabewert für ungültige/unbekannte Indizes
        inline const AlgoInfo kUnknownAlgorithm{"Unbekannt", "-", "-", "-", "-", "-", "-", "-"};
    }


    // getInfo – öffentliche Schnittstelle
    AlgoInfo getInfo(std::uint8_t algorithmIndex)
    {
        if (algorithmIndex >= kAlgorithmTable.size()) {
            return kUnknownAlgorithm;
        }
        return kAlgorithmTable[algorithmIndex];
    }

}