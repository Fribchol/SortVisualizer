#include "SortAlgorithms.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

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
// Hinweis zur Index-Übergabe: Frühere Fassungen dieser Datei reichten
// std::span-"Fenster" durch die Rekursion und rechneten den absoluten
// Index über eine Zeigerdifferenz zurück
// (subArrayView.data() - arr.data()). Das war korrekt, SOLANGE arr
// nie durch resize()/push_back() im Speicher verschoben wird - aber
// genau diese Voraussetzung ist an keiner Stelle im Typsystem
// sichtbar und könnte bei künftigen Änderungen still brechen (ohne
// Compilerwarnung, mit undefiniertem Verhalten als Folge).
//
// Diese Fassung reicht stattdessen einfache std::size_t-Indizes
// (low/high) durch die Rekursion - identisch zum Stil von
// QuickSort.cpp und MergeSortIt.cpp. Das ist genauso schnell (keine
// zusätzlichen Kopien), aber unabhängig davon, ob/wie sich "arr" im
// Speicher verhält.

namespace SortAlgorithms
{
    namespace
    {
        // In einen anonymen Namespace verschoben (Konsistenz mit den
        // übrigen Algorithmen): mergeImpl und mergeSortRange sind
        // reine Implementierungsdetails und sollen keine externe
        // Bindung haben. Kollidiert bewusst nicht mit dem gleich-
        // namigen mergeImpl in MergeSortIt.cpp, da anonyme
        // Namespaces pro Translation Unit isoliert sind.

        // --------------------------------------------------------
        // mergeImpl – verschmilzt zwei bereits sortierte Teilbereiche
        // --------------------------------------------------------
        // Die Teilbereiche [left, mid] und [mid+1, right] von "arr" sind
        // beide für sich bereits sortiert. Diese Funktion führt sie zu
        // einem einzigen sortierten Bereich [left, right] zusammen.
        //
        // Ablauf:
        //   1) Den relevanten Ausschnitt [left, right] nach "buffer"
        //      kopieren (wir dürfen nicht direkt in "arr" vergleichen,
        //      während wir gleichzeitig "arr" überschreiben).
        //   2) Zwei Lesezeiger i (linke Hälfte) und j (rechte Hälfte) im
        //      Puffer laufen lassen und immer den kleineren der beiden
        //      aktuellen Werte zurück nach "arr" schreiben.
        //   3) Restliche Elemente einer Hälfte (falls die andere zuerst
        //      "leer" ist) einfach anhängen.
        template <bool EnableVisuals>
        void mergeImpl(std::vector<std::int32_t>& arr,
                       std::vector<std::int32_t>& buffer,
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
                      buffer.begin() + static_cast<std::ptrdiff_t>(left));
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

                if (buffer[i] <= buffer[j])
                {
                    // Zuweisung berührt zwei Speicherstellen: Lesen aus dem
                    // Puffer (buffer[i]) und Schreiben nach arr[k].
                    arr[k] = buffer[i];
                    m.arrayAccesses += 2;
                    if constexpr (EnableVisuals) cb(arr, static_cast<std::int32_t>(k), static_cast<std::int32_t>(i));
                    ++i;
                }
                else
                {
                    arr[k] = buffer[j];
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
                arr[k] = buffer[i];
                m.arrayAccesses += 2;
                ++i; ++k;
            }
            while (j <= right)
            {
                arr[k] = buffer[j];
                m.arrayAccesses += 2;
                ++j; ++k;
            }
        }

        // --------------------------------------------------------
        // mergeSortRange – teilt rekursiv auf (Divide) und ruft danach
        // mergeImpl auf (Combine)
        // --------------------------------------------------------
        // "low"/"high" beschreiben den aktuell zu sortierenden
        // Ausschnitt von "arr" als Indexpaar (inklusive) - kein
        // std::span, keine Zeigerarithmetik.
        template <bool EnableVisuals>
        void mergeSortRange(std::vector<std::int32_t>& arr,
                            std::vector<std::int32_t>& buffer,
                            std::size_t                low,
                            std::size_t                high,
                            const StepCallback&        cb,
                            LiveMetrics&               m)
        {
            // Abbruchbedingung: Ein Bereich mit 0 oder 1 Elementen ist
            // per Definition bereits sortiert.
            if (low >= high) return;

            const std::size_t mid = low + (high - low) / 2;

            // Divide + Conquer: beide Hälften zunächst unabhängig sortieren.
            mergeSortRange<EnableVisuals>(arr, buffer, low, mid, cb, m);
            mergeSortRange<EnableVisuals>(arr, buffer, mid + 1, high, cb, m);

            // Combine: die beiden jetzt sortierten Hälften verschmelzen.
            mergeImpl<EnableVisuals>(arr, buffer, low, mid, high, cb, m);
        }
    } // namespace

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
        const std::size_t low  = 0;
        const std::size_t high = arr.size() - 1;

        // Compile-Zeit-Entscheidung (if constexpr in den Unterfunktionen):
        // Ist ein Callback gesetzt, werden Visualisierungsschritte
        // mitgeschickt; ohne Callback (z.B. reiner Benchmark) entsteht
        // dafür kein Laufzeit-Overhead.
        if (cb) mergeSortRange<true>(arr, buffer, low, high, cb, m);
        else    mergeSortRange<false>(arr, buffer, low, high, cb, m);
    }

} // namespace SortAlgorithms