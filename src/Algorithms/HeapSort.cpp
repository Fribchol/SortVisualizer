// HeapSort.cpp – in-place Max-Heap

#include "SortAlgorithms.hpp"
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

// ==================================================================
// Heapsort (in-place, Max-Heap)
// ==================================================================
//
// Siehe Skript "Algorithmen und Datenstrukturen", Kap. 4.4,
// Heapsort (Folien 237-248):
//
//   Heap-Eigenschaft (Max-Heap): Für eine als Array gespeicherte
//   Folge k1, ..., kn gilt ki >= k(2i) und ki >= k(2i+1), sofern die
//   jeweiligen Kindindizes noch im Array liegen. Das Maximum steht
//   also immer an Position 1 (bzw. Index 0 in 0-basierter Zählung).
//
//   Ablauf (zwei Phasen, siehe Folien 238-244):
//     1. Heap aufbauen: Lasse die Schlüssel k(n/2), ..., k1
//        "versickern" - dadurch werden schrittweise immer größere
//        Teilheaps aufgebaut, bis das gesamte Array die
//        Heap-Eigenschaft erfüllt. Das geht in O(n), nicht in
//        O(n log n) - die meisten Knoten stehen weit unten im Baum
//        und müssen daher nur wenige Ebenen versickern (Analyse
//        Folien 245-247).
//     2. Sortieren: Wiederhole n-1 mal: Vertausche die Wurzel
//        (Maximum) mit dem letzten noch nicht sortierten Element,
//        verkleinere den Heap-Bereich um 1 und lasse die neue
//        Wurzel wieder versickern.
//
//   "Versickern" heißt: der Schlüssel wird immer mit dem größten
//   seiner Nachfolger vertauscht, bis entweder beide Nachfolger
//   kleiner sind oder er unten angekommen ist. Das entspricht der
//   Funktion "heapify" unten - hier bewusst ITERATIV umgesetzt
//   (while-Schleife statt Rekursion), um unnötigen Rekursions-
//   Stack-Overhead zu vermeiden.
//
//   Eigenschaften laut Skript (Folie 248):
//     - Laufzeit O(n log n), auch im schlechtesten Fall.
//     - In-situ: benötigt nur konstant viel zusätzlichen Speicher.
//     - Nicht cache-effizient (viele Sprünge zwischen weit
//       entfernten Array-Positionen -> viele Cache-Misses).
//     - Eine Vorsortierung der Eingabe hilft nicht und schadet nicht.

namespace SortAlgorithms
{
    namespace
    {
        // In einen anonymen Namespace verschoben (Konsistenz mit
        // QuickSort.cpp): heapify und heapSortImpl sind reine
        // Implementierungsdetails und sollen keine externe Bindung
        // haben.

        // --------------------------------------------------------
        // heapify – lässt das Element an Index "i" versickern
        // --------------------------------------------------------
        // Voraussetzung: Beide Teilbäume unter "i" erfüllen bereits die
        // Heap-Eigenschaft, nur "i" selbst könnte zu klein sein. Die
        // Funktion vertauscht "i" so lange mit seinem größten Kind, bis
        // die Heap-Eigenschaft an dieser Stelle wiederhergestellt ist.
        // "n" ist die aktuell betrachtete Heap-Größe (kann kleiner sein
        // als arr.size(), da bereits sortierte Elemente am Ende des
        // Arrays aus dem Heap-Bereich herausfallen).
        template <bool EnableVisuals>
        void heapify(std::vector<std::int32_t>& arr,
                     std::int32_t               n,
                     std::int32_t               i,
                     const StepCallback&        cb,
                     LiveMetrics&               m)
        {
            auto current = i;

            while (true)
            {
                auto largest     = current;
                const auto left  = 2 * current + 1;
                const auto right = 2 * current + 2;

                // Linkes Kind vorhanden? Dann findet gleich ein
                // Wertevergleich statt, der ZWEI Positionen liest:
                // arr[left] und arr[largest].
                if (left < n)
                {
                    ++m.comparisons;
                    m.arrayAccesses += 2;
                    if (arr[static_cast<std::size_t>(left)] > arr[static_cast<std::size_t>(largest)])
                        largest = left;
                }

                // Rechtes Kind vorhanden? Analog: liest arr[right] und
                // arr[largest] (largest kann durch den Schritt oben
                // bereits auf "left" zeigen).
                if (right < n)
                {
                    ++m.comparisons;
                    m.arrayAccesses += 2;
                    if (arr[static_cast<std::size_t>(right)] > arr[static_cast<std::size_t>(largest)])
                        largest = right;
                }

                if (largest != current)
                {
                    std::swap(arr[static_cast<std::size_t>(current)], arr[static_cast<std::size_t>(largest)]);
                    ++m.swaps;
                    m.arrayAccesses += 2;

                    if constexpr (EnableVisuals) cb(arr, current, largest);

                    // Iterativ eine Ebene tiefer weiter versickern,
                    // statt heapify(arr, n, largest, ...) rekursiv
                    // aufzurufen.
                    current = largest;
                }
                else
                {
                    // Beide Kinder sind kleiner (oder nicht vorhanden) -
                    // der Schlüssel ist "unten angekommen", fertig.
                    break;
                }
            }
        }

        // --------------------------------------------------------
        // heapSortImpl – Kernlogik: Heap aufbauen, dann sortieren
        // --------------------------------------------------------
        template <bool EnableVisuals>
        void heapSortImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
        {
            const auto n = static_cast<std::int32_t>(arr.size());

            // Phase 1 (Folien 243-244): Heap aufbauen. Alle Knoten mit
            // mindestens einem Kind stehen im Indexbereich [0, n/2-1];
            // Blätter (Indizes >= n/2) sind bereits triviale Ein-Element-
            // Heaps und müssen nicht versickert werden.
            for (auto i = n / 2 - 1; i >= 0; --i)
            {
                heapify<EnableVisuals>(arr, n, i, cb, m);
            }

            // Phase 2 (Folien 239-242): Wiederholt das Maximum (Wurzel)
            // mit dem letzten Element des noch aktiven Heap-Bereichs
            // vertauschen, den Heap-Bereich um 1 verkleinern (i sinkt)
            // und die neue Wurzel wieder versickern lassen.
            for (auto i = n - 1; i > 0; --i)
            {
                std::swap(arr[0], arr[static_cast<std::size_t>(i)]);
                ++m.swaps;
                m.arrayAccesses += 2;

                if constexpr (EnableVisuals) cb(arr, 0, i);

                // Heap-Größe ist jetzt "i" (das Element an Position i
                // steht bereits final sortiert und wird nicht mehr
                // betrachtet).
                heapify<EnableVisuals>(arr, i, 0, cb, m);
            }
        }
    } // namespace

    // --------------------------------------------------------
    // heapSort – öffentliche Schnittstelle
    // --------------------------------------------------------
    void heapSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        // Compile-Zeit-Entscheidung wie bei den übrigen Algorithmen:
        // Mit Callback -> Visualisierungsschritte, ohne Callback
        // (reiner Benchmark) -> kein Laufzeit-Overhead durch if constexpr.
        if (cb) {
            heapSortImpl<true>(arr, cb, m);
        } else {
            heapSortImpl<false>(arr, cb, m);
        }
    }

} // namespace SortAlgorithms