#include "SortAlgorithms.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>

// Alias-Namespace für sauberere IO-Operationen
namespace IO
{
    // Referenzierung der std:: Objekte direkt
    auto& in  = std::cin;
    auto& out = std::cout;

    // Hilfsfunktion zum Leeren des Buffers (DOD/Clean Code Prinzip)
    void clearBuffer() {
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void runBenchmarkFor(const std::uint8_t algoIndex, const std::int32_t size)
{
    IO::out << "\nInitialisiere Array mit n = " << size << " Elementen (Zufallsszenario)...\n";

    std::vector<std::int32_t> testArray(static_cast<std::size_t>(size));

    std::mt19937 rng(1337);
    std::uniform_int_distribution<std::int32_t> dist(1, 1000000);

    std::ranges::generate(testArray, [&]() { return dist(rng); });

    SortAlgorithms::LiveMetrics metrics{};
    const auto start = std::chrono::high_resolution_clock::now();

    switch (algoIndex) {
        case 0: SortAlgorithms::quickSort(testArray, nullptr, metrics); break;
        case 1: SortAlgorithms::mergeSort(testArray, nullptr, metrics); break;
        case 2: SortAlgorithms::mergeSortIt(testArray, nullptr, metrics); break;
        case 3: SortAlgorithms::heapSort(testArray, nullptr, metrics); break;
        case 4: SortAlgorithms::radixSort(testArray, nullptr, metrics); break;
        case 5: SortAlgorithms::countingSort(testArray, nullptr, metrics); break;
        case 6: SortAlgorithms::bubbleSort(testArray, nullptr, metrics); break;
        default:
            IO::out << "Ungültiger Algorithmus!\n";
            return;
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    IO::out << "\n--- Ergebnis ---\n"
            << "Dauer:         " << duration << " ms\n"
            << "Vergleiche:    " << metrics.comparisons << "\n"
            << "Speicherz.:    " << metrics.arrayAccesses << "\n";

    if (std::ranges::is_sorted(testArray)) {
        IO::out << "[Status] Korrekt sortiert!\n\n";
    } else {
        IO::out << "[FEHLER] Array ist nicht sortiert!\n\n";
    }
}

int main()
{
    std::int32_t arraySize = 50000;

    while (true)
    {
        // Konsolen Ausgaben
        IO::out << "****************************************\n"
                << "       Konsolen Benchmark               \n"
                << "****************************************\n"
                << "Algorithmus aussuchen:\n"
                << " [0] QuickSort | [1] MergeSort (rek) | [2] MergeSort (it)\n"
                << " [3] HeapSort  | [4] RadixSort       | [5] CountingSort\n"
                << " [6] BubbleSort\n"
                << "****************************************\n"
                << " [7] Array (n) anpassen\n"
                << "****************************************\n"
                << "Aktuelles Array (n): " << arraySize << "\n"
                << "Ihre Auswahl (0-7): ";

        std::int32_t choice = -1;
        IO::in >> choice;

        if (IO::in.fail()) {
            IO::in.clear();
            IO::clearBuffer();
            IO::out << "[Fehler] Falsche Eingabe. Bitte eine Ziffer eingeben.\n\n";
            continue;
        }

        IO::clearBuffer();

        if (choice >= 0 && choice <= 6) {
            IO::out << "\nStarte Benchmark...\n";
            runBenchmarkFor(static_cast<std::uint8_t>(choice), arraySize);
        }
        else if (choice == 7) {
            IO::out << "\nNeue Array-Größe für n eingeben: ";
            IO::in >> arraySize;
            IO::clearBuffer();

            if (arraySize <= 0) {
                IO::out << "[Fehler] Array muss > 0 sein! Setze auf 50000\n";
                arraySize = 50000;
            }
            IO::out << "[Info] Array auf n=" << arraySize << " gesetzt.\n\n";
        }
        else {
            IO::out << "[Fehler] Falsche Auswahl!\n";
        }
    }
}