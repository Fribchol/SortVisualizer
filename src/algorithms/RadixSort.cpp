// ============================================================
// RadixSort.cpp – LSD RadixSort, Basis 10
//
// C++20 Features & Optimierungen (Zero-Cost Abstraction):
// ┌──────────────────────┬─────────────────────────────────────┐
// │ if constexpr         │ Compile-Time Branching (Zero-Cost)  │
// │ Einmalige Allocation │ 'output' wird nur 1x auf dem Heap   │
// │                      │ angelegt.                           │
// │ std::array (Stack)   │ 'count' liegt auf dem Stack (O(1))  │
// │ std::ranges::fill    │ Schnelles Resetten des Arrays       │
// │ std::ranges::max     │ Direkter Container-Zugriff          │
// │ std::format          │ Typsicheres String-Formatting       │
// └──────────────────────┴─────────────────────────────────────┘
// ============================================================
#include "RadixSort.hpp"
#include <algorithm>
#include <format>
#include <ranges>
#include <array>

namespace Algorithms
{
    // ============================================================
    // radixSortImpl – Kernlogik als Template
    // ============================================================
    template <bool EnableVisuals>
    static void radixSortImpl(std::vector<int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        const int32_t maxVal = std::ranges::max(arr);
        const int32_t n      = static_cast<int32_t>(arr.size());

        // ── DOD: Puffer nur EINMAL anlegen ──
        // 'output' liegt auf dem Heap (Größe n).
        // 'count' liegt auf dem ultra-schnellen Stack (feste Größe 10).
        std::vector<int32_t> output(n);
        std::array<int32_t, 10> count{};

        // exp = 1, 10, 100, 1000...
        for (int32_t exp = 1; maxVal / exp > 0; exp *= 10)
        {
            // Reset des Zähler-Arrays auf dem Stack (rasend schnell)
            std::ranges::fill(count, 0);

            // Phase 1: Häufigkeiten der aktuellen Stelle zählen
            for (int32_t i = 0; i < n; ++i)
            {
                ++count[(arr[i] / exp) % 10];
                ++m.arrayAccesses;
            }

            // Phase 2: Kumulierte Summe berechnen
            for (int32_t i = 1; i < 10; ++i)
            {
                count[i] += count[i - 1];
            }

            // Phase 3: Stabil von hinten in das output-Array einsortieren
            for (int32_t i = n - 1; i >= 0; --i)
            {
                const int32_t digit = (arr[i] / exp) % 10;
                output[--count[digit]] = arr[i];
                ++m.arrayAccesses;
            }

            // Phase 4: Ergebnis aus dem Puffer zurück in arr schreiben
            // Wir iterieren hier per Hand, um die GUI-Schritte und Metriken zu triggern.
            for (int32_t i = 0; i < n; ++i)
            {
                arr[i] = output[i];
                ++m.swaps;         // Wertung als Schreib-Operation
                ++m.arrayAccesses;

                // Zero-Cost Abstraction: Im CLI existiert dieser Block nicht!
                if constexpr (EnableVisuals) {
                    const std::string stelle =
                        (exp == 1)   ? "Einerstelle"
                        : (exp == 10)  ? "Zehnerstelle"
                        : (exp == 100) ? "Hunderterstelle"
                        :                std::format("{}er-Stelle", exp);

                    cb(arr, i, -1, std::format(
                        "Sortiere nach {}: {} hat Ziffer {} und kommt an Position {}",
                        stelle, arr[i], (arr[i] / exp) % 10, i));
                }
            }
        }
    }

    // ============================================================
    // radixSort – Öffentliche Schnittstelle
    // Der Dispatcher: Hier gabeln sich die Wege für GUI und CLI.
    // ============================================================
    void radixSort(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        if (cb) {
            // GUI-Modus: Mit allen Visualisierungen und Strings
            radixSortImpl<true>(arr, cb, m);
        } else {
            // CLI-Modus: Pure native Array-Performance
            radixSortImpl<false>(arr, cb, m);
        }
    }

} // namespace Algorithms