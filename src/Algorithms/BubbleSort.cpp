#include "SortAlgorithms.hpp"
#include <cstdint>
#include <utility>
#include <vector>

// ==================================================================
// Bubblesort (optimiert: schrumpfende Grenze + Früh-Abbruch)
// ==================================================================
//
// Hinweis: Bubblesort wird im begleitenden Skript "Algorithmen und
// Datenstrukturen" nicht behandelt (dort: Selection-, Insertion-,
// Quick-, Merge-, Heap-, Counting-, Radix- und Bucketsort). Die
// folgende Implementierung folgt daher der Standard-Lehrbuch-Logik
// (z.B. Cormen et al., "Introduction to Algorithms").
//
// Grundidee: Wiederholt benachbarte Elemente vergleichen und bei
// Bedarf vertauschen. Nach jedem Durchlauf "blubbert" das jeweils
// größte noch unsortierte Element ans Ende des betrachteten Bereichs
// (daher der Name).
//
// Zwei Optimierungen gegenüber der naiven Variante:
//   1. Schrumpfende Grenze ("bound"): Nach einem Durchlauf ist alles
//      AB der Position des letzten Swaps garantiert schon endgültig
//      einsortiert - der nächste Durchlauf muss also nicht mehr bis
//      ans Arrayende laufen, sondern nur noch bis dorthin.
//   2. Früh-Abbruch: Findet ein kompletter Durchlauf keinen einzigen
//      Swap mehr, ist das Array bereits vollständig sortiert -
//      weitere Durchläufe wären reine Zeitverschwendung.

namespace SortAlgorithms
{
    // --------------------------------------------------------
    // bubbleSortImpl – Kernlogik als Template
    // --------------------------------------------------------
    template <bool EnableVisuals>
    void bubbleSortImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        std::size_t bound = arr.size(); // oberer Rand des noch unsortierten Bereichs

        // Äußere Schleife läuft, solange im letzten Durchlauf noch getauscht wurde.
        while (bound > 1)
        {
            std::size_t lastSwapIdx = 0; // Position des letzten Swaps in diesem Durchlauf

            // Innere Schleife: Hier verbringt die CPU 99 % der Zeit.
            for (std::size_t j = 0; j + 1 < bound; ++j)
            {
                ++m.comparisons;
                m.arrayAccesses += 2; // liest arr[j] und arr[j+1]

                if constexpr (EnableVisuals) {
                    cb(arr, static_cast<std::int32_t>(j), static_cast<std::int32_t>(j + 1));
                }

                if (arr[j] > arr[j + 1])
                {
                    std::swap(arr[j], arr[j + 1]);
                    ++m.swaps;
                    m.arrayAccesses += 4; // std::swap: je 1 Lese- + 1 Schreibzugriff pro Element

                    if constexpr (EnableVisuals) {
                        cb(arr, static_cast<std::int32_t>(j), static_cast<std::int32_t>(j + 1));
                    }
                    lastSwapIdx = j + 1;
                }
            }

            // Kein Swap passiert -> Array ist bereits vollständig sortiert.
            if (lastSwapIdx == 0)
            {
                if constexpr (EnableVisuals) {
                    cb(arr, -1, -1);
                }
                break;
            }

            // Alles ab lastSwapIdx ist garantiert schon an der richtigen Stelle,
            // der nächste Durchlauf muss nur noch bis dorthin prüfen.
            bound = lastSwapIdx;
        }
    }

    // --------------------------------------------------------
    // bubbleSort – öffentliche Schnittstelle
    // --------------------------------------------------------
    void bubbleSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        // Compile-Zeit-Entscheidung wie bei den übrigen Algorithmen:
        // Mit Callback -> Visualisierungsschritte, ohne Callback
        // (reiner Benchmark) -> kein Laufzeit-Overhead durch if constexpr.
        if (cb) {
            bubbleSortImpl<true>(arr, cb, m);
        } else {
            bubbleSortImpl<false>(arr, cb, m);
        }
    }

} // namespace SortAlgorithms