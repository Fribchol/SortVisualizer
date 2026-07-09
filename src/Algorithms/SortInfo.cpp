#include "SortAlgorithms.hpp"
#include <array>
#include <cstdint>

// ==================================================================
// Algorithmus-Metadaten als Tabelle (Data-Oriented Design)
// ==================================================================
//
// Statt eines switch/case mit 7 fast identisch aufgebauten
// return-Zweigen pflegen wir GENAU EINE Tabelle: einen
// std::array<AlgoInfo, N>, indiziert über den Algorithmus-Index.
//
// Warum das besseres DOD ist als switch/case:
//   - Eine einzige "Source of Truth": die Reihenfolge der Zeilen
//     entspricht direkt dem Index, kein Case kann vergessen oder
//     versehentlich doppelt belegt werden.
//   - Zusammenhängender Speicherbereich (std::array liegt komplett
//     contiguous), statt einer vom Compiler erzeugten Sprungtabelle
//     mit verstreuten return-Statements.
//   - Die Tabelle wird EINMAL aufgebaut (statische Speicherdauer),
//     nicht bei jedem Aufruf neu konstruiert - anders als beim
//     switch/case, wo bei jedem getInfo()-Aufruf ein frisches
//     AlgoInfo-Objekt (inkl. aller Strings) neu erzeugt wird.
//
// RAII: std::array verwaltet seinen Speicher selbst (automatische
// Speicherdauer bei lokaler Nutzung, hier: statische Speicherdauer
// über "inline const") - kein manuelles new/delete, keine
// Ressourcen, um die man sich kümmern müsste.

namespace SortAlgorithms
{
    namespace
    {
        // Die eigentliche Datentabelle. Reihenfolge der Zeilen =
        // Algorithmus-Index (0 = QuickSort, 1 = MergeSort rek., ...).
        //
        // Hinweis: Sollte AlgoInfo ausschließlich aus string_view-
        // artigen (nicht heap-allozierenden) Feldern bestehen, kann
        // diese Tabelle zusätzlich als "constexpr" statt nur "const"
        // deklariert werden - dann liegt sie komplett zur Compile-
        // Zeit fertig im Programm-Image.
        inline const std::array<AlgoInfo, 7> kAlgorithmTable{ {
            {"QuickSort",       "O(n log n)", "O(n log n)", "O(n^2)",     "O(log n)", "Nein", "Sehr schnell",       "Lomuto Partition"},
            {"MergeSort (rek)", "O(n log n)", "O(n log n)", "O(n log n)", "O(n)",     "Ja",   "Stabil",             "Teile und Herrsche"},
            {"MergeSort (it)",  "O(n log n)", "O(n log n)", "O(n log n)", "O(n)",     "Ja",   "Stabil",             "Bottom-Up"},
            {"HeapSort",        "O(n log n)", "O(n log n)", "O(n log n)", "O(1)",     "Nein", "Sehr effizient",     "Binär-Heap"},
            {"RadixSort",       "O(nk)",      "O(nk)",      "O(nk)",      "O(n+k)",   "Ja",   "Nicht-vergleichend", "Digit-basiert"},
            {"CountingSort",    "O(n+k)",     "O(n+k)",     "O(n+k)",     "O(k)",     "Ja",   "Linear",             "Frequenz-Array"},
            {"BubbleSort",      "O(n)",       "O(n^2)",     "O(n^2)",     "O(1)",     "Ja",   "Sehr langsam",       "Austausch-basiert"},
        } };

        // Rückgabewert für ungültige/unbekannte Indizes - ebenfalls
        // nur einmal angelegt statt bei jedem Fehlaufruf neu gebaut.
        inline const AlgoInfo kUnknownAlgorithm{"Unbekannt", "-", "-", "-", "-", "-", "-", "-"};
    } // namespace

    // --------------------------------------------------------
    // getInfo – öffentliche Schnittstelle
    // --------------------------------------------------------
    // Reiner Tabellen-Lookup mit Bounds-Check statt Verzweigungs-
    // logik. algorithmIndex ist vorzeichenlos (std::uint8_t), daher
    // genügt eine einseitige Prüfung gegen die Tabellengröße.
    AlgoInfo getInfo(std::uint8_t algorithmIndex)
    {
        if (algorithmIndex >= kAlgorithmTable.size()) {
            return kUnknownAlgorithm;
        }
        return kAlgorithmTable[algorithmIndex];
    }

} // namespace SortAlgorithms