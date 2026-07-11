#include "SortAlgorithms.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>

// ==================================================================
// Radixsort (LSD, Least Significant Digit first), Basis 10
// ==================================================================
//
// Siehe Skript "Algorithmen und Datenstrukturen", Kap. 4.4,
// Radixsort (Folien 281-286):
//
//   Problem bei Countingsort: ist ineffizient, wenn der Wertebereich
//   k groß ist im Vergleich zu n (Laufzeit/Speicher Θ(n + k)).
//
//   Idee Radixsort: Sortiere nicht nach dem gesamten Schlüssel auf
//   einmal, sondern ziffernweise, beginnend bei der niederwertigsten
//   Stelle (LSD). Für jede Ziffernposition wird ein STABILES
//   Sortierverfahren verwendet (hier: Countingsort mit Basis 10) -
//   Stabilität ist entscheidend für die Korrektheit (siehe
//   Induktionsbeweis auf Folie 284): Zahlen, die sich an der
//   aktuellen Ziffernposition gleichen, müssen ihre bisherige
//   (bereits korrekte) Relativordnung aus den niederwertigeren
//   Stellen behalten.
//
//   Anzahl Phasen: eine pro Dezimalstelle des größten Elements.
//
// WICHTIG (Vorbedingung, siehe Anmerkung Folie 282): Radixsort ist
// hier nur für NICHTNEGATIVE ganze Zahlen definiert. Bei negativen
// Werten wäre "Ziffer % 10" negativ und der darauffolgende Zugriff
// auf das count-Array undefiniertes Verhalten (nicht nur falsches
// Ergebnis!).
//
// Die Prüfung lief zuvor über assert() - das verschwindet in
// Release-Builds (NDEBUG) komplett und würde die Vorbedingung dann
// stillschweigend NICHT mehr absichern; ein Verstoß wäre dann echtes
// undefiniertes Verhalten statt eines kontrollierten Programmfehlers.
// Stattdessen wird die Vorbedingung jetzt als echte, immer aktive
// Laufzeitprüfung durchgesetzt (std::invalid_argument), damit ein
// falscher Aufruf in JEDER Build-Konfiguration kontrolliert
// fehlschlägt statt undefiniert zu sein.
//
// Data-Oriented Design: Sowohl der Ausgabepuffer "output" als auch
// das Histogramm "count" werden EINMAL pro Sortierlauf angelegt und
// über alle Ziffernphasen hinweg wiederverwendet - keine Allokation
// pro Phase.

namespace SortAlgorithms
{
    namespace
    {
        // In einen anonymen Namespace verschoben (Konsistenz mit den
        // übrigen Algorithmen): radixSortImpl ist reines
        // Implementierungsdetail und soll keine externe Bindung haben.

        // --------------------------------------------------------
        // radixSortImpl – LSD-Radixsort über alle Dezimalstellen
        // --------------------------------------------------------
        template <bool EnableVisuals>
        void radixSortImpl(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
        {
            // Vorbedingung: nur nichtnegative Schlüssel (siehe Kommentar
            // oben). Ein Verstoß wäre kein Sortierfehler, sondern
            // Undefined Behavior beim Zugriff auf "count" weiter unten -
            // daher eine echte, immer aktive Prüfung statt eines
            // reinen Debug-Assert.
            if (!std::ranges::all_of(arr, [](std::int32_t v) { return v >= 0; })) {
                throw std::invalid_argument(
                    "Radixsort (Basis 10) ist hier nur fuer nichtnegative Werte definiert.");
            }

            const auto maxVal = std::ranges::max(arr);
            const auto n      = static_cast<std::int32_t>(arr.size());

            // Wiederverwendeter Ausgabepuffer fuer alle Ziffernphasen -
            // RAII über std::vector, einmalige Allokation.
            std::vector<std::int32_t> output(static_cast<std::size_t>(n));

            // Histogramm der Ziffernhäufigkeiten (Basis 10 -> 10 Fächer),
            // wird in jeder Phase neu befüllt, aber nicht neu allokiert.
            std::array<std::int32_t, 10> count{};

            // Eine Phase pro Dezimalstelle des größten Elements. "exp"
            // wählt jeweils die aktuell betrachtete Ziffer aus:
            // exp=1 -> Einerstelle, exp=10 -> Zehnerstelle, usw.
            for (auto exp = 1; maxVal / exp > 0; exp *= 10)
            {
                // --- Countingsort bezüglich der aktuellen Ziffer ---

                // Schritt 1: Histogramm zurücksetzen und Ziffernhäufigkeiten
                // zählen (wie viele Elemente haben Ziffer 0, 1, ..., 9 an
                // Position "exp"?).
                std::ranges::fill(count, 0);
                for (auto i = 0; i < n; ++i)
                {
                    const auto digit = (arr[static_cast<std::size_t>(i)] / exp) % 10;
                    ++count[static_cast<std::size_t>(digit)];
                    ++m.arrayAccesses;
                }

                // Schritt 2: Präfixsummen bilden. count[d] gibt danach an,
                // wie viele Elemente eine Ziffer <= d an dieser Position
                // haben - das ist zugleich die (um 1 verschobene) Zielposition
                // im Ausgabepuffer für das jeweils LETZTE Element mit dieser
                // Ziffer.
                for (auto i = 1; i < 10; ++i)
                {
                    count[static_cast<std::size_t>(i)] += count[static_cast<std::size_t>(i - 1)];
                }

                // Schritt 3: Rückwärts (von n-1 bis 0) durch das Array laufen
                // und jedes Element an seine Zielposition im Ausgabepuffer
                // schreiben. Die Rückwärtsrichtung ist der Schlüssel zur
                // STABILITÄT: Von zwei Elementen mit gleicher Ziffer behält
                // so dasjenige, das in "arr" weiter vorne stand, auch im
                // Ausgabepuffer die vordere Position - und damit bleibt die
                // Sortierung bezüglich der niederwertigeren, bereits
                // sortierten Stellen erhalten (Induktionsschritt, Folie 284).
                for (auto i = n - 1; i >= 0; --i)
                {
                    const auto digit = (arr[static_cast<std::size_t>(i)] / exp) % 10;
                    const auto pos   = --count[static_cast<std::size_t>(digit)];
                    output[static_cast<std::size_t>(pos)] = arr[static_cast<std::size_t>(i)];
                    ++m.arrayAccesses;
                }

                // Schritt 4: Ergebnis dieser Phase zurück nach "arr" kopieren,
                // damit die nächste Phase (nächsthöhere Ziffer) wieder auf
                // "arr" aufsetzen kann. Das ist ein reines Kopieren, kein
                // Elementtausch - daher wird hier bewusst NICHT m.swaps
                // hochgezählt (anders als z.B. bei BubbleSort/QuickSort,
                // wo "swaps" echte Vertauschungen zweier Positionen zählt).
                for (auto i = 0; i < n; ++i)
                {
                    arr[static_cast<std::size_t>(i)] = output[static_cast<std::size_t>(i)];
                    ++m.arrayAccesses;

                    if constexpr (EnableVisuals) {
                        cb(arr, i, -1);
                    }
                }
            }
        }
    } // namespace

    // --------------------------------------------------------
    // radixSort – öffentliche Schnittstelle
    // --------------------------------------------------------
    void radixSort(std::vector<std::int32_t>& arr, const StepCallback& cb, LiveMetrics& m)
    {
        if (arr.empty()) return;

        // Compile-Zeit-Entscheidung wie bei den MergeSort-Varianten:
        // Mit Callback -> Visualisierungsschritte, ohne Callback
        // (reiner Benchmark) -> kein Laufzeit-Overhead durch if constexpr.
        if (cb) {
            radixSortImpl<true>(arr, cb, m);
        } else {
            radixSortImpl<false>(arr, cb, m);
        }
    }

} // namespace SortAlgorithms