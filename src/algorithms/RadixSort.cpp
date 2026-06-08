// ============================================================
// RadixSort.cpp – LSD RadixSort, Basis 10
//
// Algorithmus-Erklärung:
// RadixSort sortiert OHNE Vergleiche! Stattdessen werden
// Zahlen nach ihren Stellen (Einer, Zehner, Hunderter...)
// in "Buckets" (0-9) einsortiert.
//
// LSD = Least Significant Digit (niedrigste Stelle zuerst)
// → Wir starten bei den Einern, dann Zehnern, usw.
//
// Beispiel mit [170, 45, 75, 90, 802, 24, 2, 66]:
//   Nach Einerstelle:  [170, 90, 802, 2, 24, 45, 75, 66]
//   Nach Zehnerstelle: [802, 2, 24, 45, 66, 170, 75, 90]
//   Nach Hunderterstelle: [2, 24, 45, 66, 75, 90, 170, 802]
//   → Fertig sortiert!
//
// Warum funktioniert das?
// Weil wir von der niedrigsten zur höchsten Stelle gehen
// UND stabil sortieren (gleiche Ziffern behalten Reihenfolge).
//
// Vorteil:  O(nk) – besser als O(n log n) wenn k klein
//           Stabil: gleiche Zahlen behalten ihre Reihenfolge
// Nachteil: Nur für ganze Zahlen
//           Braucht O(n+k) extra Speicher
//
// C++20 Features:
// ┌──────────────────────┬─────────────────────────────────────┐
// │ std::ranges::max     │ Direkter Container-Zugriff          │
// │ std::format          │ Typsicheres String-Formatting       │
// │ string_view          │ Kein Kopieren bei Callbacks         │
// │ const std::string    │ Unveränderliche lokale Variable      │
// │ const int32_t        │ Unveränderliche Werte               │
// │ static (intern)      │ Funktion nur in dieser Datei        │
// └──────────────────────┴─────────────────────────────────────┘
// ============================================================
#include "RadixSort.hpp"
#include <algorithm>  // std::max_element (Fallback)
#include <format>     // std::format (C++20)
#include <ranges>     // std::ranges::max (C++20)

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
    // Referenz: std::function kopieren ist teuer (Heap-Alloc).
    // Referenz = kein Kopieren, O(1).
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
    // radixSort – Hauptfunktion
    //
    // ── std::ranges::max (C++20) ──────────────────────────────
    // Alt:    *std::max_element(arr.begin(), arr.end())
    // Modern: std::ranges::max(arr)
    // Vorteil: kürzer, direkter auf Container, kein begin/end
    //
    // ── Warum maxVal? ─────────────────────────────────────────
    // Wir brauchen zu wissen wie viele Stellen die größte Zahl hat.
    // exp läuft: 1 → 10 → 100 → ... bis maxVal / exp == 0
    // Dann haben wir alle relevanten Stellen verarbeitet.
    // ============================================================
    void radixSort(std::vector<int32_t>& arr,
                   StepCallback          cb,
                   LiveMetrics&          m)
    {
        if (arr.empty()) return;

        // ── std::ranges::max ──────────────────────────────────
        // Größte Zahl bestimmt wie viele Stellen wir durchlaufen
        const int32_t maxVal = std::ranges::max(arr);
        const int32_t n      = static_cast<int32_t>(arr.size());

        // ── Äußere Schleife: Stelle für Stelle ────────────────
        // exp = 1:   Einerstelle   (arr[i] / 1)   % 10
        // exp = 10:  Zehnerstelle  (arr[i] / 10)  % 10
        // exp = 100: Hunderterstelle (arr[i] / 100) % 10
        // Abbruch: wenn maxVal / exp == 0 (keine Stelle mehr)
        for (int32_t exp = 1; maxVal / exp > 0; exp *= 10)
        {
            // ── Stellenname für Erklärung ─────────────────────
            // const std::string: wird einmal berechnet, nie geändert
            // Ternärer Operator: kompakte Fallunterscheidung
            const std::string stelle =
                (exp == 1)   ? "Einerstelle"
                : (exp == 10)  ? "Zehnerstelle"
                : (exp == 100) ? "Hunderterstelle"
                :                std::format("{}er-Stelle", exp);

            // ── output und count Arrays ────────────────────────
            // output: temporäres Array für das sortierte Ergebnis
            // count:  Zählt wie oft jede Ziffer (0-9) vorkommt
            std::vector<int32_t> output(n, 0);
            std::vector<int32_t> count(10, 0);

            // ── Phase 1: Häufigkeiten der aktuellen Stelle zählen
            // Für jede Zahl: Ziffer an Position exp extrahieren
            // und im count-Array hochzählen.
            // Beispiel exp=1: 123 → Ziffer 3, 456 → Ziffer 6
            for (int32_t i = 0; i < n; ++i)
            {
                ++count[(arr[i] / exp) % 10];
                ++m.arrayAccesses;
            }

            // ── Phase 2: Kumulierte Summe berechnen ───────────
            // count[i] enthält jetzt: wie viele Zahlen haben
            // Ziffer <= i? Das gibt uns die finale Position.
            // Beispiel: count = [0,2,3,1,...] →
            //           nach Kumulation: [0,2,5,6,...]
            // Bedeutet: Zahlen mit Ziffer 2 kommen an Index 2-4
            for (int32_t i = 1; i < 10; ++i)
                count[i] += count[i - 1];

            // ── Phase 3: Stabil von hinten einsortieren ────────
            // Rückwärts iterieren für STABILITÄT:
            // Gleiche Ziffern behalten ihre ursprüngliche Reihenfolge.
            // --count[digit]: Position vor dem Einfügen dekrementieren
            for (int32_t i = n - 1; i >= 0; --i)
            {
                // ── Ziffer an aktueller Stelle extrahieren ─────
                const int32_t digit = (arr[i] / exp) % 10;

                // ── An berechnete Position einfügen ───────────
                // --count[digit]: nächste freie Position für diese Ziffer
                output[--count[digit]] = arr[i];
                ++m.arrayAccesses;
            }

            // ── Phase 4: Ergebnis zurück ins Array kopieren ───
            // output → arr: jetzt nach dieser Stelle sortiert
            for (int32_t i = 0; i < n; ++i)
            {
                arr[i] = output[i];
                ++m.swaps;
                ++m.arrayAccesses;

                // ── std::format: Erklärung auf Deutsch ────────
                step(arr, cb, i, -1,
                     std::format(
                         "Sortiere nach {}: {} hat Ziffer {} "
                         "und kommt an Position {}",
                         stelle,               // z.B. "Einerstelle"
                         arr[i],               // die Zahl selbst
                         (arr[i] / exp) % 10,  // ihre Ziffer an dieser Stelle
                         i));                  // ihre neue Position
            }
        }
    }

} // namespace Algorithms