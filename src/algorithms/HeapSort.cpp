// ============================================================
// HeapSort.cpp – in-place Max-Heap
//
// C++20 Features & Modernisierungen:
// ┌─────────────────────┬──────────────────────────────────────┐
// │ Anonymer Namespace  │ Verhindert Linker-Konflikte sauber   │
// │ if constexpr        │ Compile-Time Branching! Entfernt     │
// │                     │ ungenutzten Code physisch aus dem    │
// │                     │ Assembly. 100% native Performance.   │
// │ std::int32_t        │ Explizite Typen aus <cstdint>        │
// │ std::format         │ Typsicheres String-Formatting        │
// │ std::swap           │ Optimaler Elementtausch              │
// └─────────────────────┴──────────────────────────────────────┘
//
// Data-Oriented Design (DOD) Notiz:
// HeapSort hat einen garantierten O(n log n) Durchsatz und
// braucht keinen Extra-Speicher. Allerdings ist der
// Speicherzugriff (2*i+1, 2*i+2) nicht sequenziell (viele
// Cache-Misses), weshalb er in der Praxis bei kleinen bis
// mittleren Arrays oft langsamer als QuickSort ist.
// ============================================================
#include "HeapSort.hpp"
#include <algorithm>  // std::swap
#include <format>     // std::format (C++20)
#include <cstdint>    // std::int32_t

namespace Algorithms
{
    // ── Anonymer Namespace ──────────────────────────────────────
    // Ersetzt 'static' für interne Funktionen. Alles hierin
    // ist strikt nur in dieser Translation Unit (.cpp) sichtbar.
    namespace
    {
        // ============================================================
        // heapify – Max-Heap-Eigenschaft wiederherstellen als Template
        // ============================================================
        template <bool EnableVisuals>
        void heapify(std::vector<std::int32_t>& arr,
                     std::int32_t               n,
                     std::int32_t               i,
                     const StepCallback&        cb,
                     LiveMetrics&               m)
        {
            std::int32_t       largest = i;         // Annahme: Wurzel ist am größten
            const std::int32_t left    = 2 * i + 1; // linkes Kind
            const std::int32_t right   = 2 * i + 2; // rechtes Kind

            // Metriken erfassen
            if (left  < n) { ++m.comparisons; ++m.arrayAccesses; }
            if (right < n) { ++m.comparisons; ++m.arrayAccesses; }

            if (left  < n && arr[left]  > arr[largest]) largest = left;
            if (right < n && arr[right] > arr[largest]) largest = right;

            // Wenn die Wurzel nicht das größte Element ist -> tauschen
            if (largest != i)
            {
                std::swap(arr[i], arr[largest]);
                ++m.swaps;
                m.arrayAccesses += 2;

                // Compile-Time Verzweigung: Im CLI-Modus existiert das hier nicht!
                if constexpr (EnableVisuals) {
                    cb(arr, i, largest, std::format(
                        "{} ist größer und muss nach oben – Tausch mit {} an Position {}",
                        arr[i], arr[largest], largest));
                }

                // Rekursiv weiter nach unten prüfen mit demselben Template-Parameter
                heapify<EnableVisuals>(arr, n, largest, cb, m);
            }
        }

        // ============================================================
        // heapSortImpl – Kernlogik als Template
        // ============================================================
        template <bool EnableVisuals>
        void heapSortImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
        {
            const std::int32_t n = static_cast<std::int32_t>(arr.size());

            // Phase 1: Max-Heap aufbauen (von unten nach oben)
            for (std::int32_t i = n / 2 - 1; i >= 0; --i) {
                heapify<EnableVisuals>(arr, n, i, cb, m);
            }

            // Phase 2: Element für Element das Maximum ans Ende tauschen
            for (std::int32_t i = n - 1; i > 0; --i)
            {
                std::swap(arr[0], arr[i]);
                ++m.swaps;
                m.arrayAccesses += 2;

                if constexpr (EnableVisuals) {
                    cb(arr, 0, i, std::format(
                        "Größtes Element {} wird ans Ende an Position {} verschoben",
                        arr[i], i));
                }

                // Heap-Eigenschaft für den geschrumpften Bereich wiederherstellen
                heapify<EnableVisuals>(arr, i, 0, cb, m);
            }
        }
    } // Ende des anonymen Namespaces

    // ============================================================
    // heapSort – Öffentliche Schnittstelle
    // Der Dispatcher: Hier gabeln sich die Wege für GUI und CLI.
    // ============================================================
    void heapSort(std::vector<std::int32_t>& arr, StepCallback cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        if (cb) {
            // GUI-Modus: Voller Feature-Umfang mit Visualisierung
            heapSortImpl<true>(arr, cb, m);
        } else {
            // CLI-Modus: Rohe Rechenpower ohne String-Overhead
            heapSortImpl<false>(arr, cb, m);
        }
    }

} // namespace Algorithms