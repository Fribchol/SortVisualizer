#include "Visualizer.hpp"
#include "SortAlgorithms.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <vector>
#include <string_view>
#include <random>
#include <format>
#include <charconv>
#include <cstdint>
#include <span>
#include <memory>
#include <algorithm>
#include <chrono>

namespace {

    using ArgsSpan = std::span<char*>;

    struct CliConfig {
        bool             runCli{false};
        std::string_view algoName{"quicksort"};
        std::int32_t     arraySize{100'000};
    };

    struct UserRequestedStop {};

    [[nodiscard]] CliConfig parseArguments(const ArgsSpan args) noexcept
    {
        CliConfig config;
        for (std::size_t i = 1; i < args.size(); ++i)
        {
            const std::string_view arg = args[i];
            if (arg == "--cli") {
                config.runCli = true;
            }
            else if (arg == "--algo" && (i + 1) < args.size()) {
                config.algoName = args[++i];
            }
            else if (arg == "--size" && (i + 1) < args.size()) {
                const std::string_view sizeStr = args[++i];
                std::int32_t parsed{};
                const auto result = std::from_chars(
                    sizeStr.data(), sizeStr.data() + sizeStr.size(), parsed);

                if (result.ec == std::errc{} &&
                    result.ptr == sizeStr.data() + sizeStr.size() &&
                    parsed > 0)
                {
                    config.arraySize = parsed;
                }
                else
                {
                    std::cerr << std::format(
                        "Warnung: Ungueltige --size '{}', verwende Default ({}).\n",
                        sizeStr, config.arraySize);
                }
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
        std::random_device rd;
        std::mt19937 rng{rd()};
        std::uniform_int_distribution<std::int32_t> dist(1, config.arraySize);

        std::ranges::generate(data, [&]{ return dist(rng); });

        auto dummyCb = [](const std::vector<std::int32_t>&, std::int32_t, std::int32_t, SortAlgorithms::StepKind) noexcept {};

        SortAlgorithms::LiveMetrics metrics{};
        std::cout << "Starte Sortierung...\n\n";

        const auto start = std::chrono::steady_clock::now();

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

        const auto elapsed = std::chrono::steady_clock::now() - start;
        const auto elapsedMs = std::chrono::duration<double, std::milli>(elapsed).count();

        std::cout << std::format("Fertig in {:.3f} ms\n", elapsedMs);
        std::cout << std::format("Vergleiche  : {}\n", metrics.comparisons);
        std::cout << std::format("Swaps       : {}\n", metrics.swaps);
        std::cout << std::format("Array-Zugr. : {}\n", metrics.arrayAccesses);

        if (!std::ranges::is_sorted(data)) {
            std::cerr << "Warnung: Ergebnis ist NICHT sortiert!\n";
        }
    }

}

int main(const int argc, char* argv[])
{
    try
    {
        const ArgsSpan args{argv, static_cast<std::size_t>(argc)};

        if (const CliConfig config = parseArguments(args); config.runCli) {
            runBenchmark(config);
        } else {
            const auto vis = std::make_unique<Visualizer>();
            vis->run();
        }
    }
    catch (const UserRequestedStop&)
    {
        // Abbruch ist kein Fehler, beendet Programm sauber.
        return 0;
    }
    catch (const std::exception& e)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Kritischer Fehler", e.what(), nullptr);
        return 1;
    }
    return 0;
}