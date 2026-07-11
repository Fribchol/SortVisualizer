#pragma once

#include <exception>

// ==============================================================
// UserRequestedExit
// ==============================================================
// Wird im Sortier-Thread (siehe Visualizer::sortThreadFunc) geworfen,
// sobald der jthread's stop_token einen Abbruch signalisiert (z.B.
// weil der Nutzer während des Sortierens auf "Abbruch" geklickt oder
// das Fenster geschlossen hat). Der StepCallback prüft nach jedem
// Schritt stopToken.stop_requested() und wirft in diesem Fall diese
// Exception, um die Sortierfunktion sofort zu verlassen - unabhängig
// davon, wie tief sie gerade rekursiv verschachtelt ist (z.B. mitten
// in QuickSort). Der try/catch-Block in sortThreadFunc fängt sie ab
// und behandelt sie als normalen, sauberen Abbruch (kein echter Fehler).
class UserRequestedExit final : public std::exception
{
public:
    [[nodiscard]] const char* what() const noexcept override
    {
        return "UserRequestedExit: Sortierung wurde vom Nutzer abgebrochen.";
    }
};