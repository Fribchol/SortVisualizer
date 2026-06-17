// ============================================================
// main.cpp – Einstiegspunkt (GUI & CLI Dual-Mode)
//
// C++20 Features & Modernisierungen:
// ┌──────────────────┬─────────────────────────────────────────┐
// │ Anonymer Bereich │ namespace { } statt globale Variablen   │
// │ std::span        │ Sichere Kapselung von C-Arrays (argv)   │
// │ std::unique_ptr  │ Memory-Safety für das Visualizer-Objekt │
// │ std::from_chars  │ Modernes, allokationsfreies Parsing     │
// └──────────────────┴─────────────────────────────────────────┘
// ============================================================
#include "Visualizer.hpp"
#include "SortAlgorithms.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> // WICHTIG: Löst den "WinMain" Linker-Fehler auf Windows!

#include <iostream>
#include <vector>
#include <string_view>
#include <random>
#include <chrono>
#include <format>
#include <algorithm>
#include <charconv>  // Für std::from_chars
#include <cstdint>   // Für std::int32_t
#include <span>

// ── Anonymer Namespace ──────────────────────────────────────
namespace {

    using ArgsSpan = std::span<char*>;

    struct CliConfig {
        bool             runCli{false};
        std::string_view algoName{"quicksort"};
        std::int32_t     arraySize{100'000};
    };

    CliConfig parseArguments(ArgsSpan args)
    {
        CliConfig config;
        for (std::size_t i = 1; i < args.size(); ++i)
        {
            std::string_view arg = args[i];

            if (arg == "--cli") {
                config.runCli = true;
            }
            else if (arg == "--algo" && (i + 1) < args.size()) {
                config.algoName = args[++i];
            }
            else if (arg == "--size" && (i + 1) < args.size()) {
                std::string_view sizeStr = args[++i];
                std::from_chars(sizeStr.data(), sizeStr.data() + sizeStr.size(), config.arraySize);
            }
        }
        return config;
    }

    void runBenchmark(const CliConfig& config)
    {
        std::cout << std::format("\n[ SortVisualizer - CLI Benchmark ]\n");
        std::cout << std::format("Algorithmus : {}\n", config.algoName);
        std::cout << std::format("Elemente    : {}\n", config.arraySize);
        std::cout << "Generiere Zufallsdaten...\n";

        std::vector<std::int32_t> data(static_cast<std::size_t>(config.arraySize));
        std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<std::int32_t> dist(1, config.arraySize);
        std::ranges::generate(data, [&]{ return dist(rng); });

        // Angepasst: Das Callback akzeptiert nur noch 3 Parameter (Array, indexA, indexB)
        auto dummyCb = [](const std::vector<std::int32_t>&, std::int32_t, std::int32_t) {};

        LiveMetrics metrics;
        std::cout << "Starte Sortierung...\n\n";

        auto start = std::chrono::high_resolution_clock::now();

        if (config.algoName == "quicksort")      SortAlgorithms::quickSort(data, dummyCb, metrics);
        else if (config.algoName == "mergesort") SortAlgorithms::mergeSort(data, dummyCb, metrics);
        else if (config.algoName == "mergesort_it") SortAlgorithms::mergeSortIt(data, dummyCb, metrics);
        else if (config.algoName == "heapsort")  SortAlgorithms::heapSort(data, dummyCb, metrics);
        else if (config.algoName == "radixsort") SortAlgorithms::radixSort(data, dummyCb, metrics);
        else if (config.algoName == "countingsort") SortAlgorithms::countingSort(data, dummyCb, metrics);
        else if (config.algoName == "bubblesort") SortAlgorithms::bubbleSort(data, dummyCb, metrics);
        else {
            std::cerr << std::format("Fehler: Unbekannter Algorithmus '{}'.\n", config.algoName);
            return;
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << std::format("=== ERGEBNISSE ===\n");
        std::cout << std::format("Benötigte Zeit : {} ms\n", duration.count());
        std::cout << std::format("Vergleiche     : {}\n", metrics.comparisons);
        std::cout << std::format("Array-Zugriffe : {}\n", metrics.arrayAccesses);
        std::cout << std::format("Tausch-Ops     : {}\n", metrics.swaps);
        std::cout << "\nBenchmark abgeschlossen.\n";
    }

} // namespace

int main(int argc, char* argv[])
{
    try
    {
        ArgsSpan args{argv, static_cast<std::size_t>(argc)};
        CliConfig config = parseArguments(args);

        if (config.runCli) {
            runBenchmark(config);
        } else {
            auto vis = std::make_unique<Visualizer>();
            vis->run();
        }
    }
    catch (const std::exception& e)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Kritischer Fehler", e.what(), nullptr);
        return 1;
    }
    return 0;
}