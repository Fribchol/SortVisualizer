// ============================================================
// MergeSortRec.cpp – Top-Down MergeSort, rekursiv (Modern C++)
// ============================================================
// Modern C++20/C++23 & Data-Oriented Design Richtlinien:
// ┌───────────────────┬────────────────────────────────────────────────────────┐
// │ Cache-Lokalität   │ Zusammenhängender Vektorspeicher im Puffer, lineares   │
// │                   │ Kopieren von Speicherblöcken zur Optimierung der L1/L2 │
// │                   │ Cache-Zugriffe.                                        │
// ├───────────────────┼────────────────────────────────────────────────────────┤
// │ RAII              │ Temporärer Puffer wird als Stack/Heap-Vektor lokal     │
// │                   │ allokiert und beim Verlassen automatisch freigegeben.  │
// └───────────────────┴────────────────────────────────────────────────────────┘

#include "MergeSortRec.hpp"
#include <span>
#include <vector>
#include <cstdint>
#include <algorithm> // Notwendig für std::copy

namespace Algorithms
{
    namespace
    {
        template <bool EnableVisuals>
        void mergeImpl(std::vector<std::int32_t>& arr,
                       std::span<std::int32_t>    bufferView,
                       std::size_t                left,
                       std::size_t                mid,
                       std::size_t                right,
                       const StepCallback&        cb,
                       LiveMetrics&               m)
        {
            // Bestimme die Gesamtlänge des zu betrachtenden Teilbereichs
            const std::size_t length = right - left + 1;

            // Kopiere den zu sortierenden Bereich in den temporären Puffer
            // Data Oriented Design: Lineare Speicherblöcke werden kopiert (hohe Cache-Effizienz)
            std::copy(arr.begin() + static_cast<std::ptrdiff_t>(left),
                      arr.begin() + static_cast<std::ptrdiff_t>(right) + 1,
                      bufferView.begin() + static_cast<std::ptrdiff_t>(left));
            m.arrayAccesses += static_cast<std::int64_t>(length) * 2;

            // Initialisiere die Zeiger für die linke und rechte sortierte Hälfte
            std::size_t i = left;          // Zeiger für linke Hälfte im Puffer
            std::size_t j = mid + 1;       // Zeiger für rechte Hälfte im Puffer
            std::size_t k = left;          // Zielzeiger für das Original-Array

            // Verschmelze die beiden Teillisten, solange Elemente in beiden vorhanden sind
            while (i <= mid && j <= right)
            {
                ++m.comparisons;
                // Vergleich der beiden Elemente aus dem Zwischenpuffer
                if (bufferView[i] <= bufferView[j])
                {
                    arr[static_cast<std::size_t>(k)] = bufferView[i];
                    ++i;
                    ++m.arrayAccesses;

                    if constexpr (EnableVisuals) {
                        // Übergabe des Arrays sowie des aktuellen Zielindex (k) und Quellindex (i)
                        cb(arr, static_cast<std::int32_t>(k), static_cast<std::int32_t>(i));
                    }
                }
                else
                {
                    arr[static_cast<std::size_t>(k)] = bufferView[j];
                    ++j;
                    ++m.arrayAccesses;

                    if constexpr (EnableVisuals) {
                        cb(arr, static_cast<std::int32_t>(k), static_cast<std::int32_t>(j));
                    }
                }
                ++k;
            }

            // Restliche Elemente der linken Hälfte übernehmen (falls vorhanden)
            while (i <= mid)
            {
                arr[static_cast<std::size_t>(k)] = bufferView[i];
                ++i;
                ++m.arrayAccesses;

                if constexpr (EnableVisuals) {
                    cb(arr, static_cast<std::int32_t>(k), static_cast<std::int32_t>(i));
                }
                ++k;
            }

            // Restliche Elemente der rechten Hälfte übernehmen (falls vorhanden)
            while (j <= right)
            {
                arr[static_cast<std::size_t>(k)] = bufferView[j];
                ++j;
                ++m.arrayAccesses;

                if constexpr (EnableVisuals) {
                    cb(arr, static_cast<std::int32_t>(k), static_cast<std::int32_t>(j));
                }
                ++k;
            }
        }

        // mergeSortRange – Rekursive Teilung über direkte Sichten (Span)
        // Der redundante Offset-Parameter wurde entfernt und wird relativ zur Sicht ermittelt.
        template <bool EnableVisuals>
        void mergeSortRange(std::vector<std::int32_t>& arr,
                            std::span<std::int32_t>    fullBufferView,
                            std::span<std::int32_t>    subArrayView,
                            const StepCallback&        cb,
                            LiveMetrics&               m)
        {
            // Terminierungsbedingung der Rekursion: Wenn Teilbereichslänge <= 1
            if (subArrayView.size() <= 1) {
                return;
            }

            // Berechne den Mittelpunkt (Teilen/Divide)
            const std::size_t midOffset = subArrayView.size() / 2;

            // Teile die Sicht in eine linke und rechte Teilsicht auf
            std::span<std::int32_t> leftSubView  = subArrayView.first(midOffset);
            std::span<std::int32_t> rightSubView = subArrayView.last(subArrayView.size() - midOffset);

            // Abstieg auf die Teilsichten
            mergeSortRange<EnableVisuals>(arr, fullBufferView, leftSubView, cb, m);
            mergeSortRange<EnableVisuals>(arr, fullBufferView, rightSubView, cb, m);

            // Aufsteigende Verschmelzung (Conquer & Combine) über Pointer-Differenz
            const std::size_t absStartIdx = static_cast<std::size_t>(subArrayView.data() - arr.data());
            const std::size_t absMid      = absStartIdx + leftSubView.size() - 1;
            const std::size_t absRight    = absStartIdx + subArrayView.size() - 1;

            mergeImpl<EnableVisuals>(arr, fullBufferView, absStartIdx, absMid, absRight, cb, m);
        }
        // NOLINTEND(readability-function-cognitive-complexity, bugprone-recursive-recursion, misc-no-recursion)

    } // namespace

    // --------------------------------------------------------
    // mergeSortRec – Öffentliche Schnittstelle (RAII Kapselung)
    // --------------------------------------------------------
    void mergeSortRec(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        // Abbruchbedingung bei leeren Arrays oder Array-Größe 1 (bereits sortiert)
        if (arr.size() < 2) {
            return;
        }

        // RAII: Initialisiere temporären Puffer auf dem Heap
        std::vector<std::int32_t> buffer(arr.size());
        std::span<std::int32_t>   bufferView(buffer);
        std::span<std::int32_t>   mainView(arr);

        // NOLINTNEXTLINE(readability-static-accessed-through-instance, clang-analyzer-core.CallAndMessage)
        if (cb) {
            mergeSortRange<true>(arr, bufferView, mainView, cb, m);
        } else {
            mergeSortRange<false>(arr, bufferView, mainView, cb, m);
        }
    }

} // namespace Algorithms