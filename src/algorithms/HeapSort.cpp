// ============================================================
// HeapSort.cpp – in-place Max-Heap
//
// Algorithmus-Erklärung:
// HeapSort nutzt eine spezielle Datenstruktur: den Max-Heap.
// Ein Max-Heap ist ein Baum wo jeder Knoten >= seinen Kindern.
// Das größte Element steht IMMER an der Wurzel (Index 0).
//
// Phase 1: Heap aufbauen (heapify von unten nach oben)
// Phase 2: Wurzel (Maximum) ans Ende → Heap verkleinern
//
// Vorteil:  O(n log n) garantiert, O(1) extra Speicher
// Nachteil: nicht stabil, Cache-unfreundlich (Sprungzugriffe)
//
// Baum-Index-Arithmetik (0-basiert):
//   Knoten i → linkes Kind:  2*i + 1
//   Knoten i → rechtes Kind: 2*i + 2
//   Knoten i → Elternteil:   (i-1) / 2
//
// C++20 Features:
// ┌─────────────────┬──────────────────────────────────────────┐
// │ std::format     │ Typsicheres String-Formatting            │
// │ std::swap       │ Effizientes Tauschen ohne Temp-Variable  │
// │ string_view     │ Kein Kopieren bei Callback-Aufrufen      │
// │ const int32_t   │ Unveränderliche lokale Variablen         │
// │ static (intern) │ Funktion nur in dieser Datei sichtbar    │
// └─────────────────┴──────────────────────────────────────────┘
// ============================================================
#include "HeapSort.hpp"
#include <algorithm>  // std::swap
#include <format>     // std::format (C++20)

namespace Algorithms
{
    // ============================================================
    // step – Hilfsfunktion: Schritt melden
    //
    // ── static (interne Linkage) ──────────────────────────────
    // Nur in dieser .cpp Datei sichtbar → kein Namenskonflikt
    // mit step() in anderen Algorithmus-Dateien.
    //
    // ── StepCallback& (Referenz) ──────────────────────────────
    // Referenz statt Kopie: std::function kopieren ist teuer
    // (möglicher Heap-Alloc). Referenz = O(1).
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
    // heapify – Max-Heap-Eigenschaft wiederherstellen
    //
    // Stellt sicher dass arr[i] >= arr[linkes Kind]
    //                    arr[i] >= arr[rechtes Kind]
    // Falls nicht: tauschen und rekursiv weiter unten prüfen.
    //
    // ── const int32_t für Kindindizes ─────────────────────────
    // left und right ändern sich nie → const macht das klar
    // und verhindert versehentliches Überschreiben.
    //
    // ── std::swap ─────────────────────────────────────────────
    // Alt:    int tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
    // Modern: std::swap(arr[i], arr[j]);
    // Vorteil: kein temporäre Variable, klar lesbar, optimal
    // ============================================================
    static void heapify(std::vector<int32_t>& arr,
                        int32_t               n,
                        int32_t               i,
                        StepCallback&         cb,
                        LiveMetrics&          m)
    {
        int32_t       largest = i;         // Annahme: Wurzel ist größte
        const int32_t left    = 2 * i + 1; // linkes Kind
        const int32_t right   = 2 * i + 2; // rechtes Kind

        // ── Kinder mit Wurzel vergleichen ─────────────────────
        // Erst prüfen ob Kind existiert (< n), dann vergleichen
        if (left  < n) { ++m.comparisons; ++m.arrayAccesses; }
        if (right < n) { ++m.comparisons; ++m.arrayAccesses; }

        if (left  < n && arr[left]  > arr[largest]) largest = left;
        if (right < n && arr[right] > arr[largest]) largest = right;

        // ── Tausch nur wenn nötig ─────────────────────────────
        // Wenn Wurzel nicht das Größte → tauschen und
        // Heap-Eigenschaft rekursiv weiter unten prüfen
        if (largest != i)
        {
            // ── std::swap ─────────────────────────────────────
            std::swap(arr[i], arr[largest]);
            ++m.swaps;
            m.arrayAccesses += 2;

            // ── std::format: Erklärung auf Deutsch ────────────
            // arr[i] und arr[largest] sind NACH dem Swap getauscht!
            // Deswegen: arr[i] ist jetzt das größere Element (oben)
            //           arr[largest] ist jetzt das kleinere (unten)
            step(arr, cb, i, largest,
                 std::format(
                     "{} ist grösser und muss nach oben – "
                     "Tausch mit {} an Position {}",
                     arr[i],       // das größere Element (jetzt oben)
                     arr[largest], // das kleinere Element (jetzt unten)
                     largest));    // Position des kleineren Elements

            // ── Rekursion: Heap-Eigenschaft weiter prüfen ─────
            // Nach dem Tausch könnte die Heap-Eigenschaft
            // beim Kind verletzt sein → rekursiv prüfen
            heapify(arr, n, largest, cb, m);
        }
    }

    // ============================================================
    // heapSort – Hauptfunktion
    //
    // ── Phase 1: Max-Heap aufbauen ────────────────────────────
    // Starte bei n/2 - 1 (letzter Nicht-Blatt-Knoten).
    // Blätter (untere Hälfte) sind bereits gültige Heaps.
    // Rückwärts iterieren: von unten nach oben aufbauen.
    //
    // ── Phase 2: Sortieren ────────────────────────────────────
    // Maximum (Index 0) ans Ende tauschen.
    // Heap um 1 verkleinern (i als neues n).
    // Heap-Eigenschaft wiederherstellen.
    // Wiederholen bis Heap Größe 1 hat.
    // ============================================================
    void heapSort(std::vector<int32_t>& arr,
                  StepCallback          cb,
                  LiveMetrics&          m)
    {
        if (arr.empty()) return;

        const int32_t n = static_cast<int32_t>(arr.size());

        // ── Phase 1: Max-Heap aufbauen ────────────────────────
        // n/2 - 1: letzter innerer Knoten (Blätter überspringen)
        // Rückwärts: von unten nach oben, damit Teilbäume
        // bereits gültige Heaps sind wenn wir sie brauchen
        for (int32_t i = n / 2 - 1; i >= 0; --i)
            heapify(arr, n, i, cb, m);

        // ── Phase 2: Element für Element sortieren ────────────
        // i: aktuelle Heap-Größe (wird jeden Schritt kleiner)
        for (int32_t i = n - 1; i > 0; --i)
        {
            // ── Maximum (Wurzel) ans Ende tauschen ────────────
            // arr[0] = größtes Element im aktuellen Heap
            // arr[i] = letzte Position des unsortierten Bereichs
            std::swap(arr[0], arr[i]);
            ++m.swaps;
            m.arrayAccesses += 2;

            // ── std::format: Erklärung ─────────────────────────
            // arr[i] ist NACH dem Swap das Maximum (jetzt am Ende)
            step(arr, cb, 0, i,
                 std::format(
                     "Grösstes Element {} wird ans Ende "
                     "an Position {} verschoben",
                     arr[i], // das Maximum (jetzt an Position i)
                     i));    // seine finale sortierte Position

            // ── Heap ohne das letzte Element wiederherstellen ──
            // i = neue Heap-Größe (letztes Element ist fertig sortiert)
            heapify(arr, i, 0, cb, m);
        }
    }

} // namespace Algorithms