// ============================================================
// MergeSortRec.cpp – Top-Down MergeSort, rekursiv
//
// Algorithmus-Erklärung:
// MergeSort folgt dem "Teile und Herrsche" Prinzip:
//
//   TEILE:    Array in zwei Hälften aufteilen
//   HERRSCHE: Jede Hälfte rekursiv sortieren
//   FÜGE:     Beide sortierten Hälften zusammenführen
//
// Visualisierung (n=8):
//   [8|3|5|1|4|7|2|6]
//   [8|3|5|1] [4|7|2|6]       ← Teilen
//   [8|3] [5|1] [4|7] [2|6]   ← Teilen
//   [3|8] [1|5] [4|7] [2|6]   ← Merge (Paare)
//   [1|3|5|8] [2|4|6|7]       ← Merge (Vierer)
//   [1|2|3|4|5|6|7|8]         ← Merge (fertig!)
//
// Vorteil:  O(n log n) garantiert, stabil
// Nachteil: O(n) extra Speicher für Kopien (L und R)
//
// Overflow-sichere Mitte:
//   Falsch:  mid = (left + right) / 2  → kann überlaufen!
//   Richtig: mid = left + (right - left) / 2
//
// C++20 Features:
// ┌──────────────────────┬─────────────────────────────────────┐
// │ std::format          │ Typsicheres String-Formatting       │
// │ string_view          │ Kein Kopieren bei Callbacks         │
// │ Range-Konstruktor    │ vector direkt aus Iterator-Range    │
// │ const int32_t        │ Unveränderliche lokale Variablen    │
// │ static_cast<int32_t> │ Explizite, sichere Typkonvertierung │
// │ static (intern)      │ Funktion nur in dieser Datei        │
// └──────────────────────┴─────────────────────────────────────┘
// ============================================================
#include "MergeSortRec.hpp"
#include <format>  // std::format (C++20)

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
    // merge – Zwei sortierte Hälften zusammenführen
    //
    // ── Range-Konstruktor ─────────────────────────────────────
    // std::vector<int32_t> L(arr.begin() + left, arr.begin() + mid + 1);
    // Baut einen neuen Vektor direkt aus einem Iterator-Bereich.
    // Moderner und sicherer als manuelles Element-für-Element Kopieren.
    //
    // ── Warum Kopien (L und R) nötig sind ────────────────────
    // Wir schreiben in arr[] während wir aus arr[] lesen.
    // Ohne Kopien: überschriebene Werte werden fälschlicherweise
    // nochmal gelesen → falsches Ergebnis!
    // L und R sind temporäre, sichere Kopien der zwei Hälften.
    //
    // ── L[i] <= R[j] für Stabilität ──────────────────────────
    // <= statt <: Bei gleichen Elementen kommt das linke zuerst.
    // Das garantiert STABILITÄT: gleiche Werte behalten ihre
    // ursprüngliche relative Reihenfolge im Array.
    // ============================================================
    static void merge(std::vector<int32_t>& arr,
                      int32_t               left,
                      int32_t               mid,
                      int32_t               right,
                      StepCallback&         cb,
                      LiveMetrics&          m)
    {
        // ── Range-Konstruktor ─────────────────────────────────
        // Linke Hälfte:  Elemente von [left  .. mid]
        // Rechte Hälfte: Elemente von [mid+1 .. right]
        std::vector<int32_t> L(arr.begin() + left,
                                arr.begin() + mid + 1);
        std::vector<int32_t> R(arr.begin() + mid + 1,
                                arr.begin() + right + 1);

        int32_t i = 0;    // Index in L (linke Hälfte)
        int32_t j = 0;    // Index in R (rechte Hälfte)
        int32_t k = left; // Schreibindex in arr

        // ── Hauptschleife: kleineres Element vorne einsetzen ──
        // Vergleicht jeweils die vordersten Elemente von L und R.
        // Das kleinere kommt zuerst ins Array.
        while (i < static_cast<int32_t>(L.size()) &&
               j < static_cast<int32_t>(R.size()))
        {
            ++m.comparisons;

            if (L[i] <= R[j])
            {
                // ── Linkes Element kleiner/gleich → stabil! ───
                // Bei Gleichheit linkes zuerst → Stabilität!
                arr[k] = L[i++];
                ++m.arrayAccesses;
                step(arr, cb, k, -1,
                     std::format(
                         "Zusammenführen: {} ist kleiner oder gleich, "
                         "wird an Position {} gesetzt",
                         arr[k], k));
            }
            else
            {
                // ── Rechtes Element kleiner → nach vorne ──────
                arr[k] = R[j++];
                ++m.arrayAccesses;
                step(arr, cb, k, -1,
                     std::format(
                         "Zusammenführen: {} ist kleiner, "
                         "wird an Position {} gesetzt",
                         arr[k], k));
            }
            ++k;
        }

        // ── Rest der linken Hälfte einfügen ───────────────────
        // Falls L noch Elemente hat: alle sind größer als R's letztes.
        // Direkt einfügen, kein Vergleich mehr nötig.
        while (i < static_cast<int32_t>(L.size()))
        {
            arr[k] = L[i++];
            ++m.arrayAccesses;
            step(arr, cb, k, -1,
                 std::format(
                     "Rest der linken Haelfte: "
                     "{} wird an Position {} gesetzt",
                     arr[k], k));
            ++k;
        }

        // ── Rest der rechten Hälfte einfügen ──────────────────
        // Falls R noch Elemente hat: analog zur linken Hälfte.
        while (j < static_cast<int32_t>(R.size()))
        {
            arr[k] = R[j++];
            ++m.arrayAccesses;
            step(arr, cb, k, -1,
                 std::format(
                     "Rest der rechten Haelfte: "
                     "{} wird an Position {} gesetzt",
                     arr[k], k));
            ++k;
        }
    }

    // ============================================================
    // mergeSortRecHelper – Rekursiver Kern
    //
    // ── Basisfall: left >= right ──────────────────────────────
    // Ein Array mit 0 oder 1 Elementen ist bereits sortiert.
    // Die Rekursion stoppt wenn left == right (ein Element).
    //
    // ── Overflow-sichere Mittenberechnung ────────────────────
    // Falsch:  (left + right) / 2
    //          → kann überlaufen wenn left + right > INT_MAX!
    // Richtig: left + (right - left) / 2
    //          → right - left ist immer <= INT_MAX
    //
    // ── Rekursionsreihenfolge ─────────────────────────────────
    // 1. Linke Hälfte sortieren  (rekursiv)
    // 2. Rechte Hälfte sortieren (rekursiv)
    // 3. Beide Hälften mergen
    // Reihenfolge ist wichtig: merge() setzt sortierte Hälften voraus!
    // ============================================================
    static void mergeSortRecHelper(std::vector<int32_t>& arr,
                                    int32_t               left,
                                    int32_t               right,
                                    StepCallback&         cb,
                                    LiveMetrics&          m)
    {
        // ── Basisfall ─────────────────────────────────────────
        // left == right: nur noch ein Element → bereits sortiert
        // left >  right: leeres Teilarray → nichts zu tun
        if (left >= right) return;

        // ── Overflow-sichere Mitte ────────────────────────────
        const int32_t mid = left + (right - left) / 2;

        // ── Divide: Teilen ────────────────────────────────────
        mergeSortRecHelper(arr, left,    mid,   cb, m); // links
        mergeSortRecHelper(arr, mid + 1, right, cb, m); // rechts

        // ── Conquer: Zusammenführen ───────────────────────────
        // Jetzt sind [left..mid] und [mid+1..right] sortiert.
        // merge() fügt sie zu einem sortierten Bereich zusammen.
        merge(arr, left, mid, right, cb, m);
    }

    // ============================================================
    // mergeSortRec – Öffentliche Schnittstelle
    //
    // ── Leerheitscheck vor Rekursion ─────────────────────────
    // Verhindert undefined behavior bei leerem Array
    // (size() - 1 wäre bei size()==0 ein Unterlauf: 0-1 = UINT_MAX)
    // ============================================================
    void mergeSortRec(std::vector<int32_t>& arr,
                      StepCallback          cb,
                      LiveMetrics&          m)
    {
        if (arr.empty()) return;

        // ── static_cast<int32_t> ──────────────────────────────
        // size() gibt size_t (unsigned) zurück.
        // -1 bei unsigned würde zu sehr großer Zahl werden!
        // static_cast: sichere, explizite Konvertierung zu signed.
        mergeSortRecHelper(arr,
                           0,
                           static_cast<int32_t>(arr.size()) - 1,
                           cb,
                           m);
    }

} // namespace Algorithms