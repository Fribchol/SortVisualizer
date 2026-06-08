// ============================================================
// SortAlgorithms.hpp
// Zentrale Typen, Structs und Dispatcher-Deklarationen
//
// C++20 Features:
// ┌─────────────────┬──────────────────────────────────────────┐
// │ [[nodiscard]]   │ Warnung wenn Rückgabe ignoriert wird     │
// │ string_view     │ String ohne Kopie übergeben              │
// │ using Alias     │ Moderner Ersatz für typedef              │
// │ Member-Init     │ Default-Werte direkt im Struct           │
// └─────────────────┴──────────────────────────────────────────┘
// ============================================================
#pragma once

#include <cstdint>      // int32_t, uint64_t → exakte Typen statt int
#include <functional>   // std::function → typsichere Callbacks
#include <string>       // std::string
#include <string_view>  // std::string_view → kein Kopieren
#include <vector>       // std::vector

// ============================================================
// SortStep – Snapshot eines Sortierschritts
//
// ── Aggregate Initialization (C++20) ─────────────────────────
// Kein Konstruktor nötig. Direkte {}-Initialisierung:
//   SortStep s{arr, 2, 5, "Tausch"};
// ── Member Default-Werte ──────────────────────────────────────
// {-1} bedeutet: kein Index markiert
// ============================================================
struct SortStep
{
    std::vector<int32_t> array;       // Array-Zustand in diesem Schritt
    int32_t              indexA{-1};  // erster markierter Index
    int32_t              indexB{-1};  // zweiter markierter Index
    std::string          action;      // z.B. "Tausch [2]=17 <-> [5]=43"
};

// ============================================================
// AlgoInfo – Theoretische Eigenschaften eines Algorithmus
// Wird im Metriken-Panel angezeigt.
// ============================================================
struct AlgoInfo
{
    std::string name;           // z.B. "QuickSort"
    std::string bestCase;       // z.B. "O(n log n)"
    std::string avgCase;
    std::string worstCase;
    std::string space;          // Speicherplatzkomplexität
    std::string stable;         // "Ja" oder "Nein"
    std::string quality;        // Güte in der Praxis
    std::string communication;  // Cache-Verhalten
};

// ============================================================
// LiveMetrics – Live gemessene Werte während der Sortierung
//
// ── uint64_t statt int ────────────────────────────────────────
// uint64_t: 64-bit unsigned → kein Überlauf bei großen Arrays
// Aus <cstdint>: exakte Größe garantiert auf jeder Plattform
// ============================================================
struct LiveMetrics
{
    uint64_t comparisons   {0};   // Anzahl Vergleiche
    uint64_t swaps         {0};   // Anzahl Tauschoperationen
    uint64_t arrayAccesses {0};   // Anzahl Array-Zugriffe
    double   elapsedMs     {0.0}; // gemessene Zeit in ms
};

// ============================================================
// StepCallback – Callback-Typ für Sortierschritte
//
// ── using statt typedef (C++11/20) ───────────────────────────
// Alt:    typedef std::function<void(...)> StepCallback;
// Modern: using StepCallback = std::function<void(...)>;
// Vorteil: lesbarer, funktioniert mit Templates
//
// ── std::string_view statt const std::string& ─────────────────
// string_view = "Blick" auf einen String ohne Kopie.
// Kein Heap-Alloc, kein Overhead. Perfekt für read-only Strings.
// ============================================================
using StepCallback = std::function<void(
    const std::vector<int32_t>& arr,  // aktueller Array-Zustand
    int32_t                     a,    // erster markierter Index
    int32_t                     b,    // zweiter markierter Index
    std::string_view            action // Beschreibung des Schritts
)>;

// ============================================================
// SortAlgorithms Namespace
//
// ── [[nodiscard]] (C++17/20) ─────────────────────────────────
// Compiler warnt wenn Rückgabewert ignoriert wird:
//   getInfo(i);              // ← Compiler-Warnung!
//   AlgoInfo info = getInfo(i); // ← korrekt
// ============================================================
namespace SortAlgorithms
{
    [[nodiscard]] AlgoInfo getInfo(uint8_t algorithmIndex);

    void quickSort   (std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m);
    void mergeSort   (std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m);
    void mergeSortIt (std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m);
    void heapSort    (std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m);
    void radixSort   (std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m);
    void countingSort(std::vector<int32_t>& arr, StepCallback cb, LiveMetrics& m);
}