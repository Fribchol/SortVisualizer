// ============================================================
// BubbleSort.cpp
//
// C++20 Features & Modernisierungen:
// ┌─────────────────────┬──────────────────────────────────────┐
// │ Anonymer Namespace  │ Verhindert Linker-Konflikte (besser  │
// │                     │ als das alte C-static)               │
// │ if constexpr        │ Compile-Time Branching! Entfernt     │
// │                     │ ungenutzten Code im CLI-Modus (100%  │
// │                     │ native Performance).                 │
// │ std::int32_t        │ Explizite Typen aus <cstdint>        │
// │ std::format         │ Typsicheres String-Formatting        │
// └─────────────────────┴──────────────────────────────────────┘
//
// Data-Oriented Design (DOD):
// Auch wenn BubbleSort langsam ist O(n^2), greift er strikt
// sequenziell auf den Speicher zu. Das nutzt den L1-Cache
// (Spatial Locality) der CPU perfekt aus.
// ============================================================
#include "BubbleSort.hpp"
#include <format>
#include <utility>
#include <cstdint> // C++ Standard für feste Integer-Breiten

namespace Algorithms
{
    // ── Anonymer Namespace ──────────────────────────────────────
    // Ersetzt 'static' für interne Funktionen. Alles hierin
    // ist strikt nur in dieser Translation Unit (.cpp) sichtbar.
    namespace
    {
        // ============================================================
        // bubbleSortImpl – Kernlogik als Template
        // ============================================================
        template <bool EnableVisuals>
        void bubbleSortImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
        {
            const std::int32_t n = static_cast<std::int32_t>(arr.size());
            bool swapped;

            // Äußere Schleife: Reduziert den zu sortierenden Bereich schrittweise
            for (std::int32_t i = 0; i < n - 1; ++i) {
                swapped = false;

                // Innere Schleife: Hier verbringt die CPU 99% der Zeit.
                // Läuft nur bis n - i - 1, da die letzten i Elemente bereits sortiert sind.
                for (std::int32_t j = 0; j < n - i - 1; ++j) {
                    ++m.comparisons;
                    m.arrayAccesses += 2; // Zwei Lesevorgänge für den Vergleich

                    // Zero-Cost Abstraction:
                    // Ohne Callback (CLI-Modus) existiert dieser Block im Maschinencode gar nicht!
                    if constexpr (EnableVisuals) {
                        cb(arr, j, j + 1, std::format("Vergleiche {} und {}", arr[j], arr[j+1]));
                    }

                    if (arr[j] > arr[j + 1]) {
                        std::swap(arr[j], arr[j + 1]);
                        ++m.swaps;
                        m.arrayAccesses += 4; // std::swap benötigt 2 Reads und 2 Writes

                        if constexpr (EnableVisuals) {
                            cb(arr, j, j + 1, std::format("Tausche {} und {}", arr[j], arr[j+1]));
                        }
                        swapped = true;
                    }
                }

                // Frühzeitiger Abbruch, wenn das Array bereits komplett sortiert ist
                if (!swapped) {
                    if constexpr (EnableVisuals) {
                        cb(arr, -1, -1, "Keine Tauschvorgänge mehr nötig, Array ist sortiert!");
                    }
                    break;
                }
            }
        }
    } // Ende des anonymen Namespaces

    // ============================================================
    // bubbleSort – Öffentliche Schnittstelle
    // Der Dispatcher: Hier gabeln sich die Wege für GUI und CLI.
    // ============================================================
    void bubbleSort(std::vector<std::int32_t>& arr, StepCallback cb, LiveMetrics& m)
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