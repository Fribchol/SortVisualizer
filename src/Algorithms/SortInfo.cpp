#include "SortAlgorithms.hpp"

namespace SortAlgorithms
{
    AlgoInfo getInfo(std::uint8_t algorithmIndex)
    {
        // Hier definieren wir die Eigenschaften für deine 7 Algorithmen
        switch (algorithmIndex) {
            case 0: return {"QuickSort", "O(n log n)", "O(n log n)", "O(n^2)", "O(log n)", "Nein", "Sehr schnell", "Lomuto Partition"};
            case 1: return {"MergeSort (rek)", "O(n log n)", "O(n log n)", "O(n log n)", "O(n)", "Ja", "Stabil", "Teile und Herrsche"};
            case 2: return {"MergeSort (it)", "O(n log n)", "O(n log n)", "O(n log n)", "O(n)", "Ja", "Stabil", "Bottom-Up"};
            case 3: return {"HeapSort", "O(n log n)", "O(n log n)", "O(n log n)", "O(1)", "Nein", "Sehr effizient", "Binär-Heap"};
            case 4: return {"RadixSort", "O(nk)", "O(nk)", "O(nk)", "O(n+k)", "Ja", "Nicht-vergleichend", "Digit-basiert"};
            case 5: return {"CountingSort", "O(n+k)", "O(n+k)", "O(n+k)", "O(k)", "Ja", "Linear", "Frequenz-Array"};
            case 6: return {"BubbleSort", "O(n)", "O(n^2)", "O(n^2)", "O(1)", "Ja", "Sehr langsam", "Austausch-basiert"};
            default: return {"Unbekannt", "-", "-", "-", "-", "-", "-", "-"};
        }
    }
}