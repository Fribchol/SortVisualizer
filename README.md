# 🔢 Algorithmus Visualizer

Ein interaktiver **Sortieralgorithmus-Visualizer** in modernem **C++20** mit **SDL3**.

![C++](https://img.shields.io/badge/C%2B%2B-20-blue?style=flat-square&logo=cplusplus)
![SDL3](https://img.shields.io/badge/SDL-3-green?style=flat-square)
![License](https://img.shields.io/badge/License-MIT-yellow?style=flat-square)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey?style=flat-square&logo=windows)

---

## ✨ Features

- **Live-Animation** – Sieh zu wie der Algorithmus das Array sortiert
- **Schritt-für-Schritt Modus** – Jeden einzelnen Schritt selbst steuern mit `>` und `<`
- **Vor & Zurück Navigation** – Frei durch die komplette History navigieren
- **Stop** – Animation jederzeit anhalten (auch per `Space`)
- **Balken-Ansicht** – Klassische Visualisierung mit farbigen Balken
- **Zahlen-Ansicht** – Zeigt die tatsächlichen Zahlen und Tausch-Pfeile
- **Schritt-Erklärungen** – Rechtes Panel erklärt jeden Schritt auf Deutsch
- **Scrollbares Panel** – Mausrad zum Scrollen durch alle vergangenen Schritte
- **Zufalls-Array** – Neues Array mit einem Klick generieren
   **Array-Größe** – Von 5 bis x Elemente einstellbar

---

## Implementierte Algorithmen

| Algorithmus | Best Case | Avg Case | Worst Case | Speicher |
|---|---|---|---|---|
| **QuickSort** | O(n log n) | O(n log n) | O(n²) | O(log n) |
| **MergeSort (rekursiv)** | O(n log n) | O(n log n) | O(n log n) | O(n) |
| **MergeSort (iterativ)** | O(n log n) | O(n log n) | O(n log n) | O(n) |
| **HeapSort** | O(n log n) | O(n log n) | O(n log n) | O(1) |
| **RadixSort** | O(nk) | O(nk) | O(nk) | O(n+k) |
| **CountingSort** | O(n+k) | O(n+k) | O(n+k) | O(k) |

---

## Bedienung

| Aktion | Beschreibung |
|---|---|
| **Algorithmus-Button** | Algorithmus auswählen |
| **Start** | Live-Animation starten |
| **Stop** / `Space` | Animation stoppen |
| **`>`** / `Pfeil-Rechts` | Einen Schritt vorwärts |
| **`<`** / `Pfeil-Links` | Einen Schritt zurück |
| **Random** | Neues Zufalls-Array generieren |
| **`+` / `-`** | Array-Größe erhöhen / verringern |
| **Balken / Zahlen** | Ansicht wechseln |
| **Mausrad** | Schritt-Erklärungen scrollen |

---

## Voraussetzungen

- **Windows** (x64)
- **CMake** >= 3.20
- **Visual Studio 2022** (mit C++ Workload) oder CLion
- **vcpkg** (Paketmanager für SDL3)

---

## Installation

### 1. Repository klonen

```bash
git clone https://git.ide3.de/pafri005/algorithmus-visualizer.git
cd algorithmus-visualizer
```

### 2. vcpkg installieren (falls noch nicht vorhanden)

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
```

### 3. Abhängigkeiten installieren

```powershell
C:\vcpkg\vcpkg.exe install sdl3:x64-windows
C:\vcpkg\vcpkg.exe install sdl3-ttf:x64-windows
```

### 4. CMake konfigurieren

In CLion unter **Settings → Build → CMake → CMake options**:
```
-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Oder per Kommandozeile:
```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

### 5. Font hinzufügen

Lade **DejaVuSans.ttf** herunter und lege die Datei neben `CMakeLists.txt`:
- Download: [dejavu-fonts.github.io](https://dejavu-fonts.github.io/Download.html)

---

## Derzeitige Projektstruktur

```
algorithmus-visualizer/
├── CMakeLists.txt
├── DejaVuSans.ttf             
├── README.md
└── src/
    ├── AddExceptions.hpp
    ├── Main.cpp
    ├── SortAlgorithms.hpp
    ├── TerminalMain.cpp 
    ├── Visualizer.cpp           
    ├── Visualizer.hpp          
    ├── VisualizerDraw.cpp       
    ├── VisualizerEvents.cpp  
    └── Algorithms/
        ├── BubbleSort.cpp
        ├── CountingSort.cpp
        ├── HeapSort.cpp
        ├── MergeSortIt.cpp
        ├── MergeSortRec.cpp
        ├── QuickSort.cpp
        ├── RadixSort.cpp
        └── SortInfo.cpp
```

---


## Lizenz

```
MIT License

Copyright (c) 2025

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---
