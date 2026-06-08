// ============================================================
// QuickSort.cpp – Lomuto Partition, rekursiv
//
// Algorithmus-Erklärung:
// QuickSort wählt ein "Pivot"-Element und partitioniert
// das Array so dass gilt:
//   Alle Elemente LINKS  vom Pivot: <= Pivot
//   Alle Elemente RECHTS vom Pivot: >  Pivot
// Dann wird links und rechts rekursiv sortiert.
//
// Lomuto-Partition (diese Implementierung):
//   Pivot = letztes Element (arr[high])
//   i     = Grenze zwischen "kleiner" und "größer" Bereich
//   j     = aktuell betrachtetes Element
//
// Visualisierung eines Partitionierungsschritts:
//   [3|7|1|5|2|4]  Pivot=4
//    i       j
//   3 <= 4 → links  → tauschen, i++
//   7 >  4 → rechts → kein Tausch
//   1 <= 4 → links  → tauschen, i++
//   5 >  4 → rechts → kein Tausch
//   2 <= 4 → links  → tauschen, i++
//   Pivot (4) an Position i+1 → fertig!
//   [3|1|2|4|7|5]  Pivot 4 ist korrekt platziert!
//
// Vorteil:  Sehr schnell in der Praxis, in-place, Cache-freundlich
// Nachteil: Worst-Case O(n²) bei bereits sortiertem Array
//           Nicht stabil
//
// C++20 Features:
// ┌─────────────────────┬──────────────────────────────────────┐
// │ std::format         │ Typsicheres String-Formatting        │
// │ std::swap           │ Effizientes Tauschen                 │
// │ string_view         │ Kein Kopieren bei Callbacks          │
// │ const int32_t       │ Unveränderliche lokale Variablen     │
// │ static (intern)     │ Funktionen nur in dieser Datei       │
// │ Ternärer Operator   │ Kompakte if-else Ausdrücke           │
// └─────────────────────┴──────────────────────────────────────┘
// ============================================================
#include "QuickSort.hpp"
#include <algorithm>  // std::swap
#include <format>     // std::format (C++20)

namespace Algorithms
{
    // ============================================================
    // step – Hilfsfunktion: Schritt melden
    //
    // ── static (interne Linkage) ──────────────────────────────
    // Nur in dieser .cpp Datei sichtbar.
    // Kein Namenskonflikt mit step() in anderen Algorithmen.
    //
    // ── StepCallback& statt StepCallback ──────────────────────
    // Referenz statt Kopie: std::function kopieren ist teuer.
    // Referenz = O(1), keine Heap-Allokation.
    //
    // ── std::string_view ──────────────────────────────────────
    // Nimmt Strings ohne Kopieren entgegen.
    // Funktioniert mit string, const char*, string-Literal.
    // ============================================================
    static void step(const std::vector<int32_t>& arr,
                     StepCallback&               cb,
                     int32_t                     a,
                     int32_t                     b,
                     std::string_view            action)
    {
        if (cb) cb(arr, a, b, action);
    }

    // ============================================================
    // partition – Lomuto-Partition
    //
    // Teilt arr[low..high] in zwei Bereiche:
    //   [low .. i]   → alle Elemente <= Pivot
    //   [i+1 .. high] → alle Elemente >  Pivot
    // Pivot wird an seine finale Position i+1 gesetzt.
    //
    // ── const int32_t pivot ───────────────────────────────────
    // pivot ändert sich nie → const macht das explizit.
    // Verhindert versehentliches Überschreiben.
    //
    // ── Ternärer Operator im std::format ──────────────────────
    // arr[j] <= pivot ? "kleiner..." : "größer..."
    // Inline-if direkt im Format-String → kompakter Code.
    // ============================================================
    static int32_t partition(std::vector<int32_t>& arr,
                              int32_t               low,
                              int32_t               high,
                              StepCallback&         cb,
                              LiveMetrics&          m)
    {
        // ── Pivot = letztes Element (Lomuto) ──────────────────
        // const: Pivot ändert sich während der Partition nicht.
        const int32_t pivot = arr[high];

        // i = Grenzindex: alles bis i ist <= Pivot
        // Startet bei low-1 (noch kein Element verarbeitet)
        int32_t i = low - 1;

        // ── j läuft durch alle Elemente außer Pivot ───────────
        for (int32_t j = low; j < high; ++j)
        {
            ++m.comparisons;
            ++m.arrayAccesses;

            // ── Vergleich mit Pivot: Erklärung auf Deutsch ────
            // Ternärer Operator: kompakte if-else Auswahl
            step(arr, cb, j, high,
                 std::format(
                     "Vergleich: {} mit Pivot {} – {}",
                     arr[j],
                     pivot,
                     arr[j] <= pivot
                         ? "kleiner oder gleich, kommt nach links"
                         : "grösser, bleibt rechts"));

            if (arr[j] <= pivot)
            {
                // ── Element gehört auf die linke Seite ────────
                // i vorwärts bewegen: linker Bereich wächst
                ++i;

                // ── std::swap ─────────────────────────────────
                // arr[i]: erstes Element das noch nicht links ist
                // arr[j]: das kleinere Element das nach links soll
                std::swap(arr[i], arr[j]);
                ++m.swaps;
                m.arrayAccesses += 2;

                // ── Erklärung nach dem Tausch ─────────────────
                // arr[i] und arr[j] sind NACH dem Swap getauscht!
                step(arr, cb, i, j,
                     std::format(
                         "Tausch: {} und {} wechseln die Plaetze",
                         arr[i], // jetzt links (kleineres Element)
                         arr[j]  // jetzt rechts
                     ));
            }
            // Wenn arr[j] > pivot: kein Tausch, j++ reicht
        }

        // ── Pivot an seine finale Position bringen ────────────
        // arr[i+1] ist das erste Element das > Pivot war.
        // Pivot tauscht mit arr[i+1] → Pivot ist jetzt korrekt!
        // Links von Pivot: <= Pivot
        // Rechts von Pivot: > Pivot
        std::swap(arr[i + 1], arr[high]);
        ++m.swaps;
        m.arrayAccesses += 2;

        step(arr, cb, i + 1, high,
             std::format(
                 "Pivot {} ist jetzt an seiner richtigen Position {}",
                 arr[i + 1], // der Pivot-Wert (nach Swap an i+1)
                 i + 1));    // seine finale sortierte Position

        return i + 1; // finale Pivot-Position zurückgeben
    }

    // ============================================================
    // quickSortRec – Rekursiver Kern
    //
    // ── Basisfall: low >= high ────────────────────────────────
    // Ein Array mit 0 oder 1 Elementen ist bereits sortiert.
    // low == high: genau ein Element → fertig.
    // low >  high: leeres Teilarray → nichts zu tun.
    //
    // ── Rekursionsreihenfolge ─────────────────────────────────
    // 1. partition(): Pivot an richtige Position setzen
    // 2. Linkes Teilarray  [low .. pi-1]  rekursiv sortieren
    // 3. Rechtes Teilarray [pi+1 .. high] rekursiv sortieren
    //
    // ── const int32_t pi ──────────────────────────────────────
    // pi = Pivot-Index nach partition() → ändert sich nie.
    // const macht die Unveränderlichkeit explizit.
    // ============================================================
    static void quickSortRec(std::vector<int32_t>& arr,
                              int32_t               low,
                              int32_t               high,
                              StepCallback&         cb,
                              LiveMetrics&          m)
    {
        // ── Basisfall: ein oder kein Element → fertig ─────────
        if (low >= high) return;

        // ── Pivot platzieren und Index zurückbekommen ─────────
        const int32_t pi = partition(arr, low, high, cb, m);

        // ── Linkes Teilarray sortieren [low .. pi-1] ──────────
        // Alle Elemente hier sind <= Pivot
        quickSortRec(arr, low, pi - 1, cb, m);

        // ── Rechtes Teilarray sortieren [pi+1 .. high] ────────
        // Alle Elemente hier sind > Pivot
        quickSortRec(arr, pi + 1, high, cb, m);
    }

    // ============================================================
    // quickSort – Öffentliche Schnittstelle
    //
    // ── Leerheitscheck ────────────────────────────────────────
    // Verhindert undefined behavior:
    // size() - 1 bei size()==0 wäre Unterlauf (0u-1 = sehr groß)!
    // static_cast<int32_t>: size_t → signed, sicher konvertiert.
    // ============================================================
    void quickSort(std::vector<int32_t>& arr,
                   StepCallback          cb,
                   LiveMetrics&          m)
    {
        if (arr.empty()) return;

        // ── static_cast für size()-1 ──────────────────────────
        // size() gibt size_t (unsigned) zurück.
        // Bei leerem Array: 0u - 1 = UINT_MAX → Absturz!
        // Leerheitscheck oben verhindert das, cast macht es explizit.
        quickSortRec(arr,
                     0,
                     static_cast<int32_t>(arr.size()) - 1,
                     cb,
                     m);
    }

} // namespace Algorithms