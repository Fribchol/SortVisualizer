// ============================================================
// CountingSort.cpp – O(n+k), stabil
//
// Algorithmus-Erklärung:
// CountingSort vergleicht keine Elemente miteinander!
// Stattdessen:
//   1. Zähle wie oft jede Zahl vorkommt (Count-Array)
//   2. Baue das Array direkt aus dem Count-Array neu auf
//
// Vorteil: O(n+k) – schneller als O(n log n) wenn k klein
// Nachteil: braucht O(k) extra Speicher für Count-Array
//           funktioniert nur mit ganzen Zahlen
//
// C++20 Features:
// ┌───────────────────┬────────────────────────────────────────┐
// │ std::ranges::max  │ Direkter Container-Zugriff             │
// │ std::ranges::min  │ Ohne begin/end                         │
// │ std::format       │ Typsicheres String-Formatting          │
// │ string_view       │ Kein Kopieren bei Callback-Aufrufen    │
// │ Range-based for   │ Direkt über Container iterieren        │
// │ const auto&       │ Kein Kopieren, read-only               │
// └───────────────────┴────────────────────────────────────────┘
// ============================================================
#include "CountingSort.hpp"
#include <format>   // std::format (C++20)
#include <ranges>   // std::ranges::min, std::ranges::max (C++20)

namespace Algorithms
{
    // ============================================================
    // step – Hilfsfunktion: Schritt melden
    //
    // ── static (interne Linkage) ──────────────────────────────
    // Nur in dieser .cpp Datei sichtbar.
    // Kein Namenskonflikt mit step() in anderen Algorithmen.
    //
    // ── std::string_view ──────────────────────────────────────
    // Kein Kopieren des Strings – nur ein "Blick" darauf.
    // Perfekt für read-only Strings die nur weitergegeben werden.
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
    // countingSort – Hauptfunktion
    //
    // ── std::ranges::max / std::ranges::min (C++20) ───────────
    // Alt:    *std::max_element(arr.begin(), arr.end())
    // Modern: std::ranges::max(arr)
    // Vorteil: kürzer, direkter auf Container, kein begin/end
    //
    // ── Range-based for mit const auto& ───────────────────────
    // Alt:    for (int32_t i = 0; i < arr.size(); ++i)
    // Modern: for (const auto& val : arr)
    // Vorteil: kein Index, kein Zugriff außerhalb, read-only
    //
    // ── std::format (C++20) ───────────────────────────────────
    // Alt:    "Zahl " + std::to_string(x) + " kommt " + ...
    // Modern: std::format("Zahl {} kommt {} mal", x, n)
    // Vorteil: typsicher, lesbar, schneller als string-Konkatenation
    // ============================================================
    void countingSort(std::vector<int32_t>& arr,
                      StepCallback          cb,
                      LiveMetrics&          m)
    {
        if (arr.empty()) return;

        // ── std::ranges::max / min ────────────────────────────
        // Wertebereich bestimmen → Count-Array Größe berechnen
        // const: diese Werte ändern sich nie
        const int32_t maxVal = std::ranges::max(arr);
        const int32_t minVal = std::ranges::min(arr);
        const int32_t range  = maxVal - minVal + 1;

        // ── Count-Array anlegen ───────────────────────────────
        // Index = Wert - minVal (Offset für negative Zahlen)
        // Alle Werte mit 0 initialisieren
        std::vector<int32_t> count(range, 0);

        // ── Phase 1: Häufigkeiten zählen ──────────────────────
        // Range-based for mit const auto&:
        // - kein Index nötig
        // - kein versehentliches Ändern von val
        // - direkt lesbar: "für jeden Wert im Array"
        for (const auto& val : arr)
        {
            ++count[val - minVal]; // Häufigkeit erhöhen
            ++m.arrayAccesses;
        }

        // ── Phase 2: Array rekonstruieren ─────────────────────
        // Für jeden möglichen Wert (0 bis range-1):
        // Wert so oft einfügen wie er gezählt wurde
        int32_t idx = 0; // aktueller Schreibindex im Array

        for (int32_t i = 0; i < range; ++i)
        {
            // count[i]--: post-decrement → Wert VOR dem Decrement prüfen
            // Solange count[i] > 0: diesen Wert noch einmal einfügen
            while (count[i]-- > 0)
            {
                // ── Wert einsetzen ────────────────────────────
                arr[idx] = i + minVal; // Offset zurückrechnen
                ++m.swaps;
                ++m.arrayAccesses;

                // ── std::format: Erklärung auf Deutsch ────────
                // count[i] + 1: weil wir schon decrementiert haben
                // Zeigt dem Nutzer was gerade passiert
                step(arr, cb, idx, -1,
                     std::format(
                         "Zahl {} wurde {} mal gezaehlt – "
                         "wird an Position {} eingesetzt",
                         i + minVal,   // die aktuelle Zahl
                         count[i] + 1, // wie oft sie vorkommt
                         idx));        // wohin sie geschrieben wird

                ++idx;
            }
        }
    }

} // namespace Algorithms