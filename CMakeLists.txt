cmake_minimum_required(VERSION 3.20)
project(SortVisualizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)

# ========================================================
# Target 1: Der grafische SortVisualizer (mit GUI, SDL, etc.)
# ========================================================
add_executable(SortVisualizer
        # -- Core & Main --
        src/main.cpp
        src/SortAlgorithms.cpp

        # -- Visualizer & UI --
        src/Visualizer.cpp
        src/VisualizerDraw.cpp
        src/VisualizerEvents.cpp

        # -- Algorithmen --
        src/algorithms/QuickSort.cpp
        src/algorithms/MergeSortRec.cpp
        src/algorithms/MergeSortIt.cpp
        src/algorithms/HeapSort.cpp
        src/algorithms/RadixSort.cpp
        src/algorithms/CountingSort.cpp
        src/algorithms/BubbleSort.cpp
        src/algorithms/BubbleSort.hpp
)

target_link_libraries(SortVisualizer PRIVATE
        SDL3::SDL3
        SDL3_ttf::SDL3_ttf
)

target_include_directories(SortVisualizer PRIVATE src)

# Kopiert die Schriftart automatisch in den Build-Ordner
configure_file(
        ${CMAKE_SOURCE_DIR}/DejaVuSans.ttf
        ${CMAKE_BINARY_DIR}/DejaVuSans.ttf
        COPYONLY
)

# ========================================================
# Target 2: High-Performance Konsolen-Benchmark (Terminal)
# ========================================================
add_executable(SortBenchmark
        src/TerminalMain.cpp
        src/SortAlgorithms.cpp
        src/algorithms/QuickSort.cpp
        src/algorithms/MergeSortRec.cpp
        src/algorithms/MergeSortIt.cpp
        src/algorithms/HeapSort.cpp
        src/algorithms/RadixSort.cpp
        src/algorithms/CountingSort.cpp
        src/algorithms/BubbleSort.cpp
)

target_include_directories(SortBenchmark PRIVATE src)

# Setze Compiler-Optimierungen für maximale Performance im Terminal (ohne GUI-Overhead)
# Diese greifen nur im Release-Modus, um Konflikte bei Runtime-Checks im Debug-Modus zu vermeiden
if (MSVC)
    # Visual Studio Compiler: Maximale Geschwindigkeit /O2 (nur bei Release)
    target_compile_options(SortBenchmark PRIVATE $<$<CONFIG:Release>:/O2;/Ob2;/fp:fast>)
else()
    # GCC / Clang Compiler: Höchste Optimierungsstufe -O3 (nur bei Release)
    target_compile_options(SortBenchmark PRIVATE $<$<CONFIG:Release>:-O3;-march=native>)
endif()