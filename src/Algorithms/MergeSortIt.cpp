#include "SortAlgorithms.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

// ==================================================================
// MergeSort (iterativ, Bottom-Up) mit wiederverwendetem Hilfspuffer
// ==================================================================
//
// Grundidee: Anders als beim rekursiven Top-Down-Ansatz wird hier
// nicht erst "geteilt" (Divide), bevor gemerged wird. Stattdessen
// starten wir direkt bei der kleinstmöglichen Blockgröße (1 Element,
// per Definition sortiert) und verschmelzen paarweise benachbarte
// Blöcke zu immer größeren sortierten Blöcken:
//
//   size=1: [5] [2] [8] [1]  -> je 2 Einzelelemente mergen
//   size=2: [2 5] [1 8]      -> je 2 Zweierblöcke mergen
//   size=4: [1 2 5 8]        -> fertig
//
// Die Verdopplung von "size" in jedem Durchlauf ersetzt die
// Rekursionstiefe der Top-Down-Variante - das "Teilen" passiert
// also implizit über die Blockgrößen-Iteration, nicht über
// Funktionsaufrufe. Vorteil: keine Rekursion, kein Rekursions-
// Stack-Overhead.
//
// Wie beim rekursiven Pendant gilt gutes Data-Oriented Design:
// EIN einziger Hilfspuffer wird einmalig angelegt (RAII über
// std::vector) und für alle Merge-Vorgänge wiederverwendet, statt
// bei jedem Merge neu zu allozieren.
//
// Merge-Stil nach Skript "Algorithmen und Datenstrukturen", Kap. 4.4,
// Folie 232 (Prof. Dr. Rethmann, angelehnt an Volker Heun):
//
//   void merge(seq a[], seq b[], int l, int m, int r) {
//       int i = l;
//       int j = m + 1;
//       for (int k = l; k <= r; k++) {
//           if ((j > r) or ((i <= m) and (a[i].key <= a[j].key)))
//                 b[k] = a[i++];
//           else  b[k] = a[j++];
//       }
//   }
//
// Statt zwei getrennten "Rest kopieren"-Schleifen NACH der Haupt-
// schleife (wie in einer naiveren Variante, die genau hier leicht
// einen Bug einbauen kann, wenn man eine der beiden Restschleifen
// vergisst) läuft hier EINE einzige Schleife über k von l bis r.
// Die Randbehandlung steckt direkt in der Bedingung:
//   - "j > r"  -> rechte Hälfte ist leer -> nimm (kurzschluss-
//                 ausgewertet, ohne Elementvergleich!) von links.
//   - sonst, wenn "i <= m" und a[i] <= a[j] -> nimm von links.
//   - sonst -> nimm von rechts. Das deckt automatisch auch den
//              Fall "linke Hälfte ist leer" (i > m) mit ab, weil
//              dann "i <= m" bereits false ist und die erste
//              Bedingung gar nicht mehr auf den Wertevergleich
//              angewiesen ist.
// Dadurch kann die Randbehandlung strukturell nicht vergessen
// werden - es gibt keine separate Schleife, die man auslassen kann.

namespace SortAlgorithms
{
    // --------------------------------------------------------
    // mergeImpl – verschmilzt zwei bereits sortierte Teilbereiche
    // --------------------------------------------------------
    // [left, mid] und [mid+1, right] sind beide für sich sortiert.
    // Diese Funktion führt sie zu einem sortierten Bereich
    // [left, right] zusammen.
    template <bool EnableVisuals>
    void mergeImpl(std::vector<std::int32_t>& arr,
                   std::vector<std::int32_t>& buffer,
                   std::int32_t               left,
                   std::int32_t               mid,
                   std::int32_t               right,
                   const StepCallback&        cb,
                   LiveMetrics&               m)
    {
        // Schritt 1: Ausschnitt in den Hilfspuffer kopieren, damit wir
        // beim Zurückschreiben nach "arr" nicht versehentlich Werte
        // überschreiben, die wir weiter unten noch vergleichen müssen.
        std::copy(arr.begin() + left, arr.begin() + right + 1, buffer.begin() + left);
        m.arrayAccesses += static_cast<std::int64_t>(right - left + 1) * 2;

        auto i = left;      // Lesezeiger linke Hälfte (im Puffer), entspricht "i" im Skript
        auto j = mid + 1;   // Lesezeiger rechte Hälfte (im Puffer), entspricht "j" im Skript

        // Schritt 2: EINE Schleife über alle Zielpositionen k von
        // left bis right. Pro Durchlauf entscheidet die Bedingung,
        // ob von links (i) oder rechts (j) übernommen wird - inklusive
        // beider Randfälle ("linke leer" / "rechte leer"), ohne dass
        // dafür zusätzliche Schleifen nötig sind.
        for (auto k = left; k <= right; ++k)
        {
            // "j > right" wird zuerst geprüft: Ist die rechte Hälfte
            // bereits vollständig übertragen, wird dank Kurzschluss-
            // auswertung von "or" gar kein Elementvergleich mehr
            // durchgeführt - wir übernehmen direkt von links.
            const bool takeFromLeft = (j > right) ||
                                       ((i <= mid) && (buffer[static_cast<std::size_t>(i)] <=
                                                        buffer[static_cast<std::size_t>(j)]));

            // Nur zählen, wenn wirklich zwei Elemente verglichen wurden
            // (d.h. beide Hälften hatten an dieser Stelle noch Elemente
            // übrig) - reine "Rest anhängen"-Schritte sind kein Vergleich.
            if (i <= mid && j <= right) ++m.comparisons;

            if (takeFromLeft)
            {
                arr[static_cast<std::size_t>(k)] = buffer[static_cast<std::size_t>(i)];
                ++m.arrayAccesses;
                if constexpr (EnableVisuals) cb(arr, k, i);
                ++i;
            }
            else
            {
                arr[static_cast<std::size_t>(k)] = buffer[static_cast<std::size_t>(j)];
                ++m.arrayAccesses;
                if constexpr (EnableVisuals) cb(arr, k, j);
                ++j;
            }
        }
    }

    // --------------------------------------------------------
    // mergeSortItImpl – Bottom-Up-Steuerung über Blockgrößen
    // --------------------------------------------------------
    // Äußere Schleife: Blockgröße verdoppelt sich pro Durchlauf
    // (1, 2, 4, 8, ...), bis sie die gesamte Array-Länge erreicht.
    // Innere Schleife: läuft alle Blockpaare der aktuellen Größe
    // durch und merged sie paarweise.
    //
    // "right" wird per std::min gekappt, falls die Array-Länge kein
    // Vielfaches der aktuellen Blockgröße ist (letzter Block ist
    // dann kleiner) - so bleibt der Zugriff immer innerhalb der
    // Array-Grenzen.
    template <bool EnableVisuals>
    void mergeSortItImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        const auto n = static_cast<std::int32_t>(arr.size());

        // Hilfspuffer wird genau EINMAL angelegt (RAII, kein manuelles
        // new/delete) und über alle Merge-Schritte hinweg wiederverwendet.
        std::vector<std::int32_t> buffer(static_cast<std::size_t>(n));

        for (auto size = 1; size < n; size *= 2)
        {
            for (auto left = 0; left < n - size; left += 2 * size)
            {
                const auto mid   = left + size - 1;
                const auto right = std::min(left + 2 * size - 1, n - 1);
                mergeImpl<EnableVisuals>(arr, buffer, left, mid, right, cb, m);
            }
        }
    }

    // --------------------------------------------------------
    // mergeSortIt – öffentliche Schnittstelle
    // --------------------------------------------------------
    void mergeSortIt(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        // Compile-Zeit-Entscheidung wie bei der rekursiven Variante:
        // Mit Callback -> Visualisierungsschritte, ohne Callback
        // (reiner Benchmark) -> kein Laufzeit-Overhead durch if constexpr.
        if (cb) {
            mergeSortItImpl<true>(arr, cb, m);
        } else {
            mergeSortItImpl<false>(arr, cb, m);
        }
    }

} // namespace SortAlgorithms