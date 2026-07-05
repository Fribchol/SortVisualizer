#include "Visualizer.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <thread>

namespace {
    constexpr float MET_X = 1400.0f - 380.0f;

    // Konstanten für UI-Labels
    constexpr std::string_view LABEL_FULLSCREEN_ON  = "Vollbild: AN";
    constexpr std::string_view LABEL_FULLSCREEN_OFF = "Vollbild: AUS";
}

bool Visualizer::isInside(float mx, float my, const Button& btn) {
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
                float lx, ly;
                SDL_RenderCoordinatesFromWindow(m_renderer.get(), e.button.x, e.button.y, &lx, &ly);
                if      (m_appState == AppState::MainMenu)   handleMainMenuClick(lx, ly);
                else if (m_appState == AppState::Settings)   handleSettingsClick(lx, ly);
                else if (m_appState == AppState::Visualizer) handleButtonClick(lx, ly);
            }
        }
        else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                m_isDraggingVolume = false;
                m_isDraggingSpeed  = false;
                m_isDraggingSize   = false;
            }
        }
        else if (e.type == SDL_EVENT_MOUSE_MOTION) {
            float lx, ly;
            SDL_RenderCoordinatesFromWindow(m_renderer.get(), e.motion.x, e.motion.y, &lx, &ly);

            if (m_isDraggingVolume && m_appState == AppState::Settings) {
                m_volume = std::clamp((lx - m_volumeSliderBg.x) / m_volumeSliderBg.w, 0.0f, 1.0f);
            }
            else if (m_isDraggingSpeed && m_appState == AppState::Visualizer) {
                m_delayMs = static_cast<std::uint32_t>(1000.0f * std::clamp((lx - m_speedSliderBg.x) / m_speedSliderBg.w, 0.0f, 1.0f));
            }
            else if (m_isDraggingSize && m_appState == AppState::Visualizer) {
                float ratio = std::clamp((lx - m_sizeSliderBg.x) / m_sizeSliderBg.w, 0.0f, 1.0f);
                m_arraySize = 5 + static_cast<std::int32_t>(ratio * 495.0f);
                fillSpecialCase(m_sortCase);
            }
        }
        else if (e.type == SDL_EVENT_MOUSE_WHEEL && m_appState == AppState::Visualizer) {
            float mx, my; SDL_GetMouseState(&mx, &my);
            float lx, ly; SDL_RenderCoordinatesFromWindow(m_renderer.get(), mx, my, &lx, &ly);

            // Scrollen im rechten Erklärungsfenster aktivieren, wenn Maus dort positioniert ist
            if (lx >= MET_X) {
                if (e.wheel.y != 0) {
                    m_explanationScrollY -= static_cast<std::int32_t>(e.wheel.y);
                }
            } else {
                // Linker Bereich (Zahlenansicht scrollen)
                m_autoScrollNumbers = false;
                if (e.wheel.y != 0) {
                    m_numbersScrollY -= static_cast<std::int32_t>(e.wheel.y) * 3;
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
    if (isInside(mx, my, m_btnSettingsBack)) m_appState = AppState::MainMenu;
    else if (isInside(mx, my, m_btnSettingsFullscreen)) {
        m_fullscreen = !m_fullscreen;
        SDL_SetWindowFullscreen(m_window.get(), m_fullscreen);
        m_btnSettingsFullscreen.label = std::string(m_fullscreen ? LABEL_FULLSCREEN_ON : LABEL_FULLSCREEN_OFF);
    }
    else if (mx >= m_volumeSliderBg.x - 10 && mx <= m_volumeSliderBg.x + m_volumeSliderBg.w + 10 &&
             my >= m_volumeSliderBg.y - 15 && my <= m_volumeSliderBg.y + m_volumeSliderBg.h + 15) {
        m_isDraggingVolume = true;
        m_volume = std::clamp((mx - m_volumeSliderBg.x) / m_volumeSliderBg.w, 0.0f, 1.0f);
    }
}

void Visualizer::handleButtonClick(float mx, float my) {
    if (!m_sorting && isInside(mx, my, m_btnBackToMenu)) { m_appState = AppState::MainMenu; return; }

    if (!m_sorting) {
        for (std::size_t i = 0; i < m_algoButtons.size(); ++i) {
            if (isInside(mx, my, m_algoButtons[i])) {
                for (auto& b : m_algoButtons) b.active = false;
                m_algoButtons[i].active = true;
                m_algorithm = static_cast<Algorithm>(i);
                m_algoInfo = SortAlgorithms::getInfo(static_cast<std::uint8_t>(m_algorithm));
                fillSpecialCase(m_sortCase); return;
            }
        }
    }

    if (mx >= m_speedSliderBg.x && mx <= m_speedSliderBg.x + m_speedSliderBg.w && my >= m_speedSliderBg.y && my <= m_speedSliderBg.y + m_speedSliderBg.h) {
        m_isDraggingSpeed = true;
        m_delayMs = static_cast<std::uint32_t>(1000.0f * std::clamp((mx - m_speedSliderBg.x) / m_speedSliderBg.w, 0.0f, 1.0f));
    }
    else if (!m_sorting && mx >= m_sizeSliderBg.x && mx <= m_sizeSliderBg.x + m_sizeSliderBg.w && my >= m_sizeSliderBg.y && my <= m_sizeSliderBg.y + m_sizeSliderBg.h) {
        m_isDraggingSize = true;
        float ratio = std::clamp((mx - m_sizeSliderBg.x) / m_sizeSliderBg.w, 0.0f, 1.0f);
        m_arraySize = 5 + static_cast<std::int32_t>(ratio * 495.0f);
        fillSpecialCase(m_sortCase);
    }
    else if (isInside(mx, my, m_startButton)) { if (!m_sorting) { m_autoScrollNumbers = true; startLive(); } }
    else if (isInside(mx, my, m_stopButton)) { if (m_sorting) { if (m_liveMode) pauseSort(); else { m_autoScrollNumbers = true; resumeSort(); } } }
    else if (isInside(mx, my, m_cancelButton)) { if (m_sorting || m_historyIndex > 0) cancelSort(); }
    else if (isInside(mx, my, m_stepBackButton)) { m_autoScrollNumbers = true; stepBackward(); }
    else if (isInside(mx, my, m_stepFwdButton)) {
        m_autoScrollNumbers = true;
        if (!m_sorting && m_historyIndex == 0 && m_history.size() <= 1) {
            startStepping();
            for (std::int32_t i = 0; i < 50; ++i) {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_history.size() > 1 || m_threadFinished) break;
            }
        }
        stepForward();
    }
    // Auswertung der neuen Vorbelegungs-Buttons für Szenarien
    else if (!m_sorting && isInside(mx, my, m_caseRandomBtn))  fillSpecialCase(SortCase::Random);
    else if (!m_sorting && isInside(mx, my, m_caseSortedBtn))  fillSpecialCase(SortCase::Sorted);
    else if (!m_sorting && isInside(mx, my, m_caseReverseBtn)) fillSpecialCase(SortCase::Reverse);
    else if (!m_sorting && isInside(mx, my, m_caseEqualBtn))   fillSpecialCase(SortCase::Equal);
    else if (!m_sorting && isInside(mx, my, m_randomButton)) fillRandom();
    else if (isInside(mx, my, m_viewBarsButton)) { m_viewBarsButton.active = true; m_viewNumsButton.active = false; m_viewMode = ViewMode::Bars; }
    else if (isInside(mx, my, m_viewNumsButton)) { m_viewBarsButton.active = false; m_viewNumsButton.active = true; m_viewMode = ViewMode::Numbers; m_autoScrollNumbers = true; }
    else if (isInside(mx, my, m_btnBenchmark)) std::system("start cmd");
}

void Visualizer::stepForward() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_historyIndex < static_cast<std::int32_t>(m_history.size()) - 1) {
        m_historyIndex++;
        const auto& step = m_history[static_cast<std::size_t>(m_historyIndex)];
        m_array = step.array; m_highlightA = step.indexA; m_highlightB = step.indexB;
    } else if (m_threadFinished && m_sorting) {
        m_sorting = false; m_highlightA = -1; m_highlightB = -1;
    }
}

void Visualizer::stepBackward() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_historyIndex > 0) {
        m_historyIndex--;
        const auto& step = m_history[static_cast<std::size_t>(m_historyIndex)];
        m_array = step.array; m_highlightA = step.indexA; m_highlightB = step.indexB;
    }
}