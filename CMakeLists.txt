cmake_minimum_required(VERSION 3.20)
project(SortVisualizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)

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

# Sorgt dafür, dass unter Windows im Release-Modus keine störende Konsole im Hintergrund aufgeht
if(WIN32)
    set_target_properties(SortVisualizer PROPERTIES
            WIN32_EXECUTABLE $<CONFIG:Release>
    )
endif()