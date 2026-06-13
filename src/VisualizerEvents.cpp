// ============================================================
// VisualizerEvents.cpp – Event-Handling & Interaktion
//
// C++20 Features & Modernisierungen:
// ┌──────────────────────┬─────────────────────────────────────┐
// │ Anonymer Namespace   │ Ersetzt C-static für Datei-Konst.   │
// │ std::int32_t         │ Explizite Typen aus <cstdint>       │
// │ std::size_t          │ Für Schleifen über Container        │
// │ std::uint8_t         │ Für enum-Casts (Algorithm)          │
// └──────────────────────┴─────────────────────────────────────┘
// ============================================================
#include "Visualizer.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cstdlib>
#include <cstdint> // Für explizite Integer-Typen
#include <thread>  // Für std::this_thread::sleep_for

// ── Anonymer Namespace ──────────────────────────────────────
namespace {
    constexpr float MET_X = 1400.0f - 380.0f; // WIN_W - METRICS_W
}

bool Visualizer::isInside(float mx, float my, const Button& btn) const {
    return mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h;
}

void Visualizer::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            m_running = false;
        }
        else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                float logicalX, logicalY;
                SDL_RenderCoordinatesFromWindow(m_renderer.get(), e.button.x, e.button.y, &logicalX, &logicalY);

                if (m_appState == AppState::MainMenu)        handleMainMenuClick(logicalX, logicalY);
                else if (m_appState == AppState::Settings)   handleSettingsClick(logicalX, logicalY);
                else if (m_appState == AppState::Visualizer) handleButtonClick(logicalX, logicalY);
            }
        }
        else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                m_isDraggingVolume = false;
            }
        }
        else if (e.type == SDL_EVENT_MOUSE_MOTION) {
            if (m_isDraggingVolume && m_appState == AppState::Settings) {
                float logicalX, logicalY;
                SDL_RenderCoordinatesFromWindow(m_renderer.get(), e.motion.x, e.motion.y, &logicalX, &logicalY);
                m_volume = std::clamp((logicalX - m_volumeSliderBg.x) / m_volumeSliderBg.w, 0.0f, 1.0f);
            }
        }
        else if (e.type == SDL_EVENT_MOUSE_WHEEL && m_appState == AppState::Visualizer) {
            float mx, my;
            SDL_GetMouseState(&mx, &my);
            float logicalX, logicalY;
            SDL_RenderCoordinatesFromWindow(m_renderer.get(), mx, my, &logicalX, &logicalY);

            const bool shift = (SDL_GetModState() & SDL_KMOD_SHIFT) != 0;

            if (logicalX < MET_X) {
                m_autoScrollNumbers = false;
                if (e.wheel.x != 0) m_numbersScrollX -= e.wheel.x * 40.0f;
                if (e.wheel.y != 0) {
                    if (shift) m_numbersScrollX -= e.wheel.y * 40.0f;
                    else       m_numbersScrollY -= static_cast<std::int32_t>(e.wheel.y) * 3;
                }
            }
            else {
                m_autoScrollNumbers = false;
                if (e.wheel.x != 0) m_explanationScrollX -= e.wheel.x * 40.0f;
                if (e.wheel.y != 0) {
                    if (shift) m_explanationScrollX -= e.wheel.y * 40.0f;
                    else       m_explanationScrollY -= static_cast<std::int32_t>(e.wheel.y) * 3;
                }
            }
        }
    }
}

void Visualizer::handleMainMenuClick(float mx, float my) {
    if (isInside(mx, my, m_btnMenuStart)) m_appState = AppState::Visualizer;
    else if (isInside(mx, my, m_btnMenuSettings)) m_appState = AppState::Settings;
    else if (isInside(mx, my, m_btnMenuQuit)) m_running = false;
}

void Visualizer::handleSettingsClick(float mx, float my) {
    if (isInside(mx, my, m_btnSettingsBack)) {
        m_appState = AppState::MainMenu;
    }
    else if (isInside(mx, my, m_btnSettingsFullscreen)) {
        m_fullscreen = !m_fullscreen;
        SDL_SetWindowFullscreen(m_window.get(), m_fullscreen);
        m_btnSettingsFullscreen.label = m_fullscreen ? "Vollbild: AN" : "Vollbild: AUS";
    }
    else if (mx >= m_volumeSliderBg.x - 10 && mx <= m_volumeSliderBg.x + m_volumeSliderBg.w + 10 &&
             my >= m_volumeSliderBg.y - 15 && my <= m_volumeSliderBg.y + m_volumeSliderBg.h + 15) {
        m_isDraggingVolume = true;
        m_volume = std::clamp((mx - m_volumeSliderBg.x) / m_volumeSliderBg.w, 0.0f, 1.0f);
    }
}

void Visualizer::handleButtonClick(float mx, float my) {
    if (!m_sorting && isInside(mx, my, m_btnBackToMenu)) {
        m_appState = AppState::MainMenu;
        return;
    }

    if (!m_sorting) {
        for (std::size_t i = 0; i < m_algoButtons.size(); ++i) {
            if (isInside(mx, my, m_algoButtons[i])) {
                for (auto& b : m_algoButtons) b.active = false;
                m_algoButtons[i].active = true;
                m_algorithm = static_cast<Algorithm>(i);
                m_algoInfo = SortAlgorithms::getInfo(static_cast<std::uint8_t>(m_algorithm));
                fillRandom();
                return;
            }
        }
    }

    if (isInside(mx, my, m_startButton)) {
        if (!m_sorting) {
            m_autoScrollNumbers = true;
            startLive();
        }
    }
    else if (isInside(mx, my, m_stopButton)) {
        if (m_sorting) {
            if (m_liveMode) pauseSort();
            else { m_autoScrollNumbers = true; resumeSort(); }
        }
    }
    else if (isInside(mx, my, m_cancelButton)) {
        if (m_sorting || m_historyIndex > 0) {
            cancelSort();
        }
    }
    else if (isInside(mx, my, m_speedDownButton)) {
        if (m_delayMs >= 100)      m_delayMs += 50;
        else if (m_delayMs >= 20)  m_delayMs += 10;
        else if (m_delayMs >= 5)   m_delayMs += 5;
        else                       m_delayMs += 1;
        if (m_delayMs > 1000)      m_delayMs = 1000;
    }
    else if (isInside(mx, my, m_speedUpButton)) {
        if (m_delayMs > 100)       m_delayMs -= 50;
        else if (m_delayMs > 20)   m_delayMs -= 10;
        else if (m_delayMs > 5)    m_delayMs -= 5;
        else if (m_delayMs > 1)    m_delayMs -= 1;
    }
    else if (isInside(mx, my, m_stepBackButton)) {
        m_autoScrollNumbers = true;
        stepBackward();
    }
    else if (isInside(mx, my, m_stepFwdButton)) {
        m_autoScrollNumbers = true;

        // Startet Thread heimlich, falls noch nichts läuft
        if (!m_sorting && m_historyIndex == 0 && m_history.size() <= 1) {
            startStepping();
            for (std::int32_t i = 0; i < 50; ++i) {
                {
                    std::lock_guard lock(m_mutex);
                    if (m_history.size() > 1 || m_threadFinished) break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        stepForward();
    }
    else if (!m_sorting && isInside(mx, my, m_randomButton)) {
        fillRandom();
    }
    else if (!m_sorting && isInside(mx, my, m_sizeDownButton)) {
        if (m_arraySize > 5) { m_arraySize -= 5; fillRandom(); }
    }
    else if (!m_sorting && isInside(mx, my, m_sizeUpButton)) {
        if (m_arraySize < 500) { m_arraySize += 5; fillRandom(); }
    }
    else if (isInside(mx, my, m_viewBarsButton)) {
        m_viewBarsButton.active = true;
        m_viewNumsButton.active = false;
        m_viewMode = ViewMode::Bars;
    }
    else if (isInside(mx, my, m_viewNumsButton)) {
        m_viewBarsButton.active = false;
        m_viewNumsButton.active = true;
        m_viewMode = ViewMode::Numbers;
        m_autoScrollNumbers = true;
    }
    // ── ANPASSUNG: Benchmark-Button öffnet nur noch die Shell ──
    else if (isInside(mx, my, m_btnBenchmark)) {
        // Öffnet ein leeres Windows-Terminal im aktuellen Verzeichnis
        std::system("start cmd");
    }
}

void Visualizer::stepForward() {
    std::lock_guard lock(m_mutex);
    if (m_historyIndex < static_cast<std::int32_t>(m_history.size()) - 1) {
        m_historyIndex++;
        const auto& step = m_history[m_historyIndex];
        m_array = step.array;
        m_highlightA = step.indexA;
        m_highlightB = step.indexB;
        m_actionText = step.action;
    } else if (m_threadFinished && m_sorting) {
        m_sorting = false;
        m_actionText = "Sortierung erfolgreich!";
        m_highlightA = -1;
        m_highlightB = -1;
    }
}

void Visualizer::stepBackward() {
    std::lock_guard lock(m_mutex);
    if (m_historyIndex > 0) {
        m_historyIndex--;
        const auto& step = m_history[m_historyIndex];
        m_array = step.array;
        m_highlightA = step.indexA;
        m_highlightB = step.indexB;
        m_actionText = step.action;
    }
}