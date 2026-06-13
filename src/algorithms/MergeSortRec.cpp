// ============================================================
// MergeSortRec.cpp – Top-Down MergeSort, rekursiv
//
// C++20 Features & Optimierungen (Zero-Cost Abstraction):
// ┌──────────────────────┬─────────────────────────────────────┐
// │ if constexpr         │ Compile-Time Branching (Zero-Cost)  │
// │ Einmalige Allocation │ Ein einziger Buffer für den ganzen  │
// │                      │ Rekursionsbaum -> Max Performance!  │
// │ std::copy            │ Hardware-beschleunigtes Kopieren    │
// │ std::format          │ Typsicheres String-Formatting       │
// └──────────────────────┴─────────────────────────────────────┘
// ============================================================
#include "MergeSortRec.hpp"
#include <algorithm>
#include <format>

namespace Algorithms
{
    // ============================================================
    // mergeImpl – Zwei sortierte Hälften zusammenführen (Template)
    // ============================================================
    template <bool EnableVisuals>
    static void mergeImpl(std::vector<int32_t>& arr,
                          std::vector<int32_t>& buffer, // Wiederverwendbarer Puffer!
                          int32_t               left,
                          int32_t               mid,
                          int32_t               right,
                          const StepCallback&   cb,
                          LiveMetrics&          m)
    {
        // ── Data-Oriented: Kopieren per memmove ──
        // Wir kopieren den relevanten Bereich aus arr in unseren Puffer.
        // Das ist millionenfach schneller als jedes Mal neue std::vectors zu erstellen.
        std::copy(arr.begin() + left, arr.begin() + right + 1, buffer.begin() + left);
        m.arrayAccesses += (right - left + 1) * 2; // Read (arr) + Write (buffer)

        int32_t i = left;      // Lese-Index linke Hälfte (im Buffer)
        int32_t j = mid + 1;   // Lese-Index rechte Hälfte (im Buffer)
        int32_t k = left;      // Schreib-Index in original arr

        // ── Hauptschleife: Das kleinere Element zurück in arr schreiben ──
        while (i <= mid && j <= right)
        {
            ++m.comparisons;

            if (buffer[i] <= buffer[j])
            {
                // <= sorgt für Stabilität des Algorithmus
                arr[k] = buffer[i++];
                ++m.arrayAccesses;

                if constexpr (EnableVisuals) {
                    cb(arr, k, -1, std::format(
                        "Zusammenfuehren (rekursiv): {} ist kleiner oder gleich -> Position {}",
                        arr[k], k));
                }
            }
            else
            {
                arr[k] = buffer[j++];
                ++m.arrayAccesses;

                if constexpr (EnableVisuals) {
                    cb(arr, k, -1, std::format(
                        "Zusammenfuehren (rekursiv): {} ist kleiner -> Position {}",
                        arr[k], k));
                }
            }
            ++k;
        }

        // ── Reste der linken Hälfte kopieren ──
        while (i <= mid)
        {
            arr[k] = buffer[i++];
            ++m.arrayAccesses;

            if constexpr (EnableVisuals) {
                cb(arr, k, -1, std::format("Rest links: {} wird an Position {} gesetzt", arr[k], k));
            }
            ++k;
        }

        // ── Reste der rechten Hälfte kopieren ──
        while (j <= right)
        {
            arr[k] = buffer[j++];
            ++m.arrayAccesses;

            if constexpr (EnableVisuals) {
                cb(arr, k, -1, std::format("Rest rechts: {} wird an Position {} gesetzt", arr[k], k));
            }
            ++k;
        }
    }

    // ============================================================
    // mergeSortRecHelper – Rekursiver Kern als Template
    // ============================================================
    template <bool EnableVisuals>
    static void mergeSortRecHelper(std::vector<int32_t>& arr,
                                   std::vector<int32_t>& buffer,
                                   int32_t               left,
                                   int32_t               right,
                                   const StepCallback&   cb,
                                   LiveMetrics&          m)
    {
        // Basisfall: Teilarray ist leer oder hat nur 1 Element
        if (left >= right) return;

        // Overflow-sichere Mittenberechnung
        const int32_t mid = left + (right - left) / 2;

        // Teile (Divide)
        mergeSortRecHelper<EnableVisuals>(arr, buffer, left, mid, cb, m);
        mergeSortRecHelper<EnableVisuals>(arr, buffer, mid + 1, right, cb, m);

        // Herrsche & Fuge zusammen (Conquer & Merge)
        mergeImpl<EnableVisuals>(arr, buffer, left, mid, right, cb, m);
    }

    // ============================================================
    // mergeSortRec – Öffentliche Schnittstelle
    // Der Dispatcher: Hier gabeln sich die Wege für GUI und CLI.
    // ============================================================
    void mergeSortRec(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        // Einziger Buffer für die GESAMTE Sortierung (verhindert Memory-Leaks/Fragmentierung)
        std::vector<int32_t> buffer(arr.size());

        if (cb) {
            // GUI-Modus
            mergeSortRecHelper<true>(arr, buffer, 0, static_cast<int32_t>(arr.size()) - 1, cb, m);
        } else {
            // CLI-Modus: Max Speed!
            mergeSortRecHelper<false>(arr, buffer, 0, static_cast<int32_t>(arr.size()) - 1, cb, m);
        }
    }

} // namespace Algorithms