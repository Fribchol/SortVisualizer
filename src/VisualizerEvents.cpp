// ============================================================
// VisualizerEvents.cpp – Event-Handling & Interaktion
// ============================================================
#include "Visualizer.hpp"
#include <SDL3/SDL.h>
#include <algorithm>

static constexpr float MET_X = 1400.0f - 380.0f; // WIN_W - METRICS_W

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

            // ── Scrollen Linkes Fenster (Zahlen/Balken) ──
            if (logicalX < MET_X) {
                m_autoScrollNumbers = false;
                if (e.wheel.x != 0) m_numbersScrollX -= e.wheel.x * 40.0f;
                if (e.wheel.y != 0) {
                    if (shift) m_numbersScrollX -= e.wheel.y * 40.0f;
                    else       m_numbersScrollY -= static_cast<int32_t>(e.wheel.y) * 3;
                }
            }
            // ── Scrollen Rechtes Fenster (Erklärungspanel) ──
            else {
                m_autoScrollNumbers = false; // Scrollen deaktiviert Auto-Follow überall
                if (e.wheel.x != 0) m_explanationScrollX -= e.wheel.x * 40.0f;
                if (e.wheel.y != 0) {
                    if (shift) m_explanationScrollX -= e.wheel.y * 40.0f;
                    else       m_explanationScrollY -= static_cast<int32_t>(e.wheel.y) * 3;
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
    // Zurück zum Hauptmenü
    if (!m_sorting && isInside(mx, my, m_btnBackToMenu)) {
        m_appState = AppState::MainMenu;
        return;
    }

    if (!m_sorting) {
        for (size_t i = 0; i < m_algoButtons.size(); ++i) {
            if (isInside(mx, my, m_algoButtons[i])) {
                for (auto& b : m_algoButtons) b.active = false;
                m_algoButtons[i].active = true;
                m_algorithm = static_cast<Algorithm>(i);
                m_algoInfo = SortAlgorithms::getInfo(static_cast<uint8_t>(m_algorithm));
                fillRandom();
                return;
            }
        }
    }

    if (!m_sorting && isInside(mx, my, m_startButton)) {
        m_autoScrollNumbers = true;
        startLive();
    }
    else if (isInside(mx, my, m_stopButton)) {
        stopSort();
    }
    else if (isInside(mx, my, m_stepBackButton)) {
        m_autoScrollNumbers = true;
        stepBackward();
    }
    else if (isInside(mx, my, m_stepFwdButton)) {
        m_autoScrollNumbers = true;
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
}

void Visualizer::stopSort() {
    m_stopRequested = true;
    { std::lock_guard lock(m_mutex); m_stepRequested = true; }
    m_stepCV.notify_all();
}

void Visualizer::stepForward() {
    if (!m_liveMode && m_sorting && !m_threadFinished) {
        { std::lock_guard lock(m_mutex); m_stepRequested = true; }
        m_stepCV.notify_one();
    } else if (!m_liveMode && (!m_sorting || m_threadFinished)) {
        if (m_historyIndex < static_cast<int32_t>(m_history.size()) - 1) {
            applyHistoryStep(m_historyIndex + 1);
        } else if (!m_threadFinished && m_historyIndex == static_cast<int32_t>(m_history.size()) - 1) {
            startStepping();
        }
    }
}

void Visualizer::stepBackward() {
    if (!m_liveMode && m_historyIndex > 0) {
        applyHistoryStep(m_historyIndex - 1);
    }
}

void Visualizer::applyHistoryStep(int32_t index) {
    std::lock_guard lock(m_mutex);
    m_historyIndex = index;
    if (index >= 0 && index < static_cast<int32_t>(m_history.size())) {
        const auto& step = m_history[index];
        m_array = step.array;
        m_highlightA = step.indexA;
        m_highlightB = step.indexB;
        m_actionText = step.action;
    }
}