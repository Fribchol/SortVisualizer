#include "SortAlgorithms.hpp"
#include <span>
#include <vector>
#include <cstdint>
#include <algorithm>

// ==================================================================
// MergeSort (rekursiv, Top-Down) mit wiederverwendetem Hilfspuffer
// ==================================================================
//
// Grundidee (siehe Skript "Algorithmen und Datenstrukturen", Kap. 4.4):
//   Divide:   Teile die Folge in zwei etwa gleich große Hälften.
//   Conquer:  Sortiere beide Hälften rekursiv.
//   Combine:  Verschmelze ("merge") die beiden sortierten Hälften
//             zu einer einzigen sortierten Folge.
//
// Besonderheit dieser Implementierung (gutes Data-Oriented Design):
// Anstatt bei jedem Merge-Schritt einen neuen Hilfs-Vektor zu
// allozieren (was bei n Rekursionsstufen sehr viele kleine, teure
// Speicherallokationen bedeuten würde), wird EIN einziger Puffer in
// Originalgröße einmalig angelegt und für sämtliche Merge-Vorgänge
// wiederverwendet. Das spart massiv Zeit bei großen Arrays.
//
// std::span (C++20) wird genutzt, um Teilbereiche des Arrays ohne
// Kopieren zu beschreiben - ein span ist nur ein "Fenster"
// (Zeiger + Länge) auf bereits existierende Daten.

namespace SortAlgorithms
{
    // --------------------------------------------------------
    // mergeImpl – verschmilzt zwei bereits sortierte Teilbereiche
    // --------------------------------------------------------
    // Die Teilbereiche [left, mid] und [mid+1, right] von "arr" sind
    // beide für sich bereits sortiert. Diese Funktion führt sie zu
    // einem einzigen sortierten Bereich [left, right] zusammen.
    //
    // Ablauf:
    //   1) Den relevanten Ausschnitt [left, right] nach "bufferView"
    //      kopieren (wir dürfen nicht direkt in "arr" vergleichen,
    //      während wir gleichzeitig "arr" überschreiben).
    //   2) Zwei Lesezeiger i (linke Hälfte) und j (rechte Hälfte) im
    //      Puffer laufen lassen und immer den kleineren der beiden
    //      aktuellen Werte zurück nach "arr" schreiben.
    //   3) Restliche Elemente einer Hälfte (falls die andere zuerst
    //      "leer" ist) einfach anhängen.
    template <bool EnableVisuals>
    void mergeImpl(std::vector<std::int32_t>& arr,
                   std::span<std::int32_t>    bufferView,
                   std::size_t                left,
                   std::size_t                mid,
                   std::size_t                right,
                   const StepCallback&        cb,
                   LiveMetrics&               m)
    {
        // Schritt 1: aktuellen Ausschnitt in den Hilfspuffer kopieren.
        // Jedes der "length" Elemente berührt zwei Speicherstellen
        // (Ursprung in arr, Ziel im Puffer) -> length * 2 Zugriffe.
        const std::size_t length = right - left + 1;
        std::copy(arr.begin() + static_cast<std::ptrdiff_t>(left),
                  arr.begin() + static_cast<std::ptrdiff_t>(right) + 1,
                  bufferView.begin() + static_cast<std::ptrdiff_t>(left));
        m.arrayAccesses += static_cast<std::uint64_t>(length) * 2;

        std::size_t i = left;      // Lesezeiger in der linken Hälfte des Puffers
        std::size_t j = mid + 1;   // Lesezeiger in der rechten Hälfte des Puffers
        std::size_t k = left;      // Schreibzeiger zurück in "arr"

        // Schritt 2: Solange in BEIDEN Hälften noch Elemente übrig sind,
        // immer den kleineren Wert nach "arr" schreiben.
        while (i <= mid && j <= right)
        {
            // Der Vergleich selbst liest zwei Positionen im Puffer.
            ++m.comparisons;
            m.arrayAccesses += 2;

            if (bufferView[i] <= bufferView[j])
            {
                // Zuweisung berührt zwei Speicherstellen: Lesen aus dem
                // Puffer (bufferView[i]) und Schreiben nach arr[k].
                arr[k] = bufferView[i];
                m.arrayAccesses += 2;
                if constexpr (EnableVisuals) cb(arr, static_cast<std::int32_t>(k), static_cast<std::int32_t>(i));
                ++i;
            }
            else
            {
                arr[k] = bufferView[j];
                m.arrayAccesses += 2;
                if constexpr (EnableVisuals) cb(arr, static_cast<std::int32_t>(k), static_cast<std::int32_t>(j));
                ++j;
            }
            ++k;
        }

        // Schritt 3: Eine der beiden Hälften ist jetzt "leer" (vollständig
        // übertragen). Die restlichen Elemente der anderen Hälfte sind
        // bereits sortiert und können einfach direkt angehängt werden -
        // ein weiterer Vergleich ist hier nicht mehr nötig.
        while (i <= mid)
        {
            arr[k] = bufferView[i];
            m.arrayAccesses += 2;
            ++i; ++k;
        }
        while (j <= right)
        {
            arr[k] = bufferView[j];
            m.arrayAccesses += 2;
            ++j; ++k;
        }
    }

    // --------------------------------------------------------
    // mergeSortRange – teilt rekursiv auf (Divide) und ruft danach
    // mergeImpl auf (Combine)
    // --------------------------------------------------------
    // "subArrayView" beschreibt den aktuell zu sortierenden Ausschnitt
    // von "arr". Da std::span nur Zeiger + Länge speichert, kostet das
    // Aufteilen in linke/rechte Hälfte (first()/last()) keinerlei
    // Kopieren - es sind nur neue "Fenster" auf dieselben Daten.
    template <bool EnableVisuals>
    void mergeSortRange(std::vector<std::int32_t>& arr,
                        std::span<std::int32_t>    fullBufferView,
                        std::span<std::int32_t>    subArrayView,
                        const StepCallback&        cb,
                        LiveMetrics&               m)
    {
        // Abbruchbedingung: Ein Bereich mit 0 oder 1 Elementen ist
        // per Definition bereits sortiert.
        if (subArrayView.size() <= 1) return;

        const std::size_t midOffset = subArrayView.size() / 2;
        std::span<std::int32_t> leftSubView  = subArrayView.first(midOffset);
        std::span<std::int32_t> rightSubView = subArrayView.last(subArrayView.size() - midOffset);

        // Divide + Conquer: beide Hälften zunächst unabhängig sortieren.
        mergeSortRange<EnableVisuals>(arr, fullBufferView, leftSubView, cb, m);
        mergeSortRange<EnableVisuals>(arr, fullBufferView, rightSubView, cb, m);

        // Combine: Da subArrayView nur ein "Fenster" in arr ist, können
        // wir aus der Zeiger-Differenz zu arr.data() die tatsächliche
        // Startposition im Gesamtarray zurückrechnen. Das funktioniert
        // sicher, SOLANGE sich arr während des gesamten Sortiervorgangs
        // nicht durch resize()/push_back() im Speicher verschiebt - was
        // hier der Fall ist, da wir ausschließlich vorhandene Elemente
        // überschreiben, aber niemals die Größe von "arr" verändern.
        const std::size_t absStartIdx = static_cast<std::size_t>(subArrayView.data() - arr.data());
        const std::size_t absMidIdx   = absStartIdx + midOffset - 1;
        const std::size_t absEndIdx   = absStartIdx + subArrayView.size() - 1;

        mergeImpl<EnableVisuals>(arr, fullBufferView, absStartIdx, absMidIdx, absEndIdx, cb, m);
    }

    // --------------------------------------------------------
    // mergeSort – öffentliche Schnittstelle
    // --------------------------------------------------------
    void mergeSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        // Arrays mit 0 oder 1 Element sind bereits sortiert.
        if (arr.size() < 2) return;

        // Der Hilfspuffer wird genau EINMAL in voller Array-Größe
        // angelegt (RAII: std::vector kümmert sich selbst um die
        // Freigabe, sobald "buffer" den Gültigkeitsbereich verlässt -
        // kein manuelles new/delete nötig) und über die gesamte
        // Rekursion hinweg wiederverwendet.
        std::vector<std::int32_t> buffer(arr.size());
        const std::span<std::int32_t> bufferView(buffer);
        const std::span<std::int32_t> mainView(arr);

        // Compile-Zeit-Entscheidung (if constexpr in den Unterfunktionen):
        // Ist ein Callback gesetzt, werden Visualisierungsschritte
        // mitgeschickt; ohne Callback (z.B. reiner Benchmark) entsteht
        // dafür kein Laufzeit-Overhead.
        if (cb) mergeSortRange<true>(arr, bufferView, mainView, cb, m);
        else    mergeSortRange<false>(arr, bufferView, mainView, cb, m);
    }

} // namespace SortAlgorithms