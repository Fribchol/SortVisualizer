// ============================================================
// HeapSort.cpp – in-place Max-Heap
//
// C++20 Features & Optimierungen (Zero-Cost Abstraction):
// ┌─────────────────────┬──────────────────────────────────────┐
// │ if constexpr        │ Compile-Time Branching! Entfernt     │
// │                     │ ungenutzten Code physisch aus dem    │
// │                     │ Assembly. 100% native Performance.   │
// │ std::format         │ Typsicheres String-Formatting        │
// │ std::swap           │ Optimaler Elementtausch              │
// └─────────────────────┴──────────────────────────────────────┘
// ============================================================
#include "HeapSort.hpp"
#include <algorithm>  // std::swap
#include <format>     // std::format (C++20)

namespace Algorithms
{
    // ============================================================
    // heapify – Max-Heap-Eigenschaft wiederherstellen als Template
    // ============================================================
    template <bool EnableVisuals>
    static void heapify(std::vector<int32_t>& arr,
                        int32_t               n,
                        int32_t               i,
                        const StepCallback&   cb,
                        LiveMetrics&          m)
    {
        int32_t       largest = i;         // Annahme: Wurzel ist am groessten
        const int32_t left    = 2 * i + 1; // linkes Kind
        const int32_t right   = 2 * i + 2; // rechtes Kind

        // Metriken erfassen
        if (left  < n) { ++m.comparisons; ++m.arrayAccesses; }
        if (right < n) { ++m.comparisons; ++m.arrayAccesses; }

        if (left  < n && arr[left]  > arr[largest]) largest = left;
        if (right < n && arr[right] > arr[largest]) largest = right;

        // Wenn die Wurzel nicht das groesste Element ist -> tauschen
        if (largest != i)
        {
            std::swap(arr[i], arr[largest]);
            ++m.swaps;
            m.arrayAccesses += 2;

            // Compile-Time Verzweigung: Im CLI-Modus existiert das hier nicht!
            if constexpr (EnableVisuals) {
                cb(arr, i, largest, std::format(
                    "{} ist groesser und muss nach oben – Tausch mit {} an Position {}",
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
    static void heapSortImpl(std::vector<int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        const int32_t n = static_cast<int32_t>(arr.size());

        // Phase 1: Max-Heap aufbauen (von unten nach oben)
        for (int32_t i = n / 2 - 1; i >= 0; --i) {
            heapify<EnableVisuals>(arr, n, i, cb, m);
        }

        // Phase 2: Element für Element das Maximum ans Ende tauschen
        for (int32_t i = n - 1; i > 0; --i)
        {
            std::swap(arr[0], arr[i]);
            ++m.swaps;
            m.arrayAccesses += 2;

            if constexpr (EnableVisuals) {
                cb(arr, 0, i, std::format(
                    "Grösstes Element {} wird ans Ende an Position {} verschoben",
                    arr[i], i));
            }

            // Heap-Eigenschaft für den geschrumpften Bereich wiederherstellen
            heapify<EnableVisuals>(arr, i, 0, cb, m);
        }
    }

    // ============================================================
    // heapSort – Öffentliche Schnittstelle
    // Der Dispatcher: Hier gabeln sich die Wege für GUI und CLI.
    // ============================================================
    void heapSort(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m)
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