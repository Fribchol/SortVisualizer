#include "SortAlgorithms.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <cstdint>
#include <limits>
#include <array>
#include <string_view>

// Alias-Namespace für sauberere IO-Operationen
namespace IO
{
    auto& in  = std::cin;
    auto& out = std::cout;

    void clearBuffer() {
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

// Szenario-Typ für die Array-Generierung (DOD: reiner Datenzustand, keine Logik gekoppelt)
enum class ArrayScenario : std::uint8_t
{
    Random = 0,
    Sorted,
    Reversed,
    Equal
};

[[nodiscard]] constexpr std::string_view scenarioName(const ArrayScenario scenario) noexcept
{
    switch (scenario) {
        case ArrayScenario::Random:   return "Zufaellig";
        case ArrayScenario::Sorted:   return "Sortiert";
        case ArrayScenario::Reversed: return "Umgekehrt sortiert";
        case ArrayScenario::Equal:    return "Gleichgross";
        default:                      return "Unbekannt";
    }
}

// Erzeugt das Test-Array passend zum gewaehlten Szenario
[[nodiscard]] std::vector<std::int32_t> generateArray(const std::int32_t size, const ArrayScenario scenario)
{
    std::vector<std::int32_t> data(static_cast<std::size_t>(size));

    switch (scenario)
    {
        case ArrayScenario::Random:
        {
            std::mt19937 rng(1337);
            std::uniform_int_distribution<std::int32_t> dist(1, 1000000);
            std::ranges::generate(data, [&]() { return dist(rng); });
            break;
        }
        case ArrayScenario::Sorted:
        {
            std::iota(data.begin(), data.end(), 1);
            break;
        }
        case ArrayScenario::Reversed:
        {
            std::iota(data.begin(), data.end(), 1);
            std::ranges::reverse(data);
            break;
        }
        case ArrayScenario::Equal:
        {
            std::ranges::fill(data, 42);
            break;
        }
    }

    return data;
}

void runBenchmarkFor(const std::uint8_t algoIndex, const std::int32_t size, const ArrayScenario scenario)
{
    IO::out << "\nInitialisiere Array mit n = " << size
            << " Elementen (" << scenarioName(scenario) << ")...\n";

    std::vector<std::int32_t> testArray = generateArray(size, scenario);

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
            IO::out << "Ungueltiger Algorithmus!\n";
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
    std::int32_t   arraySize = 50000;
    ArrayScenario  scenario  = ArrayScenario::Random;

    while (true)
    {
        IO::out << "****************************************\n"
                << "       Konsolen Benchmark               \n"
                << "****************************************\n"
                << "Algorithmus aussuchen:\n"
                << " [0] QuickSort | [1] MergeSort (rek) | [2] MergeSort (it)\n"
                << " [3] HeapSort  | [4] RadixSort       | [5] CountingSort\n"
                << " [6] BubbleSort\n"
                << "****************************************\n"
                << " [7] Array (n) anpassen\n"
                << " [8] Szenario wechseln (Zufaellig/Sortiert/Umgekehrt/Gleichgross)\n"
                << "****************************************\n"
                << "Aktuelles Array (n): " << arraySize << "\n"
                << "Aktuelles Szenario : " << scenarioName(scenario) << "\n"
                << "Ihre Auswahl (0-8): ";

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
            runBenchmarkFor(static_cast<std::uint8_t>(choice), arraySize, scenario);
        }
        else if (choice == 7) {
            IO::out << "\nNeue Array-Groesse fuer n eingeben: ";
            IO::in >> arraySize;
            IO::clearBuffer();

            if (arraySize <= 0) {
                IO::out << "[Fehler] Array muss > 0 sein! Setze auf 50000\n";
                arraySize = 50000;
            }
            IO::out << "[Info] Array auf n=" << arraySize << " gesetzt.\n\n";
        }
        else if (choice == 8) {
            static constexpr std::array<ArrayScenario, 4> scenarios{
                ArrayScenario::Random, ArrayScenario::Sorted,
                ArrayScenario::Reversed, ArrayScenario::Equal
            };

            IO::out << "\nSzenario auswaehlen:\n"
                    << " [0] Zufaellig\n"
                    << " [1] Sortiert\n"
                    << " [2] Umgekehrt sortiert\n"
                    << " [3] Gleichgross\n"
                    << "Ihre Auswahl (0-3): ";

            std::int32_t scenarioChoice = -1;
            IO::in >> scenarioChoice;

            if (IO::in.fail() || scenarioChoice < 0 || scenarioChoice > 3) {
                IO::in.clear();
                IO::clearBuffer();
                IO::out << "[Fehler] Ungueltige Auswahl. Szenario bleibt unveraendert.\n\n";
                continue;
            }

            IO::clearBuffer();
            scenario = scenarios[static_cast<std::size_t>(scenarioChoice)];
            IO::out << "[Info] Szenario auf '" << scenarioName(scenario) << "' gesetzt.\n\n";
        }
        else {
            IO::out << "[Fehler] Falsche Auswahl!\n";
        }
    }
}