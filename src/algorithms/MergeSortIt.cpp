// ============================================================
// MergeSortIt.cpp – Bottom-Up MergeSort, iterativ
//
// Algorithmus-Erklärung:
// Statt rekursiv zu teilen (Top-Down), fangen wir mit den
// kleinsten Teilarrays an und verdoppeln die Größe jede Runde.
//
// Runde 1: Paare mergen        [1|2] [3|4] [5|6] ...
// Runde 2: Vierer mergen       [1|2|3|4] [5|6|7|8] ...
// Runde 3: Achter mergen       [1|2|3|4|5|6|7|8] ...
// usw. bis das ganze Array sortiert ist.
//
// Vorteil gegenüber rekursiv:
// - Kein Rekursions-Overhead (kein Stack-Aufbau)
// - Kein Stack-Overflow bei sehr großen Arrays
// - Gleiche Komplexität: O(n log n), stabil
//
// C++20 Features:
// ┌──────────────────────┬─────────────────────────────────────┐
// │ std::format          │ Typsicheres String-Formatting       │
// │ std::min             │ Sicheres Minimum ohne Makro         │
// │ string_view          │ Kein Kopieren bei Callbacks         │
// │ Range-Konstruktor    │ vector direkt aus Iterator-Range    │
// │ const int32_t        │ Unveränderliche lokale Variablen    │
// │ static_cast<int32_t> │ Explizite, sichere Typkonvertierung │
// └──────────────────────┴─────────────────────────────────────┘
// ============================================================
#include "MergeSortIt.hpp"
#include <algorithm>  // std::min
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
    // Referenz: std::function kopieren ist teuer (Heap-Alloc).
    // Referenz = kein Kopieren, O(1) statt O(n).
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
    // merge – Zwei sortierte Hälften zusammenführen
    //
    // ── Range-Konstruktor ─────────────────────────────────────
    // std::vector<int32_t> L(arr.begin() + left, arr.begin() + mid + 1);
    // Baut einen neuen Vektor direkt aus einem Iterator-Bereich.
    // Moderner und sicherer als manuelles Element-für-Element kopieren.
    //
    // ── Warum Kopien nötig sind ───────────────────────────────
    // Wir schreiben in arr[] während wir aus arr[] lesen.
    // Ohne Kopien würden wir überschriebene Werte lesen → Fehler!
    // L und R sind temporäre Kopien der zwei Hälften.
    //
    // ── static_cast<int32_t>(L.size()) ────────────────────────
    // size() gibt size_t (unsigned) zurück.
    // Vergleich signed/unsigned → Compiler-Warnung.
    // static_cast: explizite, dokumentierte Konvertierung.
    // ============================================================
    static void merge(std::vector<int32_t>& arr,
                      int32_t               left,
                      int32_t               mid,
                      int32_t               right,
                      StepCallback&         cb,
                      LiveMetrics&          m)
    {
        // ── Range-Konstruktor (C++, modern) ───────────────────
        // Kopie der linken Hälfte:  [left  .. mid]
        // Kopie der rechten Hälfte: [mid+1 .. right]
        std::vector<int32_t> L(arr.begin() + left,
                                arr.begin() + mid + 1);
        std::vector<int32_t> R(arr.begin() + mid + 1,
                                arr.begin() + right + 1);

        int32_t i = 0;    // Index in L
        int32_t j = 0;    // Index in R
        int32_t k = left; // Schreibindex in arr

        // ── Hauptschleife: kleineres Element vorne einsetzen ──
        // Solange beide Hälften noch Elemente haben:
        // Das kleinere Element in arr[k] schreiben.
        while (i < static_cast<int32_t>(L.size()) &&
               j < static_cast<int32_t>(R.size()))
        {
            ++m.comparisons;

            if (L[i] <= R[j])
            {
                // ── Linkes Element ist kleiner/gleich ─────────
                // <= statt <: macht den Algorithmus STABIL
                // (gleiche Elemente behalten ihre Reihenfolge)
                arr[k] = L[i++];
                ++m.arrayAccesses;
                step(arr, cb, k, -1,
                     std::format(
                         "Merge (iterativ): {} ist kleiner oder gleich, "
                         "kommt an Position {}",
                         arr[k], k));
            }
            else
            {
                // ── Rechtes Element ist kleiner ────────────────
                arr[k] = R[j++];
                ++m.arrayAccesses;
                step(arr, cb, k, -1,
                     std::format(
                         "Merge (iterativ): {} ist kleiner, "
                         "kommt an Position {}",
                         arr[k], k));
            }
            ++k;
        }

        // ── Rest der linken Hälfte kopieren ───────────────────
        // Wenn L noch Elemente hat: direkt einfügen.
        // Sie sind bereits sortiert und alle größer als R's Elemente.
        while (i < static_cast<int32_t>(L.size()))
        {
            arr[k] = L[i++];
            ++m.arrayAccesses;
            step(arr, cb, k, -1,
                 std::format("Rest links: {} an Position {}",
                             arr[k], k));
            ++k;
        }

        // ── Rest der rechten Hälfte kopieren ──────────────────
        // Wenn R noch Elemente hat: direkt einfügen.
        while (j < static_cast<int32_t>(R.size()))
        {
            arr[k] = R[j++];
            ++m.arrayAccesses;
            step(arr, cb, k, -1,
                 std::format("Rest rechts: {} an Position {}",
                             arr[k], k));
            ++k;
        }
    }

    // ============================================================
    // mergeSortIt – Bottom-Up Hauptfunktion
    //
    // ── Äußere Schleife: Teilarray-Größe verdoppeln ───────────
    // size = 1:  je 2 Elemente mergen
    // size = 2:  je 4 Elemente mergen
    // size = 4:  je 8 Elemente mergen
    // usw. bis size >= n
    //
    // ── Innere Schleife: Alle Paare dieser Größe mergen ───────
    // left: Startindex des linken Teilarrays
    // mid:  Endindex des linken Teilarrays
    // right: Endindex des rechten Teilarrays
    //
    // ── std::min für right ────────────────────────────────────
    // Das letzte Paar könnte über das Array-Ende hinausgehen.
    // std::min stellt sicher dass right nie >= n wird.
    // Alt:    right = left + 2*size - 1 > n-1 ? n-1 : ...
    // Modern: right = std::min(left + 2*size - 1, n - 1)
    // ============================================================
    void mergeSortIt(std::vector<int32_t>& arr,
                     StepCallback          cb,
                     LiveMetrics&          m)
    {
        if (arr.empty()) return;

        const int32_t n = static_cast<int32_t>(arr.size());

        // ── Bottom-Up: Größe verdoppeln ────────────────────────
        // size *= 2: jede Runde doppelt so große Teilarrays mergen
        for (int32_t size = 1; size < n; size *= 2)
        {
            // ── Alle Paare dieser Größe mergen ─────────────────
            // left += 2*size: zum nächsten Paar springen
            for (int32_t left = 0; left < n - size; left += 2 * size)
            {
                // Mitte: Ende des linken Teilarrays
                const int32_t mid = left + size - 1;

                // ── std::min: Array-Ende nicht überschreiten ───
                // Das rechte Teilarray könnte kürzer sein
                // als size Elemente (letztes unvollständiges Paar)
                const int32_t right = std::min(
                    left + 2 * size - 1, // ideales Ende
                    n - 1);              // tatsächliches Ende

                merge(arr, left, mid, right, cb, m);
            }
        }
    }

} // namespace Algorithms