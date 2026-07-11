cmake_minimum_required(VERSION 3.20)
project(SortVisualizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Verhindert "fatal error C1041: Programmdatenbank kann nicht geöffnet werden"
if (MSVC)
    add_compile_options(/FS)
endif()

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)

# Hier sind alle Algos in einem Objekt gebündelt und Änderungen müssen nur hier gepflegt werden
add_library(AlgoLib STATIC
        src/Algorithms/QuickSort.cpp
        src/Algorithms/MergeSortRec.cpp
        src/Algorithms/MergeSortIt.cpp
        src/Algorithms/HeapSort.cpp
        src/Algorithms/RadixSort.cpp
        src/Algorithms/CountingSort.cpp
        src/Algorithms/BubbleSort.cpp
        src/Algorithms/SortInfo.cpp  # HIER MUSS getInfo() drin sein!
)

target_include_directories(AlgoLib PUBLIC src)

add_executable(SortVisualizer
        src/main.cpp
        src/Visualizer.cpp
        src/VisualizerDraw.cpp
        src/VisualizerEvents.cpp
)

target_link_libraries(SortVisualizer PRIVATE AlgoLib SDL3::SDL3 SDL3_ttf::SDL3_ttf)

configure_file(${CMAKE_SOURCE_DIR}/DejaVuSans.ttf ${CMAKE_BINARY_DIR}/DejaVuSans.ttf COPYONLY)

add_executable(SortBenchmark src/TerminalMain.cpp
        src/Algorithms/SortInfo.cpp)

target_link_libraries(SortBenchmark PRIVATE AlgoLib)

if (MSVC)
    target_compile_options(SortBenchmark PRIVATE $<$<CONFIG:Release>:/O2;/Ob2;/fp:fast>)
else()
    target_compile_options(SortBenchmark PRIVATE $<$<CONFIG:Release>:-O3;-march=native>)
endif()