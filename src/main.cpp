// ============================================================
// main.cpp – Einstiegspunkt
//
// C++20 Features:
// ┌──────────────────┬─────────────────────────────────────────┐
// │ try/catch        │ Saubere Fehlerbehandlung                │
// │ std::exception   │ Basis für alle Standard-Exceptions      │
// └──────────────────┴─────────────────────────────────────────┘
// ============================================================
#include "Visualizer.hpp"
#include <SDL3/SDL.h>
// WICHTIG: Dieser Include löst den "WinMain" Linker-Fehler auf Windows!
#include <SDL3/SDL_main.h>
#include <stdexcept>

// WICHTIG: Die Signatur muss exakt so aussehen (argc und argv)!
int main(int argc, char* argv[])
{
    try
    {
        // ── RAII ──────────────────────────────────────────────
        // Visualizer-Objekt auf dem Stack: wird automatisch
        // zerstört wenn main() endet → Destruktor räumt auf.
        Visualizer vis;
        vis.run();
    }
    catch (const std::exception& e)
    {
        // Alle std::exception-Subtypen werden hier gefangen
        // (std::runtime_error, std::out_of_range etc.)
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Fehler",
            e.what(),  // Fehlermeldung aus der Exception
            nullptr);
        return 1;
    }

    return 0;
}