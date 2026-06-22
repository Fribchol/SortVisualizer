// ============================================================
// terminal_main.cpp – Interaktives Konsolen-Interface
// ============================================================

#include "SortAlgorithms.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>

void runBenchmarkFor(std::uint8_t algoIndex, std::int32_t size)
{
    std::cout << "\nInitialisiere Array mit n = " << size << " Elementen (Zufallsszenario)..." << std::endl;
    std::vector<std::int32_t> testArray(static_cast<std::size_t>(size));

    // Konstanten Seed für Benchmark-Vergleichbarkeit gewollt -> Warnung wird unterdrückt
    // NOLINTNEXTLINE(cert-msc51-cpp, cppcoreguidelines-pro-type-member-init)
    std::mt19937 rng(1337);
    std::uniform_int_distribution<std::int32_t> dist(1, 1000000);

    // Modernes C++20 Ranges-Verhalten angewendet
    std::ranges::generate(testArray, [&]() { return dist(rng); });

    LiveMetrics metrics{};
    auto start = std::chrono::high_resolution_clock::now();

    // Algorithmus anhand der Index-Tabelle ausführen
    switch (algoIndex) {
        case 0: SortAlgorithms::quickSort(testArray, nullptr, metrics); break;
        case 1: SortAlgorithms::mergeSort(testArray, nullptr, metrics); break;
        case 2: SortAlgorithms::mergeSortIt(testArray, nullptr, metrics); break;
        case 3: SortAlgorithms::heapSort(testArray, nullptr, metrics); break;
        case 4: SortAlgorithms::radixSort(testArray, nullptr, metrics); break;
        case 5: SortAlgorithms::countingSort(testArray, nullptr, metrics); break;
        case 6: SortAlgorithms::bubbleSort(testArray, nullptr, metrics); break;
        default: std::cout << "Ungültiger Algorithmus!" << std::endl; return;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "\n--- Ergebnis ---" << std::endl;
    std::cout << "Dauer:         " << duration << " ms" << std::endl;
    std::cout << "Vergleiche:    " << metrics.comparisons << std::endl;
    std::cout << "Speicherz.:    " << metrics.arrayAccesses << std::endl;

    if (std::ranges::is_sorted(testArray)) {
        std::cout << "[Status] Korrekt sortiert!\n" << std::endl;
    } else {
        std::cout << "[FEHLER] Array ist nicht sortiert!\n" << std::endl;
    }
}

int main()
{
    std::int32_t arraySize = 50000;

    // Endlosschleife ist für Konsolenanwendung beabsichtigt -> Warnung unterdrücken
    // NOLINTNEXTLINE(hicpp-no-assembler, google-readability-braces-around-statements)
    while (true)
    {
        std::cout << "========================================" << std::endl;
        std::cout << "       HIGH-PERFORMANCE BENCHMARK       " << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Algorithmus aussuchen:" << std::endl;
        std::cout << " [0] QuickSort" << std::endl;
        std::cout << " [1] MergeSort (rekursiv)" << std::endl;
        std::cout << " [2] MergeSort (iterativ)" << std::endl;
        std::cout << " [3] HeapSort" << std::endl;
        std::cout << " [4] RadixSort" << std::endl;
        std::cout << " [5] CountingSort" << std::endl;
        std::cout << " [6] BubbleSort" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        std::cout << " [7] Array (n) anpassen" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "Aktuelles Array (n): " << arraySize << std::endl;
        std::cout << "Ihre Auswahl (0-7): ";

        std::int32_t choice = -1;
        std::cin >> choice;

        // Eingabepuffer leeren, um Endlosschleifen bei Fehleingaben (z.B. Buchstaben) zu verhindern
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "[Fehler] Falsche Eingabe. Bitte eine Ziffer eingeben.\n" << std::endl;
            continue;
        }

        // Puffer bereinigen, falls versehentlich zu viel eingegeben wurde
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice >= 0 && choice <= 6) {
            std::cout << "\nStarte Benchmark..." << std::endl;
            runBenchmarkFor(static_cast<std::uint8_t>(choice), arraySize);
        }
        else if (choice == 7) {
            std::cout << "\nNeue Array-Größe für n eingeben (z.B. 100000): ";
            std::cin >> arraySize;

            // Puffer nach dem Einlesen der Array-Größe leeren
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (arraySize <= 0) {
                std::cout << "[Fehler] Array muss > 0 sein! Setze auf Standard: 50000" << std::endl;
                arraySize = 50000;
            }
            std::cout << "[Info] Array erfolgreich auf n=" << arraySize << " gesetzt.\n" << std::endl;
        }
        else {
            std::cout << "[Fehler] Falsche Auswahl! Bitte wählen Sie 0 bis 7." << std::endl;
        }

        // Nach Ausführung des Algorithmus oder Änderung der Größe geht es direkt zurück zum Hauptmenü (Schleife beginnt von vorn).
    }
}