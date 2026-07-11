#pragma once

#include <exception>

class UserRequestedExit final : public std::exception
{
public:
    [[nodiscard]] const char* what() const noexcept override
    {
        return "UserRequestedExit: Sortierung wurde vom Nutzer abgebrochen.";
    }
};