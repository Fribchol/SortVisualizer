// ============================================================
// MergeSortRec.cpp – Top-Down MergeSort, rekursiv
//
// C++20 Features & Modernisierungen:
// ┌──────────────────────┬─────────────────────────────────────┐
// │ Anonymer Namespace   │ Ersetzt C-static für interne Linkage│
// │ if constexpr         │ Compile-Time Branching (Zero-Cost)  │
// │ std::int32_t         │ Explizite Typen aus <cstdint>       │
// │ Einmalige Allocation │ Ein einziger Buffer für den ganzen  │
// │                      │ Rekursionsbaum -> Max Performance!  │
// │ std::copy            │ Hardware-beschleunigtes Kopieren    │
// │ std::format          │ Typsicheres String-Formatting       │
// └──────────────────────┴─────────────────────────────────────┘
//
// Data-Oriented Design (DOD):
// Wie beim iterativen MergeSort fordern wir genau EINEN Puffer
// zu Beginn an und reichen ihn durch den Rekursionsbaum durch.
// Das verhindert das ständige Anfordern und Freigeben von Heap-
// Speicher und maximiert den Cache-Speicherdurchsatz.
// ============================================================
#include "MergeSortRec.hpp"
#include <algorithm>
#include <format>
#include <cstdint> // C++ Standard für feste Integer-Breiten

namespace Algorithms
{
    // ── Anonymer Namespace ──────────────────────────────────────
    // Ersetzt 'static' für interne Funktionen. Alles hierin
    // ist strikt nur in dieser Translation Unit (.cpp) sichtbar.
    namespace
    {
        // ============================================================
        // mergeImpl – Zwei sortierte Hälften zusammenführen (Template)
        // ============================================================
        template <bool EnableVisuals>
        void mergeImpl(std::vector<std::int32_t>& arr,
                       std::vector<std::int32_t>& buffer, // Wiederverwendbarer DOD-Puffer!
                       std::int32_t               left,
                       std::int32_t               mid,
                       std::int32_t               right,
                       const StepCallback&        cb,
                       LiveMetrics&               m)
        {
            // ── DOD: Kopieren per std::copy (oft memmove intern) ──
            // Wir kopieren den relevanten Bereich aus arr in unseren Puffer.
            // Das ist millionenfach schneller, als jedes Mal einen neuen std::vector zu erstellen.
            std::copy(arr.begin() + left, arr.begin() + right + 1, buffer.begin() + left);
            m.arrayAccesses += (right - left + 1) * 2; // Read (arr) + Write (buffer)

            std::int32_t i = left;      // Lese-Index linke Hälfte (im Buffer)
            std::int32_t j = mid + 1;   // Lese-Index rechte Hälfte (im Buffer)
            std::int32_t k = left;      // Schreib-Index im originalen arr

            // ── Hauptschleife: Das kleinere Element zurück in arr schreiben ──
            while (i <= mid && j <= right)
            {
                ++m.comparisons;

                if (buffer[i] <= buffer[j])
                {
                    // <= sorgt für die Stabilität des Algorithmus
                    arr[k] = buffer[i++];
                    ++m.arrayAccesses;

                    if constexpr (EnableVisuals) {
                        cb(arr, k, -1, std::format(
                            "Zusammenführen (rekursiv): {} ist kleiner oder gleich -> Position {}",
                            arr[k], k));
                    }
                }
                else
                {
                    arr[k] = buffer[j++];
                    ++m.arrayAccesses;

                    if constexpr (EnableVisuals) {
                        cb(arr, k, -1, std::format(
                            "Zusammenführen (rekursiv): {} ist kleiner -> Position {}",
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
            // (Diese Schleife läuft nur, wenn rechts Elemente übrig sind)
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
        void mergeSortRecHelper(std::vector<std::int32_t>& arr,
                                std::vector<std::int32_t>& buffer,
                                std::int32_t               left,
                                std::int32_t               right,
                                const StepCallback&        cb,
                                LiveMetrics&               m)
        {
            // Basisfall: Teilarray ist leer oder hat nur 1 Element
            if (left >= right) return;

            // Overflow-sichere Mittenberechnung
            const std::int32_t mid = left + (right - left) / 2;

            // Teile (Divide)
            mergeSortRecHelper<EnableVisuals>(arr, buffer, left, mid, cb, m);
            mergeSortRecHelper<EnableVisuals>(arr, buffer, mid + 1, right, cb, m);

            // Herrsche & Fuge zusammen (Conquer & Merge)
            mergeImpl<EnableVisuals>(arr, buffer, left, mid, right, cb, m);
        }
    } // Ende des anonymen Namespaces

    // ============================================================
    // mergeSortRec – Öffentliche Schnittstelle
    // Der Dispatcher: Hier gabeln sich die Wege für GUI und CLI.
    // ============================================================
    void mergeSortRec(std::vector<std::int32_t>& arr, StepCallback cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        // Einziger Buffer für die GESAMTE Sortierung (verhindert Memory-Leaks und Heap-Fragmentierung)
        std::vector<std::int32_t> buffer(arr.size());

        if (cb) {
            // GUI-Modus: Compiler generiert Variante mit Strings
            mergeSortRecHelper<true>(arr, buffer, 0, static_cast<std::int32_t>(arr.size()) - 1, cb, m);
        } else {
            // CLI-Modus: Max Speed! (Zero-Cost Abstraction)
            mergeSortRecHelper<false>(arr, buffer, 0, static_cast<std::int32_t>(arr.size()) - 1, cb, m);
        }
    }

} // namespace Algorithms