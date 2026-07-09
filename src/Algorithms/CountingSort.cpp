// ============================================================
// CountingSort.cpp – O(n+k), stabil
// ============================================================

#include "SortAlgorithms.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

// Siehe Skript "Algorithmen und Datenstrukturen", Kap. 4.4,
// Countingsort (Folien 263-280):
//
//   Sortierverfahren OHNE Schlüsselvergleiche. Eingabe: Werte aus
//   einem kleinen, bekannten Wertebereich {0, ..., k-1} (hier:
//   {minVal, ..., maxVal}). Idee: Zähle, wie oft jeder mögliche
//   Wert vorkommt, und berechne daraus direkt die Zielposition
//   jedes Elements in der sortierten Ausgabe.
//
//   Vier Phasen (Pseudocode Folie 264):
//     init:      cnt[i] := 0 für alle i               -> Θ(k)
//     count:     cnt[a[j].key]++ für alle j            -> Θ(n)
//     collect:   cnt[i] += cnt[i-1]  (Präfixsumme!)    -> Θ(k)
//     rearrange: b[cnt[a[j].key]] := a[j]; cnt[...]--,
//                RÜCKWÄRTS für j = n downto 1          -> Θ(n)
//   Gesamt: Θ(n + k).
//
//   WICHTIG - Stabilität (Folie 279): "Elemente mit gleichem
//   Schlüssel stehen nach dem Sortieren in der gleichen Reihenfolge
//   wie vor dem Sortieren." Das ist kein Zufall, sondern folgt
//   direkt aus der RÜCKWÄRTS-Iteration in der rearrange-Phase: von
//   zwei Elementen mit gleichem Wert wird zuerst das hintere in b
//   platziert (an die zu diesem Zeitpunkt letzte freie Position für
//   diesen Wert) und danach das vordere - welches dadurch die davor
//   liegende, "frühere" Position bekommt. Würde man stattdessen
//   einfach "Wert i, cnt[i]-mal wiederholt" ins Ausgabearray
//   schreiben, wäre das Ergebnis zwar auch korrekt sortiert, aber
//   die Herleitung der Stabilität aus dem Algorithmus selbst würde
//   verloren gehen (relevant, sobald Datensätze mit Schlüssel UND
//   Nutzdaten sortiert werden, nicht nur nackte Zahlen).
//
// Data-Oriented Design: count-Array und Ausgabepuffer werden je
// EINMAL pro Sortierlauf angelegt (RAII über std::vector) - keine
// wiederholten Allokationen innerhalb der Phasen.

namespace SortAlgorithms
{
    template <bool EnableVisuals>
    void countingSortImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        // Verwendung von std::ranges::max_element/min_element für
        // maximale Kompatibilität (auch unter restriktiven
        // Toolchains wie MinGW-Clang).
        const auto maxVal = *std::ranges::max_element(arr);
        const auto minVal = *std::ranges::min_element(arr);
        const auto n       = static_cast<std::int32_t>(arr.size());

        // Sicherer Cast der Differenz über int64_t, um Overflow zu
        // vermeiden (int32_t-Wertebereich könnte sonst bei der
        // Subtraktion "maxVal - minVal" knapp überlaufen).
        const auto range = static_cast<std::size_t>(static_cast<std::int64_t>(maxVal) - minVal + 1);

        // "cnt" aus dem Pseudocode: erst Häufigkeiten, später (nach
        // der collect-Phase) Präfixsummen / Zielpositionen.
        std::vector<std::int32_t> count(range, 0);

        // "b" aus dem Pseudocode: separater Ausgabepuffer, in den
        // rearrange schreibt. Countingsort ist NICHT in-place.
        std::vector<std::int32_t> output(static_cast<std::size_t>(n));

        // --- Phase "count": Häufigkeiten zählen ---
        for (const auto& val : arr)
        {
            const auto offset = static_cast<std::size_t>(static_cast<std::int64_t>(val) - minVal);
            ++count[offset];
            ++m.arrayAccesses;
        }

        // --- Phase "collect": Präfixsummen bilden ---
        // Nach dieser Schleife gibt count[offset] an, an welcher
        // (um 1 verschobenen) Position im Ausgabepuffer das jeweils
        // LETZTE Element mit diesem Wert landet.
        for (std::size_t i = 1; i < range; ++i)
        {
            count[i] += count[i - 1];
            ++m.arrayAccesses;
        }

        // --- Phase "rearrange": rückwärts einsortieren ---
        // Die Rückwärtsrichtung (j von n-1 bis 0) ist - wie oben
        // erklärt - der Grund, warum dieses Verfahren stabil ist.
        for (auto j = n - 1; j >= 0; --j)
        {
            const auto val    = arr[static_cast<std::size_t>(j)];
            const auto offset = static_cast<std::size_t>(static_cast<std::int64_t>(val) - minVal);
            const auto pos    = --count[offset];

            output[static_cast<std::size_t>(pos)] = val;
            m.arrayAccesses += 2; // Lesen von arr[j], Schreiben nach output[pos]
        }

        // Ergebnis zurück nach "arr" kopieren, damit das Interface
        // ("arr wird in-place sortiert") für den Aufrufer gleich
        // bleibt wie bei den anderen Sortierverfahren.
        for (auto idx = 0; idx < n; ++idx)
        {
            arr[static_cast<std::size_t>(idx)] = output[static_cast<std::size_t>(idx)];
            ++m.arrayAccesses;

            if constexpr (EnableVisuals) {
                cb(arr, idx, -1);
            }
        }
    }

    void countingSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        // Compile-Zeit-Entscheidung wie bei den übrigen Algorithmen:
        // Mit Callback -> Visualisierungsschritte, ohne Callback
        // (reiner Benchmark) -> kein Laufzeit-Overhead durch if constexpr.
        if (cb) {
            countingSortImpl<true>(arr, cb, m);
        } else {
            countingSortImpl<false>(arr, cb, m);
        }
    }

} // namespace SortAlgorithms