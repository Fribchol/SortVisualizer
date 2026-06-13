// ============================================================
// MergeSortIt.cpp – Bottom-Up MergeSort, iterativ
//
// C++20 Features & Modernisierungen:
// ┌──────────────────────┬─────────────────────────────────────┐
// │ Anonymer Namespace   │ Ersetzt C-static für interne-Linkage│
// │ if constexpr         │ Compile-Time Branching (Zero-Cost)  │
// │ std::int32_t         │ Explizite Typen aus <cstdint>        │
// │ std::copy            │ Hardware-beschleunigtes Kopieren    │
// │ std::format          │ Typsicheres String-Formatting       │
// │ std::min             │ Sicheres Minimum ohne Makro-Overhead│
// └──────────────────────┴─────────────────────────────────────┘
//
// Data-Oriented Design (DOD) & Performance-Gewinn:
// Anstatt bei jedem Merge-Vorgang temporäre Vektoren auf dem Heap
// zu allozieren, fordern wir GENAU EINMAL zu Beginn einen Puffer
// in Array-Größe an. Das Verlegen der Daten geschieht sequenziell
// über schnelle Cache-Zeilen hinweg.
// ============================================================
#include "MergeSortIt.hpp"
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
                       std::vector<std::int32_t>& buffer, // Wiederverwendbarer DOD-Puffer
                       std::int32_t               left,
                       std::int32_t               mid,
                       std::int32_t               right,
                       const StepCallback&        cb,
                       LiveMetrics&               m)
        {
            // ── DOD: Hardware-beschleunigtes Kopieren per std::copy ──
            // Kopiert den aktuellen Arbeitsbereich in den pre-allocated Puffer.
            std::copy(arr.begin() + left, arr.begin() + right + 1, buffer.begin() + left);
            m.arrayAccesses += (right - left + 1) * 2; // 1 Read (arr), 1 Write (buffer) pro Element

            std::int32_t i = left;      // Lese-Index in der linken Hälfte (im Buffer)
            std::int32_t j = mid + 1;   // Lese-Index in der rechten Hälfte (im Buffer)
            std::int32_t k = left;      // Schreib-Index im originalen Ziel-Array (arr)

            // ── Hauptschleife: Kleinere Elemente sequenziell einsetzen ──
            while (i <= mid && j <= right)
            {
                ++m.comparisons;

                if (buffer[i] <= buffer[j])
                {
                    // Die Verwendung von <= statt < sichert die STABILITÄT des Algorithmus
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

            // ── Rest der linken Hälfte zurückkopieren ──
            // Hinweis zur DOD-Optimierung: Die rechte Hälfte muss nicht separat geleert werden,
            // da ihre verbleibenden Elemente im Ziel-Array 'arr' ohnehin schon am richtigen Fleck stehen!
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
        void mergeSortItImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
        {
            const std::int32_t n = static_cast<std::int32_t>(arr.size());

            // ── DOD: Ein einziger Puffer für den gesamten Algorithmus ──
            // Einmalige Heap-Allokation verhindert Speicherfragmentierung im Merge-Loop komplett.
            std::vector<std::int32_t> buffer(n);

            // Bottom-Up: Größe der Teilarrays verdoppelt sich in jeder Runde (1, 2, 4, 8, 16...)
            for (std::int32_t size = 1; size < n; size *= 2)
            {
                // Alle Paare der aktuellen Größe im Array abklappern
                for (std::int32_t left = 0; left < n - size; left += 2 * size)
                {
                    const std::int32_t mid   = left + size - 1;
                    // std::min stellt sicher, dass wir am Array-Ende nicht über die Grenzen hinausschießen
                    const std::int32_t right = std::min(left + 2 * size - 1, n - 1);

                    mergeImpl<EnableVisuals>(arr, buffer, left, mid, right, cb, m);
                }
            }
        }
    } // Ende des anonymen Namespaces

    // ============================================================
    // mergeSortIt – Öffentliche Schnittstelle
    // Der Dispatcher: Hier gabeln sich die Wege für GUI und CLI.
    // ============================================================
    void mergeSortIt(std::vector<std::int32_t>& arr, StepCallback cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        if (cb) {
            // GUI-Modus: Compiler generiert die Variante inklusive String-Formatting
            mergeSortItImpl<true>(arr, cb, m);
        } else {
            // CLI-Modus: Compiler generiert eine pure, hochoptimierte Funktion ohne jeden String-Ballast
            mergeSortItImpl<false>(arr, cb, m);
        }
    }

} // namespace Algorithms