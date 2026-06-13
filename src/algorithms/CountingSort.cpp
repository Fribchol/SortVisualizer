// ============================================================
// CountingSort.cpp – O(n+k), stabil
//
// C++20 Features & Modernisierungen:
// ┌───────────────────┬────────────────────────────────────────┐
// │ Anonymer Namespace│ Verhindert Linker-Konflikte sauber     │
// │ if constexpr      │ Compile-Time Branching! Entfernt       │
// │                   │ Strings und Callbacks im CLI-Modus.    │
// │ std::ranges::max  │ Direkter Container-Zugriff (C++20)     │
// │ std::ranges::min  │ Ohne begin/end Iterator-Boilerplate    │
// │ std::format       │ Typsicheres String-Formatting          │
// │ std::int32_t      │ Explizite Typen aus <cstdint>          │
// └───────────────────┴────────────────────────────────────────┘
//
// Data-Oriented Design (DOD):
// Vergleicht keine Elemente, sondern nutzt die Werte als
// direkte Speicher-Indizes (O(1) Array-Lookup). Das sorgt
// für massiven Durchsatz, solange die Range (k) klein ist.
// ============================================================
#include "CountingSort.hpp"
#include <format>
#include <ranges>
#include <cstdint> // C++ Standard für feste Integer-Breiten

namespace Algorithms
{
    // ── Anonymer Namespace ──────────────────────────────────────
    // Ersetzt 'static' für interne Funktionen. Alles hierin
    // ist strikt nur in dieser Translation Unit (.cpp) sichtbar.
    namespace
    {
        // ============================================================
        // countingSortImpl – Kernlogik als Template
        // ============================================================
        template <bool EnableVisuals>
        void countingSortImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
        {
            // ── Wertebereich (Range) bestimmen ──
            // std::ranges (C++20) spart uns das unübersichtliche arr.begin(), arr.end()
            const std::int32_t maxVal = std::ranges::max(arr);
            const std::int32_t minVal = std::ranges::min(arr);
            const std::int32_t range  = maxVal - minVal + 1;

            // ── Count-Array anlegen und mit 0 initialisieren ──
            std::vector<std::int32_t> count(range, 0);

            // ── Phase 1: Häufigkeiten zählen ──
            // Data-Oriented: Linearer, Cache-freundlicher Speicherzugriff
            for (const auto& val : arr)
            {
                ++count[val - minVal];
                ++m.arrayAccesses;
            }

            // ── Phase 2: Array rekonstruieren ──
            std::int32_t idx = 0;

            for (std::int32_t i = 0; i < range; ++i)
            {
                // Solange dieser Wert noch in unserer "Zählliste" steht:
                while (count[i]-- > 0)
                {
                    arr[idx] = i + minVal; // Offset wieder dazurechnen
                    ++m.swaps;             // Wir werten das Einfügen als Tausch für die Metriken
                    ++m.arrayAccesses;

                    // Zero-Cost Abstraction: Block existiert im CLI-Modus physisch nicht!
                    if constexpr (EnableVisuals) {
                        cb(arr, idx, -1, std::format(
                            "Zahl {} eingesetzt (aus Count-Array, noch {} übrig) an Position {}",
                            i + minVal, count[i] + 1, idx));
                    }

                    ++idx;
                }
            }
        }
    } // Ende des anonymen Namespaces

    // ============================================================
    // countingSort – Öffentliche Schnittstelle
    // Der Dispatcher: Hier gabeln sich die Wege für GUI und CLI.
    // ============================================================
    void countingSort(std::vector<std::int32_t>& arr, StepCallback cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        if (cb) {
            // GUI-Modus: Mit allen Visualisierungen und Strings
            countingSortImpl<true>(arr, cb, m);
        } else {
            // CLI-Modus: Pure native Array-Performance ohne Strings
            countingSortImpl<false>(arr, cb, m);
        }
    }

} // namespace Algorithms