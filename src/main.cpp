// ============================================================
// main.cpp – Einstiegspunkt (GUI & CLI Dual-Mode)
//
// C++20 Features:
// ┌──────────────────┬─────────────────────────────────────────┐
// │ std::span        │ Sichere Kapselung von C-Arrays (argv)   │
// │ std::string_view │ Zero-Allocation String-Vergleiche       │
// │ std::format      │ Moderner, typsicherer String-Builder    │
// │ std::ranges      │ Moderne Algorithmen (z.B. generate)     │
// └──────────────────┴─────────────────────────────────────────┘
// ============================================================
#include "Visualizer.hpp"
#include "SortAlgorithms.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> // WICHTIG: Löst den "WinMain" Linker-Fehler auf Windows!

#include <iostream>
#include <stdexcept>
#include <span>
#include <string_view>
#include <vector>
#include <random>
#include <chrono>
#include <format>
#include <algorithm>

// ── DATA ORIENTED DESIGN: Trennung von Daten und Logik ──────
// Ein reines Datenpaket (POD), das die Shell-Konfiguration hält.
struct CliConfig {
    bool             runCli{false};
    std::string_view algoName{"quicksort"};
    int32_t          arraySize{100000};
};

// ============================================================
// Argument Parser
// Nutzt std::span für sicheren Zugriff ohne Pointer-Arithmetik
// ============================================================
CliConfig parseArguments(std::span<char*> args)
{
    CliConfig config;

    // args[0] ist der Pfad zur .exe, wir starten bei 1
    for (size_t i = 1; i < args.size(); ++i)
    {
        std::string_view arg = args[i];

        if (arg == "--cli") {
            config.runCli = true;
        }
        else if (arg == "--algo" && i + 1 < args.size()) {
            config.algoName = args[++i];
        }
        else if (arg == "--size" && i + 1 < args.size()) {
            config.arraySize = std::atoi(args[++i]);
        }
    }
    return config;
}

// ============================================================
// Shell Benchmark Runner (Läuft ohne GUI)
// ============================================================
void runBenchmark(const CliConfig& config)
{
    std::cout << std::format("\n[ SortVisualizer - CLI Benchmark ]\n");
    std::cout << std::format("Algorithmus : {}\n", config.algoName);
    std::cout << std::format("Elemente    : {}\n", config.arraySize);
    std::cout << "Generiere Zufallsdaten...\n";

    // 1. Daten generieren (Modern C++20)
    std::vector<int32_t> data(config.arraySize);
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int32_t> dist(1, config.arraySize);
    std::ranges::generate(data, [&]{ return dist(rng); });

    // 2. Dummy Callback (Zero-Cost Abstraction)
    // Der Compiler wird diese leere Funktion inline komplett wegoptimieren.
    auto dummyCb = [](const std::vector<int32_t>&, int32_t, int32_t, std::string_view) {};

    LiveMetrics metrics;
    std::cout << "Starte Sortierung...\n\n";

    // 3. Zeitmessung & Ausführung
    auto start = std::chrono::high_resolution_clock::now();

    if (config.algoName == "quicksort") {
        SortAlgorithms::quickSort(data, dummyCb, metrics);
    } else if (config.algoName == "mergesort") {
        SortAlgorithms::mergeSort(data, dummyCb, metrics);
    } else if (config.algoName == "mergesort_it") {
        SortAlgorithms::mergeSortIt(data, dummyCb, metrics);
    } else if (config.algoName == "heapsort") {
        SortAlgorithms::heapSort(data, dummyCb, metrics);
    } else if (config.algoName == "radixsort") {
        SortAlgorithms::radixSort(data, dummyCb, metrics);
    } else if (config.algoName == "countingsort") {
        SortAlgorithms::countingSort(data, dummyCb, metrics);
    } else if (config.algoName == "bubblesort") {
        SortAlgorithms::bubbleSort(data, dummyCb, metrics);
    } else {
        std::cerr << std::format("Fehler: Unbekannter Algorithmus '{}'.\n", config.algoName);
        return;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 4. Ergebnisse ausgeben
    std::cout << std::format("=== ERGEBNISSE ===\n");
    std::cout << std::format("Benoetigte Zeit : {} ms\n", duration.count());
    std::cout << std::format("Vergleiche      : {}\n", metrics.comparisons);
    std::cout << std::format("Array-Zugriffe  : {}\n", metrics.arrayAccesses);
    std::cout << std::format("Tausch-Ops      : {}\n", metrics.swaps);
    std::cout << "\nBenchmark abgeschlossen.\n";
}

// ============================================================
// MAIN
// ============================================================
int main(int argc, char* argv[])
{
    try
    {
        // argv in std::span verpacken für absolute Memory-Safety
        std::span<char*> args{argv, static_cast<size_t>(argc)};

        CliConfig config = parseArguments(args);

        if (config.runCli)
        {
            // ── CLI Modus ── (Keine GUI, reine Konsolen-Power)
            runBenchmark(config);
        }
        else
        {
            // ── GUI Modus ── (Normaler Start)
            Visualizer vis;
            vis.run();
        }
    }
    catch (const std::exception& e)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fehler", e.what(), nullptr);
        return 1;
    }

    return 0;
}