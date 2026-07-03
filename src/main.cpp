// ============================================================
// main.cpp – Einstiegspunkt (GUI & CLI Dual-Mode)
// ============================================================
// C++20/C++23 Features & Modernisierungen:
// ┌──────────────────────┬────────────────────────────────────────────────────────┐
// │ Anonymer Bereich     │ namespace { } – Kapselung, vermeidet globale Verlinkung│
// │ std::span            │ Sichere Referenzierung von C-Arrays (argv) ohne Kopien │
// │ std::unique_ptr      │ Memory-Safety (RAII) für das Visualizer-Objekt         │
// │ std::from_chars      │ Performantes, speichereffizientes String-Parsing       │
// │ Strukturierte Bindung│ C++17/20-Syntax zum direkten Entpacken von Tupeln/Structs│
// └──────────────────────┴────────────────────────────────────────────────────────┘

#include "Visualizer.hpp"
#include "SortAlgorithms.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <vector>
#include <string_view>
#include <random>
#include <chrono>
#include <format>
#include <algorithm>
#include <charconv>
#include <cstdint>
#include <span>

// Anonymer Namespace, um Symbole/Funktionen vor externen Modulen zu verbergen (Internal Linkage)
namespace {

    // Typ-Alias für vereinfachte Handhabung der C-Style Argumente via C++20 std::span
    using ArgsSpan = std::span<char*>;

    // Struktur zur Kapselung der Konsolenparameter (CommandLine Interface Parameter)
    struct CliConfig {
        bool             runCli{false};
        std::string_view algoName{"quicksort"};
        std::int32_t     arraySize{100'000};
    };

    // [[nodiscard]] erzwingt, dass der Rückgabewert ausgewertet werden muss
    [[nodiscard]] CliConfig parseArguments(const ArgsSpan args) noexcept
    {
        CliConfig config;
        // Iteriere über die Argumente beginnend ab Index 1 (Index 0 ist der Programmname selbst)
        for (std::size_t i = 1; i < args.size(); ++i)
        {
            const std::string_view arg = args[i];

            if (arg == "--cli") {
                config.runCli = true;
            }
            // Prüfe, ob --algo gefolgt von einem weiteren Parameter übergeben wurde
            else if (arg == "--algo" && (i + 1) < args.size()) {
                config.algoName = args[++i];
            }
            // Prüfe, ob --size gefolgt von einem numerischen String übergeben wurde
            else if (arg == "--size" && (i + 1) < args.size()) {
                const std::string_view sizeStr = args[++i];
                // Konvertiert den String superschnell und speicherallokationsfrei in eine Zahl
                std::from_chars(sizeStr.data(), sizeStr.data() + sizeStr.size(), config.arraySize);
            }
        }
        return config;
    }

    // High-Performance Konsolen-Benchmark ohne GUI / Zeichnen-Overhead
    void runBenchmark(const CliConfig& config)
    {
        std::cout << std::format("\n[ SortVisualizer - CLI Benchmark ]\n");
        std::cout << std::format("Algorithmus : {}\n", config.algoName);
        std::cout << "Elemente    : " << config.arraySize << "\n";
        std::cout << "Generiere Zufallsdaten...\n";

        // Vektor fester Größe im Heap anlegen (Datenorientiertes Design)
        std::vector<std::int32_t> data(static_cast<std::size_t>(config.arraySize));

        // Zufallsgenerator initialisieren (Echt-Zufall via OS Hardware).
        // Das const wurde entfernt, um Kompatibilitätsprobleme mit MSVC zu beheben.
        // Clang-Tidy constexpr-Warnung wird hier bewusst unterdrückt.
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
        std::random_device rd;
        std::mt19937 rng{rd()};
        std::uniform_int_distribution<std::int32_t> dist(1, config.arraySize);

        // Daten mit Pseudozufallszahlen befüllen
        std::ranges::generate(data, [&]{ return dist(rng); });

        // Dummy-Callback für Sortieralgorithmen, die einen Visualisierungs-Schritt erwarten (hier leer)
        auto dummyCb = [](const std::vector<std::int32_t>&, std::int32_t, std::int32_t) noexcept {};

        LiveMetrics metrics{};
        std::cout << "Starte Sortierung...\n\n";

        // Präzise Zeitmessung der reinen Sortieroperation starten
        const auto start = std::chrono::high_resolution_clock::now();

        // Dynamisches Dispatching zum passenden Algorithmus (Schnittstellenabdeckung aller 7 Algos)
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

        // Messung anhalten und Differenz (Laufzeit) ermitteln
        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // Konsolenausgabe der Messwerte (Laufzeit, Vergleiche, Zugriffe)
        std::cout << "=== ERGEBNISSE ===\n";
        std::cout << std::format("Benötigte Zeit : {} ms\n", duration.count());
        std::cout << std::format("Vergleiche     : {}\n", metrics.comparisons);
        std::cout << std::format("Array-Zugriffe : {}\n", metrics.arrayAccesses);
        std::cout << std::format("Tausch-Ops     : {}\n", metrics.swaps);
        std::cout << "\nBenchmark abgeschlossen.\n";
    }

} // namespace

// C++ konformer Haupt-Einstiegspunkt. Nimmt Parameter aus dem OS entgegen.
int main(const int argc, char* argv[])
{
    // Exception-Handling, um Laufzeitfehler im System abzufangen
    try
    {
        // Parameterübergabe an Typsicheren std::span (Schnittstellenmodernisierung)
        const ArgsSpan args{argv, static_cast<std::size_t>(argc)};

        // Direktinitialisierung der Konfiguration in der Bedingung (C++17/20 Syntax)
        if (const CliConfig config = parseArguments(args); config.runCli) {
            runBenchmark(config);
        } else {
            // Memory Safety (RAII): Heap-Allokation wird in Smart Pointer gekapselt
            const auto vis = std::make_unique<Visualizer>();
            vis->run();
        }
    }
    catch (const std::exception& e)
    {
        // Abfangen unbehandelter Exception-Fehler via OS Message Box (SDL)
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Kritischer Fehler", e.what(), nullptr);
        return 1;
    }
    return 0;
}