// ============================================================
// BubbleSort.cpp
//
// C++20 Features & Optimierungen (Zero-Cost Abstraction):
// ┌─────────────────────┬──────────────────────────────────────┐
// │ if constexpr        │ Compile-Time Branching! Entfernt     │
// │                     │ ungenutzten Code physisch aus dem    │
// │                     │ Assembly. 100% native Performance.   │
// │ std::format         │ Typsicheres String-Formatting        │
// └─────────────────────┴──────────────────────────────────────┘
// ============================================================
#include "BubbleSort.hpp"
#include <format>
#include <utility>

namespace Algorithms
{
    // ============================================================
    // bubbleSortImpl – Kernlogik als Template
    // ============================================================
    template <bool EnableVisuals>
    static void bubbleSortImpl(std::vector<int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        const int32_t n = static_cast<int32_t>(arr.size());
        bool swapped;
        
        // Äußere Schleife
        for (int32_t i = 0; i < n - 1; ++i) {
            swapped = false;
            
            // Innere Schleife: Hier verbringt die CPU 99% der Zeit
            for (int32_t j = 0; j < n - i - 1; ++j) {
                ++m.comparisons;
                m.arrayAccesses += 2; // Zwei Lesevorgänge für den Vergleich

                // if constexpr wird zur Compile-Zeit ausgewertet.
                // Ohne Callback (CLI-Modus) existiert dieser Block im Maschinencode gar nicht!
                if constexpr (EnableVisuals) {
                    cb(arr, j, j + 1, std::format("Vergleiche {} und {}", arr[j], arr[j+1]));
                }
                
                if (arr[j] > arr[j + 1]) {
                    std::swap(arr[j], arr[j + 1]);
                    ++m.swaps;
                    m.arrayAccesses += 4; // swap benötigt 2 Reads und 2 Writes
                    
                    if constexpr (EnableVisuals) {
                        cb(arr, j, j + 1, std::format("Tausche {} und {}", arr[j], arr[j+1]));
                    }
                    swapped = true;
                }
            }
            
            // Frühzeitiger Abbruch, wenn das Array bereits komplett sortiert ist
            if (!swapped) {
                if constexpr (EnableVisuals) {
                    cb(arr, -1, -1, "Keine Tausche mehr noetig, Array ist sortiert!");
                }
                break;
            }
        }
    }

    // ============================================================
    // bubbleSort – Öffentliche Schnittstelle
    // Der Dispatcher: Hier gabeln sich die Wege für GUI und CLI.
    // ============================================================
    void bubbleSort(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        if (cb) {
            // GUI-Modus: Compiler baut eine Funktion MIT Strings und Callbacks
            bubbleSortImpl<true>(arr, cb, m);
        } else {
            // CLI-Modus: Compiler baut eine Funktion GANZ OHNE Strings (Max-Speed)
            bubbleSortImpl<false>(arr, cb, m);
        }
    }

} // namespace Algorithms