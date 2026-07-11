#include <algorithm>
#include <cstdint>
#include <utility>
#include "SortAlgorithms.hpp"

// ==================================================================
// QuickSort (Lomuto-Partitionierung mit Median-of-Three-Pivotwahl)
// ==================================================================
//
// Hinweis zum gewählten Verfahren:
// Im Skript "Algorithmen und Datenstrukturen" (Kap. 4.3) wird die
// klassische Hoare-Partitionierung mit zwei laufenden Zeigern gezeigt,
// bei der als Pivot einfach das erste Element gewählt wird. Das hat
// einen bekannten Schwachpunkt: Bei bereits sortierten oder umgekehrt
// sortierten Arrays entartet Quicksort dann zu O(n^2) - und genau
// diese Spezialfälle ("Aufsteigend"/"Absteigend") bietet unser
// Visualizer als Auswahl an!
//
// Deshalb verwenden wir hier stattdessen:
//   - Lomuto-Partitionierung: nur EIN laufender Zeiger, dafür etwas
//     einfacher nachzuvollziehen als Hoare.
//   - Median-of-Three: Statt einfach das erste Element als Pivot zu
//     nehmen, wird aus dem ersten, mittleren und letzten Element der
//     Median bestimmt. Das verhindert zuverlässig den O(n^2)-Fall bei
//     vor- oder rücksortierten Arrays.
//
// Beide Varianten sind im Skript als "Quicksort" anerkannt (das
// Skript nennt selbst "randomisiertes Quicksort" als Alternative zur
// festen Pivot-Wahl, Median-of-Three ist eine deterministische
// Variante desselben Grundgedankens: den Pivot klüger wählen).

namespace SortAlgorithms
{
    namespace
    {
        // --------------------------------------------------------
        // medianOfThreeIndex – Pivot-Auswahl
        // --------------------------------------------------------
        // Schaut sich das erste (low), mittlere (mid) und letzte (high)
        // Element des aktuellen Teilbereichs an, bringt diese drei
        // Werte in die richtige Reihenfolge zueinander und verschiebt
        // den mittleren der drei Werte (den "Median") an die Position
        // 'high'. Von dort übernimmt ihn anschließend partition() als
        // Pivot-Element.
        //
        // EnableVisuals ist ein Compile-Zeit-Schalter (Template-Parameter):
        // Ist er "true", wird nach jedem Schritt der Callback cb()
        // aufgerufen, damit der Visualizer den Schritt anzeigen kann.
        // Ist er "false" (z.B. beim Shell-Benchmark ohne Anzeige),
        // entfällt dieser Aufruf komplett zur Laufzeit - dank
        // "if constexpr" erzeugt der Compiler dafür gar keinen Code.
        template <bool EnableVisuals>
        void medianOfThreeIndex(std::vector<std::int32_t>& arr,
                                 std::size_t                low,
                                 std::size_t                high,
                                 const StepCallback&        cb,
                                 LiveMetrics&                m)
        {
            const std::size_t mid = low + (high - low) / 2;

            // Die drei Werte low/mid/high paarweise vergleichen und bei
            // Bedarf tauschen, bis arr[low] <= arr[mid] <= arr[high] gilt.
            // Jeder Vergleich UND jeder eventuelle Tausch wird für die
            // Statistik mitgezählt (vorher fehlte das Zählen der
            // Vergleiche - dadurch waren die Live-Metriken zu niedrig).
            ++m.comparisons;
            m.arrayAccesses += 2;
            if (arr[low] > arr[mid]) {
                std::swap(arr[low], arr[mid]);
                ++m.swaps;
                m.arrayAccesses += 2;
            }

            ++m.comparisons;
            m.arrayAccesses += 2;
            if (arr[low] > arr[high]) {
                std::swap(arr[low], arr[high]);
                ++m.swaps;
                m.arrayAccesses += 2;
            }

            ++m.comparisons;
            m.arrayAccesses += 2;
            if (arr[mid] > arr[high]) {
                std::swap(arr[mid], arr[high]);
                ++m.swaps;
                m.arrayAccesses += 2;
            }

            // Der Median der drei Werte steht jetzt in arr[mid].
            // Wir verschieben ihn nach 'high', damit die anschließende
            // Lomuto-Partition (die den Pivot immer am rechten Rand
            // erwartet) unverändert funktioniert.
            std::swap(arr[mid], arr[high]);
            ++m.swaps;
            m.arrayAccesses += 2;

            if constexpr (EnableVisuals) {
                cb(arr, static_cast<std::int32_t>(mid), static_cast<std::int32_t>(high));
            }
        }

        // --------------------------------------------------------
        // partition – Lomuto-Partition
        // --------------------------------------------------------
        // Teilt den Bereich [low, high] in zwei Hälften auf:
        //   links vom Rückgabewert: alle Werte <= Pivot
        //   rechts vom Rückgabewert: alle Werte > Pivot
        // Das Pivot-Element selbst landet dabei genau an seiner
        // endgültigen, sortierten Position.
        template <bool EnableVisuals>
        [[nodiscard]] std::size_t partition(std::vector<std::int32_t>& arr,
                                             std::size_t                low,
                                             std::size_t                high,
                                             const StepCallback&        cb,
                                             LiveMetrics&                m)
        {
            // Pivot-Element per Median-of-Three bestimmen; es liegt
            // danach an Position 'high'.
            medianOfThreeIndex<EnableVisuals>(arr, low, high, cb, m);
            const std::int32_t pivot = arr[high];

            // 'i' markiert die Grenze der "kleiner-gleich-Pivot"-Zone.
            // Da diese Zone anfangs leer ist, beginnt 'i' gedanklich
            // eine Position VOR 'low'. Weil std::size_t nicht negativ
            // werden darf, benutzen wir dafür einen vorzeichenbehafteten
            // Typ (ptrdiff_t) - das ist der übliche, sichere Trick.
            auto i = static_cast<std::ptrdiff_t>(low) - 1;

            // Wandert einmal von 'low' bis 'high - 1' durch das Array
            // und schiebt jeden Wert, der <= Pivot ist, in die linke Zone.
            for (std::size_t j = low; j < high; ++j)
            {
                ++m.comparisons;
                ++m.arrayAccesses;

                if constexpr (EnableVisuals) {
                    cb(arr, static_cast<std::int32_t>(j), static_cast<std::int32_t>(high));
                }

                if (arr[j] <= pivot)
                {
                    ++i;
                    std::swap(arr[static_cast<std::size_t>(i)], arr[j]);
                    ++m.swaps;
                    m.arrayAccesses += 2;

                    if constexpr (EnableVisuals) {
                        cb(arr, static_cast<std::int32_t>(i), static_cast<std::int32_t>(j));
                    }
                }
            }

            // Das Pivot-Element von 'high' an seine endgültige Position
            // direkt hinter der "kleiner-gleich"-Zone verschieben.
            const auto pivotFinalIndex = static_cast<std::size_t>(i + 1);
            std::swap(arr[pivotFinalIndex], arr[high]);
            ++m.swaps;
            m.arrayAccesses += 2;

            if constexpr (EnableVisuals) {
                cb(arr, static_cast<std::int32_t>(pivotFinalIndex), static_cast<std::int32_t>(high));
            }

            return pivotFinalIndex;
        }

        // --------------------------------------------------------
        // quickSortRec – der eigentliche rekursive Sortiervorgang
        // --------------------------------------------------------
        // "Divide and Conquer": Das Array wird am Pivot in zwei Teile
        // aufgespalten (Divide), und jeder Teil wird für sich genommen
        // erneut sortiert (Conquer). Ein "Combine"-Schritt entfällt,
        // weil nach der Partitionierung schon alles an der richtigen
        // Stelle liegt.
        template <bool EnableVisuals>
        void quickSortRec(std::vector<std::int32_t>& arr,
                           std::size_t                low,
                           std::size_t                high,
                           const StepCallback&        cb,
                           LiveMetrics&                m)
        {
            // Abbruchbedingung der Rekursion: Ein Teilbereich mit einem
            // oder null Elementen ist automatisch schon sortiert.
            if (low >= high) return;

            const std::size_t pivotIndex = partition<EnableVisuals>(arr, low, high, cb, m);

            // Linke Hälfte sortieren (alles vor dem Pivot).
            // Die Prüfung "pivotIndex > 0" verhindert einen Underflow:
            // std::size_t ist vorzeichenlos, "pivotIndex - 1" würde bei
            // pivotIndex == 0 zu einer riesigen Zahl "umschlagen".
            if (pivotIndex > 0 && low <= pivotIndex - 1) {
                quickSortRec<EnableVisuals>(arr, low, pivotIndex - 1, cb, m);
            }

            // Rechte Hälfte sortieren (alles nach dem Pivot).
            // Wichtig: Hier darf wirklich nur EIN Rekursionsaufruf stehen.
            // Ein zusätzlicher, doppelter Aufruf würde auf jeder
            // Rekursionsebene die Arbeit verdoppeln und aus dem
            // erwarteten O(n log n) eine exponentielle Laufzeit machen.
            if (pivotIndex < high) {
                quickSortRec<EnableVisuals>(arr, pivotIndex + 1, high, cb, m);
            }
        }
    } // namespace

    // --------------------------------------------------------
    // quickSort – öffentliche Schnittstelle
    // --------------------------------------------------------
    // Startet den eigentlichen Sortiervorgang. Über den Callback "cb"
    // entscheiden wir zur Compile-Zeit, ob die Visualisierungsschritte
    // mitgeschickt werden (EnableVisuals = true) oder nicht
    // (EnableVisuals = false, z.B. für den reinen Performance-Benchmark
    // ohne UI). Dadurch entsteht kein Laufzeit-Overhead im Benchmark-Fall.
    void quickSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        // Ein leeres Array ist bereits sortiert - nichts zu tun.
        if (arr.empty()) return;

        constexpr std::size_t startIdx = 0;
        const std::size_t     endIdx   = arr.size() - 1;

        if (cb) {
            quickSortRec<true>(arr, startIdx, endIdx, cb, m);
        } else {
            quickSortRec<false>(arr, startIdx, endIdx, cb, m);
        }
    }

} // namespace SortAlgorithms