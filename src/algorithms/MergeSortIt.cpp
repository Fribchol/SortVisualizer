// ============================================================
// MergeSortIt.cpp – Bottom-Up MergeSort, iterativ
//
// C++20 Features & Optimierungen (Zero-Cost Abstraction):
// ┌──────────────────────┬─────────────────────────────────────┐
// │ if constexpr         │ Compile-Time Branching (Zero-Cost)  │
// │ Einmalige Allocation │ Kein Heap-Alloc im Merge (Buffer)   │
// │ std::copy            │ Hardware-beschleunigtes Kopieren    │
// │ std::format          │ Typsicheres String-Formatting       │
// │ std::min             │ Sicheres Minimum ohne Makro         │
// └──────────────────────┴─────────────────────────────────────┘
// ============================================================
#include "MergeSortIt.hpp"
#include <algorithm>
#include <format>

namespace Algorithms
{
    // ============================================================
    // mergeImpl – Zwei sortierte Hälften zusammenführen (Template)
    // ============================================================
    template <bool EnableVisuals>
    static void mergeImpl(std::vector<int32_t>& arr,
                          std::vector<int32_t>& buffer, // Wiederverwendbarer Puffer
                          int32_t               left,
                          int32_t               mid,
                          int32_t               right,
                          const StepCallback&   cb,
                          LiveMetrics&          m)
    {
        // ── Data-Oriented: Hardware-beschleunigtes Kopieren ──
        // Statt jedes Mal Vektoren neu zu allozieren, nutzen wir einen pre-allocated Puffer!
        std::copy(arr.begin() + left, arr.begin() + right + 1, buffer.begin() + left);
        m.arrayAccesses += (right - left + 1) * 2; // 1 Read, 1 Write pro Element

        int32_t i = left;      // Index in linker Hälfte (im Buffer)
        int32_t j = mid + 1;   // Index in rechter Hälfte (im Buffer)
        int32_t k = left;      // Schreibindex in original arr

        // ── Hauptschleife ──
        while (i <= mid && j <= right)
        {
            ++m.comparisons;

            if (buffer[i] <= buffer[j])
            {
                arr[k] = buffer[i++];
                ++m.arrayAccesses;

                if constexpr (EnableVisuals) {
                    cb(arr, k, -1, std::format(
                        "Merge (iterativ): {} ist kleiner oder gleich, kommt an Position {}",
                        arr[k], k));
                }
            }
            else
            {
                arr[k] = buffer[j++];
                ++m.arrayAccesses;

                if constexpr (EnableVisuals) {
                    cb(arr, k, -1, std::format(
                        "Merge (iterativ): {} ist kleiner, kommt an Position {}",
                        arr[k], k));
                }
            }
            ++k;
        }

        // ── Rest der linken Hälfte kopieren ──
        // (Rechte Hälfte muss nicht kopiert werden, die steht im arr schon am richtigen Platz!)
        while (i <= mid)
        {
            arr[k] = buffer[i++];
            ++m.arrayAccesses;

            if constexpr (EnableVisuals) {
                cb(arr, k, -1, std::format("Rest links: {} an Position {}", arr[k], k));
            }
            ++k;
        }
    }

    // ============================================================
    // mergeSortItImpl – Bottom-Up Hauptfunktion als Template
    // ============================================================
    template <bool EnableVisuals>
    static void mergeSortItImpl(std::vector<int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        const int32_t n = static_cast<int32_t>(arr.size());

        // ── DOD: Ein einziger Puffer für den gesamten Algorithmus! ──
        // Verhindert Heap-Fragmentierung und massiven Geschwindigkeitsverlust.
        std::vector<int32_t> buffer(n);

        // Bottom-Up: Größe verdoppeln (1, 2, 4, 8...)
        for (int32_t size = 1; size < n; size *= 2)
        {
            for (int32_t left = 0; left < n - size; left += 2 * size)
            {
                const int32_t mid   = left + size - 1;
                const int32_t right = std::min(left + 2 * size - 1, n - 1);

                mergeImpl<EnableVisuals>(arr, buffer, left, mid, right, cb, m);
            }
        }
    }

    // ============================================================
    // mergeSortIt – Öffentliche Schnittstelle
    // Der Dispatcher: Hier gabeln sich die Wege für GUI und CLI.
    // ============================================================
    void mergeSortIt(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        if (cb) {
            // GUI-Modus: Mit allen Visualisierungen und Strings
            mergeSortItImpl<true>(arr, cb, m);
        } else {
            // CLI-Modus: Pure native Array-Performance ohne Strings und Re-Allocs
            mergeSortItImpl<false>(arr, cb, m);
        }
    }

} // namespace Algorithms