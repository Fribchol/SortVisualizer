// MergeSortRec.cpp – Top-Down MergeSort, rekursiv (Modern C++)
// Prinzip: "Teile und Herrsche" (Divide and Conquer)
// 1. Teile das Problem in zwei Hälften.
// 2. Sortiere die Hälften rekursiv.
// 3. Verschmelze (Merge) die sortierten Hälften.


#include "MergeSortRec.hpp"
#include <span>
#include <vector>
#include <cstdint>


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
            std::ranges::copy(arr.begin() + static_cast<std::ptrdiff_t>(left),
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
        // Unterdrücke Clang-Tidy Rekursions- und Parameterwarnungen in akademischer Implementierung
        template <bool EnableVisuals>
        void mergeSortRange(std::vector<std::int32_t>& arr,
                            std::span<std::int32_t>    fullBufferView,
                            std::size_t                absStartOffset,
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

            // Rekursiver Abstieg auf die Teilsichten
            // Der linke Teilbereich beginnt relativ am selben Offset wie das Elternelement.
            mergeSortRange<EnableVisuals>(arr, fullBufferView, absStartOffset, leftSubView, cb, m);

            // Der rechte Teilbereich verschiebt den absoluten Offset zur Laufzeit
            // um die Größe des linken Teilbereichs. Die Konstante 0 im Einstieg entfällt.
            const std::size_t rightSubViewOffset = absStartOffset + leftSubView.size();
            mergeSortRange<EnableVisuals>(arr, fullBufferView, rightSubViewOffset, rightSubView, cb, m);

            // Aufsteigende Verschmelzung (Conquer & Combine) über berechnete absolute Startindizes
            const std::size_t absMid   = absStartOffset + leftSubView.size() - 1;
            const std::size_t absRight = absStartOffset + subArrayView.size() - 1;

            mergeImpl<EnableVisuals>(arr, fullBufferView, absStartOffset, absMid, absRight, cb, m);
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
            mergeSortRange<true>(arr, bufferView, 0, mainView, cb, m);
        } else {
            mergeSortRange<false>(arr, bufferView, 0, mainView, cb, m);
        }
    }

} // namespace Algorithms