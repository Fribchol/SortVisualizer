// ============================================================
// main.cpp – Einstiegspunkt (GUI & CLI Dual-Mode)
//
// C++20 Features & Modernisierungen:
// ┌──────────────────┬─────────────────────────────────────────┐
// │ Anonymer Bereich │ namespace { } statt globale Variablen   │
// │ std::span        │ Sichere Kapselung von C-Arrays (argv)   │
// │ using            │ Modernes Alias (statt typedef)          │
// │ std::unique_ptr  │ Smart Pointer für Memory-Safety         │
// │ Digit Separator  │ 100'000 statt 100000 (bessere Lesbarkeit)│
// │ std::from_chars  │ Modernes, allokationsfreies Parsing     │
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
#include <memory>    // Für std::unique_ptr
#include <charconv>  // Für std::from_chars (Modernes atoi)
#include <cstdint>   // Für std::int32_t

// ── Anonymer Namespace ──────────────────────────────────────
// Ersetzt das C-Style 'static' für globale Funktionen/Variablen.
// Alles hier drin ist strikt nur für diese Translation Unit sichtbar.
namespace {

    // ── Modern C++: using statt typedef ─────────────────────
    using ArgsSpan = std::span<char*>;

    // ── DATA ORIENTED DESIGN: Trennung von Daten und Logik ──
    // Ein reines Datenpaket (POD), das die Shell-Konfiguration hält.
    struct CliConfig {
        bool             runCli{false};
        std::string_view algoName{"quicksort"};
        // Zifferntrennzeichen für fehlerfreies Lesen
        std::int32_t     arraySize{100'000};
    };

    // ============================================================
    // Argument Parser
    // Nutzt std::span für sicheren Zugriff ohne Pointer-Arithmetik
    // ============================================================
    CliConfig parseArguments(ArgsSpan args)
    {
        CliConfig config;

        // args[0] ist der Pfad zur .exe, wir starten bei 1
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
                // Modern C++20 Parsing ohne Heap-Allokation (ersetzt std::atoi)
                std::string_view sizeStr = args[++i];
                std::from_chars(sizeStr.data(), sizeStr.data() + sizeStr.size(), config.arraySize);
            }
        }
        return config;
    }

    // ============================================================
    // Shell Benchmark Runner (Läuft ohne GUI)
    // ============================================================
    void runBenchmark(const CliConfig& config)
    {
        // Saubere deutsche Umlaute im Output
        std::cout << std::format("\n[ SortVisualizer - CLI Benchmark ]\n");
        std::cout << std::format("Algorithmus : {}\n", config.algoName);
        std::cout << std::format("Elemente    : {}\n", config.arraySize);
        std::cout << "Generiere Zufallsdaten...\n";

        // 1. Daten generieren (Modern C++20 Ranges)
        std::vector<std::int32_t> data(config.arraySize);
        std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<std::int32_t> dist(1, config.arraySize);
        std::ranges::generate(data, [&]{ return dist(rng); });

        // 2. Dummy Callback (Zero-Cost Abstraction)
        // Der Compiler wird diese leere Funktion inline komplett wegoptimieren.
        auto dummyCb = [](const std::vector<std::int32_t>&, std::int32_t, std::int32_t, std::string_view) {};

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

        // 4. Ergebnisse ausgeben (mit korrekten Umlauten)
        std::cout << std::format("=== ERGEBNISSE ===\n");
        std::cout << std::format("Benötigte Zeit : {} ms\n", duration.count());
        std::cout << std::format("Vergleiche     : {}\n", metrics.comparisons);
        std::cout << std::format("Array-Zugriffe : {}\n", metrics.arrayAccesses);
        std::cout << std::format("Tausch-Ops     : {}\n", metrics.swaps);
        std::cout << "\nBenchmark abgeschlossen.\n";
    }

} // Ende des anonymen Namespaces

// ============================================================
// MAIN
// ============================================================
int main(int argc, char* argv[])
{
    try
    {
        // argv in std::span verpacken für absolute Memory-Safety
        ArgsSpan args{argv, static_cast<std::size_t>(argc)};

        CliConfig config = parseArguments(args);

        if (config.runCli)
        {
            // ── CLI Modus ── (Keine GUI, reine Konsolen-Power)
            runBenchmark(config);
        }
        else
        {
            // ── GUI Modus ── (Normaler Start)
            // Modern C++: Nutzung von Smart Pointern (std::unique_ptr)
            // Verhindert Memory Leaks und schont den Stack bei großen Objekten.
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