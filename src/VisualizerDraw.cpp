// ============================================================
// VisualizerDraw.cpp – Rendern der Visualisierung
// ============================================================
#include "Visualizer.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <format>
#include <ranges>
#include <sstream>
#include <cstdint>

namespace {
    constexpr std::int32_t WIN_W     = 1400;
    constexpr std::int32_t WIN_H     = 860;
    constexpr float        UI_H      = 170.0f;
    constexpr float        VIS_X     = 10.0f;
    constexpr float        VIS_Y     = 10.0f;
    constexpr float        VIS_W     = static_cast<float>(WIN_W) - 20.0f; // Volle Breite über den Bildschirm
    constexpr float        VIS_H     = static_cast<float>(WIN_H) - UI_H - 20.0f;

    struct SdlSurfaceDeleter { void operator()(SDL_Surface* s) const noexcept { SDL_DestroySurface(s); } };
    struct SdlTextureDeleter { void operator()(SDL_Texture* t) const noexcept { SDL_DestroyTexture(t); } };

    using SurfacePtr = std::unique_ptr<SDL_Surface, SdlSurfaceDeleter>;
    using TexturePtr = std::unique_ptr<SDL_Texture, SdlTextureDeleter>;
}

void Visualizer::drawText(std::string_view text, float x, float y, SDL_Color color, TTF_Font* font)
{
    if (text.empty() || !font) return;

    SurfacePtr surf{ TTF_RenderText_Blended(font, std::string(text).c_str(), 0, color) };
    if (!surf) return;

    TexturePtr tex{ SDL_CreateTextureFromSurface(m_renderer.get(), surf.get()) };
    surf.reset();
    if (!tex) return;

    float tw{}, th{};
    SDL_GetTextureSize(tex.get(), &tw, &th);
    SDL_FRect destRect{x, y, tw, th};
    SDL_RenderTexture(m_renderer.get(), tex.get(), nullptr, &destRect);
}

void Visualizer::drawTextWrapped(std::string_view text, float x, float y, float maxWidth, SDL_Color color, TTF_Font* font, float& outHeight)
{
    outHeight = 0.0f;
    if (text.empty() || !font) return;

    constexpr float lineH = 14.0f;
    std::string currentLine;
    float       currentY = y;

    auto flushLine = [&](const std::string& line)
    {
        if (line.empty()) return;
        drawText(line, x, currentY, color, font);
        currentY += lineH;
        outHeight += lineH;
    };

    std::istringstream iss{std::string(text)};
    std::string word;
    while (iss >> word)
    {
        std::string test;
        if (currentLine.empty()) {
            test = word;
        } else {
            test = currentLine;
            test.append(" ").append(word);
        }

        std::int32_t tw{}, th{};
        TTF_GetStringSize(font, test.c_str(), 0, &tw, &th);

        if (static_cast<float>(tw) > maxWidth && !currentLine.empty())
        {
            flushLine(currentLine);
            currentLine = word;
        }
        else {
            currentLine = std::move(test);
        }
    }
    flushLine(currentLine);
}

void Visualizer::drawMainMenu()
{
    drawText("Sort Visualizer", static_cast<float>(WIN_W) / 2.0f - 140.0f, 150.0f, {100, 200, 255, 255}, m_fontTitle.get());

    auto drawBtn = [this](const Button& btn) {
        SDL_FRect r{btn.x, btn.y, btn.w, btn.h};
        SDL_SetRenderDrawColor(m_renderer.get(), 40, 40, 60, 255);
        SDL_RenderFillRect(m_renderer.get(), &r);
        SDL_SetRenderDrawColor(m_renderer.get(), 100, 150, 255, 255);
        SDL_RenderRect(m_renderer.get(), &r);
        drawText(btn.label, btn.x + 35.0f, btn.y + 20.0f, {220, 220, 220, 255}, m_fontLarge.get());
    };

    drawBtn(m_btnMenuStart);
    drawBtn(m_btnMenuSettings);
    drawBtn(m_btnMenuQuit);
}

void Visualizer::drawSettings()
{
    drawText("Einstellungen", static_cast<float>(WIN_W) / 2.0f - 100.0f, 150.0f, {100, 200, 255, 255}, m_fontTitle.get());

    auto drawBtn = [this](const Button& btn) {
        SDL_FRect r{btn.x, btn.y, btn.w, btn.h};
        SDL_SetRenderDrawColor(m_renderer.get(), 40, 40, 60, 255);
        SDL_RenderFillRect(m_renderer.get(), &r);
        SDL_SetRenderDrawColor(m_renderer.get(), 100, 150, 255, 255);
        SDL_RenderRect(m_renderer.get(), &r);
        drawText(btn.label, btn.x + 40.0f, btn.y + 15.0f, {220, 220, 220, 255}, m_fontLarge.get());
    };

    drawBtn(m_btnSettingsFullscreen);
    drawBtn(m_btnSettingsBack);

    drawText(std::format("Lautstärke: {}%", static_cast<std::int32_t>(m_volume * 100.0f)), m_volumeSliderBg.x, m_volumeSliderBg.y - 30.0f, {200, 200, 200, 255}, m_fontLarge.get());

    SDL_SetRenderDrawColor(m_renderer.get(), 30, 30, 45, 255);
    SDL_RenderFillRect(m_renderer.get(), &m_volumeSliderBg);

    SDL_FRect thumb = {m_volumeSliderBg.x + m_volume * m_volumeSliderBg.w - 10.0f, m_volumeSliderBg.y - 10.0f, 20.0f, m_volumeSliderBg.h + 20.0f};
    SDL_SetRenderDrawColor(m_renderer.get(), 100, 200, 100, 255);
    SDL_RenderFillRect(m_renderer.get(), &thumb);
}

void Visualizer::drawBarsView()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_array.empty() || m_history.empty()) return;

    auto maxVal = static_cast<float>(*std::ranges::max_element(m_array));
    auto barW   = VIS_W / static_cast<float>(m_array.size());
    constexpr float fixedTopPadding = 50.0f;
    float effectiveVisHeight = VIS_H - fixedTopPadding;

    auto isFinished = (m_threadFinished && m_history.size() > 1 && m_historyIndex == static_cast<std::int32_t>(m_history.size()) - 1);

    std::vector<bool> touched(m_array.size(), false);

    if (m_historyIndex > 0 && m_historyIndex < static_cast<std::int32_t>(m_history.size())) {
        for (auto s = 1; s <= m_historyIndex; ++s) {
            auto a = m_history[static_cast<std::size_t>(s)].indexA;
            auto b = m_history[static_cast<std::size_t>(s)].indexB;
            if (a >= 0 && a < static_cast<std::int32_t>(touched.size())) touched[static_cast<std::size_t>(a)] = true;
            if (b >= 0 && b < static_cast<std::int32_t>(touched.size())) touched[static_cast<std::size_t>(b)] = true;

            const auto& prevArr = m_history[static_cast<std::size_t>(s) - 1].array;
            const auto& currArr = m_history[static_cast<std::size_t>(s)].array;
            auto minLen = std::min(prevArr.size(), currArr.size());
            for (auto j = 0U; j < minLen; ++j) {
                if (currArr[j] != prevArr[j] && j < touched.size()) {
                    touched[j] = true;
                }
            }
        }
    }

    for (auto i = 0; i < static_cast<std::int32_t>(m_array.size()); ++i)
    {
        float barH = (static_cast<float>(m_array[static_cast<std::size_t>(i)]) / maxVal) * effectiveVisHeight;
        float bx   = VIS_X + static_cast<float>(i) * barW;
        float by   = VIS_Y + VIS_H - barH;

        bool shouldBeGreen = isFinished;
        if (!shouldBeGreen && m_threadFinished && i < static_cast<std::int32_t>(m_finalStepForIndex.size())) {
            if (m_historyIndex > m_finalStepForIndex[static_cast<std::size_t>(i)] && touched[static_cast<std::size_t>(i)]) {
                shouldBeGreen = true;
            }
        }

        if (i == m_highlightA && !isFinished) {
            SDL_SetRenderDrawColor(m_renderer.get(), 255, 60, 60, 255);
        } else if (i == m_highlightB && !isFinished) {
            SDL_SetRenderDrawColor(m_renderer.get(), 255, 220, 50, 255);
        } else if (shouldBeGreen) {
            if (i % 2 == 0) SDL_SetRenderDrawColor(m_renderer.get(), 40, 190, 75, 255);
            else            SDL_SetRenderDrawColor(m_renderer.get(), 50, 210, 85, 255);
        } else {
            if (i % 2 == 0) SDL_SetRenderDrawColor(m_renderer.get(), 45, 90, 170, 255);
            else            SDL_SetRenderDrawColor(m_renderer.get(), 60, 110, 200, 255);
        }

        SDL_FRect barRect{bx, by, barW, barH};
        SDL_RenderFillRect(m_renderer.get(), &barRect);

        if (barW > 28.0f) {
            SDL_Color textColor = (shouldBeGreen) ? SDL_Color{200, 255, 200, 255} : (i == m_highlightA || i == m_highlightB) ? SDL_Color{255, 255, 255, 255} : SDL_Color{150, 150, 160, 255};
            drawText(std::format("{}", m_array[static_cast<std::size_t>(i)]), bx + 2.0f, by - 15.0f, textColor, m_fontTiny.get());
        }
    }
}

void Visualizer::drawNumbersView()
{
    constexpr float lineH = 26.0f;
    constexpr float numW  = 40.0f;
    constexpr float startArrayX = VIS_X + 45.0f;

    std::int32_t visibleHistSize = 0;
    std::int32_t fullHistSize = 0;
    std::int32_t currentArraySize = 0;

    std::vector<std::int32_t> sortedTarget;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_history.empty()) return;
        fullHistSize = static_cast<std::int32_t>(m_history.size());
        visibleHistSize = std::clamp(m_historyIndex + 1, 1, fullHistSize);
        currentArraySize = static_cast<std::int32_t>(m_array.size());
        sortedTarget = m_history.front().array;
    }

    std::ranges::sort(sortedTarget);

    constexpr auto maxLines = static_cast<std::int32_t>(VIS_H / lineH);
    auto maxScrollY = std::max(0, visibleHistSize - maxLines);

    if (m_autoScrollNumbers) {
        if (m_liveMode && m_sorting) {
            m_numbersScrollY = maxScrollY;
        } else if (m_historyIndex >= 0) {
            m_numbersScrollY = std::clamp(m_historyIndex - maxLines / 2, 0, maxScrollY);
        }
    }
    m_numbersScrollY = std::clamp(m_numbersScrollY, 0, maxScrollY);

    float maxItemWidth = static_cast<float>(currentArraySize) * numW + 100.0f;
    float maxScrollX = std::max(0.0f, maxItemWidth - VIS_W);
    m_numbersScrollX = std::clamp(m_numbersScrollX, 0.0f, maxScrollX);

    auto startIdx = m_numbersScrollY;
    float offsetX = -m_numbersScrollX;

    SDL_Rect clipRect = { static_cast<std::int32_t>(VIS_X), static_cast<std::int32_t>(VIS_Y), static_cast<std::int32_t>(VIS_W), static_cast<std::int32_t>(VIS_H) };
    SDL_SetRenderClipRect(m_renderer.get(), &clipRect);

    std::vector<bool> touched(static_cast<std::size_t>(currentArraySize), false);
    for (auto s = 1; s <= startIdx && s < visibleHistSize; ++s) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto a = m_history[static_cast<std::size_t>(s)].indexA;
        auto b = m_history[static_cast<std::size_t>(s)].indexB;
        if (a >= 0 && a < static_cast<std::int32_t>(touched.size())) touched[static_cast<std::size_t>(a)] = true;
        if (b >= 0 && b < static_cast<std::int32_t>(touched.size())) touched[static_cast<std::size_t>(b)] = true;
        auto minLen = std::min(m_history[static_cast<std::size_t>(s) - 1].array.size(), m_history[static_cast<std::size_t>(s)].array.size());
        for (auto j = 0U; j < minLen; ++j) {
            if (m_history[static_cast<std::size_t>(s)].array[j] != m_history[static_cast<std::size_t>(s) - 1].array[j] && j < touched.size()) touched[j] = true;
        }
    }

    for (auto s = startIdx; s < std::min(visibleHistSize, startIdx + maxLines); ++s)
    {
        SortStep step;
        { std::lock_guard<std::mutex> lock(m_mutex); step = m_history[static_cast<std::size_t>(s)]; }

        const auto& [array, indexA, indexB] = step;
        const float fy = VIS_Y + static_cast<float>(s - startIdx) * lineH;
        const auto  n  = static_cast<std::int32_t>(array.size());
        const bool  isCur = (s == m_historyIndex);
        const bool  lineIsFinished = (m_threadFinished && fullHistSize > 1 && s == fullHistSize - 1);

        if (isCur) {
            SDL_SetRenderDrawColor(m_renderer.get(), 40, 40, 70, 255);
            SDL_FRect curRect{VIS_X, fy, VIS_W - 10.0f, lineH};
            SDL_RenderFillRect(m_renderer.get(), &curRect);
        }

        drawText(std::format("{}:", s + 1), VIS_X, fy + 4.0f, isCur ? SDL_Color{255, 220, 60, 255} : SDL_Color{130, 130, 130, 255}, m_fontTiny.get());

        for (auto i = 0; i < n; ++i)
        {
            float fx = startArrayX + static_cast<float>(i) * numW + offsetX;
            if (fx > VIS_X + VIS_W) break;
            if (fx + numW < startArrayX - 10.0f) continue;
            if (!lineIsFinished && (i == indexA || i == indexB)) {
                SDL_SetRenderDrawColor(m_renderer.get(), i == indexA ? 200 : 180, i == indexA ? 60 : 180, i == indexA ? 60 : 40, 255);
                SDL_FRect hRect{fx - 2.0f, fy + 2.0f, numW - 2.0f, lineH - 4.0f};
                SDL_RenderFillRect(m_renderer.get(), &hRect);
            }
            bool shouldBeGreen = lineIsFinished || (m_threadFinished && s > m_finalStepForIndex[static_cast<std::size_t>(i)] && touched[static_cast<std::size_t>(i)]);
            SDL_Color textColor = shouldBeGreen ? SDL_Color{100, 255, 100, 255} : (!lineIsFinished && (i == indexA || i == indexB)) ? SDL_Color{255, 255, 255, 255} : SDL_Color{130, 130, 140, 255};
            drawText(std::format("{}", array[static_cast<std::size_t>(i)]), fx, fy + 4.0f, textColor, m_fontTiny.get());
        }
    }
    SDL_SetRenderClipRect(m_renderer.get(), nullptr);
}

void Visualizer::drawButtons()
{
    auto drawBtn = [this](const Button& btn, bool enabled = true) {
        SDL_FRect r{btn.x, btn.y, btn.w, btn.h};
        if (!enabled) SDL_SetRenderDrawColor(m_renderer.get(), 35, 35, 45, 255);
        else if (btn.active) SDL_SetRenderDrawColor(m_renderer.get(), 20, 150, 80, 255);
        else SDL_SetRenderDrawColor(m_renderer.get(), 50, 50, 70, 255);
        SDL_RenderFillRect(m_renderer.get(), &r);
        SDL_SetRenderDrawColor(m_renderer.get(), enabled ? 160 : 70, enabled ? 160 : 70, enabled ? 180 : 80, 255);
        SDL_RenderRect(m_renderer.get(), &r);
        drawText(btn.label, btn.x + 8.0f, btn.y + 12.0f, enabled ? SDL_Color{220, 220, 220, 255} : SDL_Color{90, 90, 100, 255}, m_fontLarge.get());
    };

    std::ranges::for_each(m_algoButtons, [&](const Button& b){ drawBtn(b, !m_sorting); });
    drawBtn(m_startButton, !m_sorting);

    {
        SDL_FRect r{m_stopButton.x, m_stopButton.y, m_stopButton.w, m_stopButton.h};
        auto isPaused = m_sorting && !m_liveMode;
        if (!m_sorting) SDL_SetRenderDrawColor(m_renderer.get(), 35, 35, 45, 255);
        else if (isPaused) SDL_SetRenderDrawColor(m_renderer.get(), 20, 150, 80, 255);
        else SDL_SetRenderDrawColor(m_renderer.get(), 200, 120, 30, 255);
        SDL_RenderFillRect(m_renderer.get(), &r);
        SDL_SetRenderDrawColor(m_renderer.get(), m_sorting ? 160 : 70, m_sorting ? 160 : 70, m_sorting ? 180 : 80, 255);
        SDL_RenderRect(m_renderer.get(), &r);
        drawText(isPaused ? "Weiter" : "Pause", m_stopButton.x + 20.0f, m_stopButton.y + 12.0f,
                 m_sorting ? SDL_Color{255, 255, 255, 255} : SDL_Color{90, 90, 100, 255}, m_fontLarge.get());
    }

    drawBtn(m_cancelButton, (m_sorting || m_historyIndex > 0));
    drawBtn(m_stepFwdButton, (!m_liveMode && (m_historyIndex < static_cast<std::int32_t>(m_history.size()) - 1 || (!m_sorting && m_historyIndex == 0))));
    drawBtn(m_stepBackButton, (m_historyIndex > 0 && !m_liveMode));
    drawBtn(m_randomButton, !m_sorting);
    drawBtn(m_viewBarsButton);
    drawBtn(m_viewNumsButton);
    drawBtn(m_btnBackToMenu, !m_sorting);
    drawBtn(m_btnBenchmark, true);

    SDL_SetRenderDrawColor(m_renderer.get(), 30, 30, 45, 255);
    SDL_RenderFillRect(m_renderer.get(), &m_speedSliderBg);
    float sThumbX = m_speedSliderBg.x + (static_cast<float>(m_delayMs) / 1000.0f) * m_speedSliderBg.w;
    SDL_FRect sThumb = {sThumbX - 5.0f, m_speedSliderBg.y - 5.0f, 10.0f, m_speedSliderBg.h + 10.0f};
    SDL_SetRenderDrawColor(m_renderer.get(), 100, 140, 200, 255);
    SDL_RenderFillRect(m_renderer.get(), &sThumb);
    drawText(std::format("{} ms", m_delayMs), m_speedSliderBg.x, m_speedSliderBg.y - 25.0f, {200, 200, 200, 255}, m_fontLarge.get());

    SDL_SetRenderDrawColor(m_renderer.get(), 30, 30, 45, 255);
    SDL_RenderFillRect(m_renderer.get(), &m_sizeSliderBg);
    float nRatio = static_cast<float>(m_arraySize - 5) / 495.0f;
    float nThumbX = m_sizeSliderBg.x + nRatio * m_sizeSliderBg.w;
    SDL_FRect nThumb = {nThumbX - 5.0f, m_sizeSliderBg.y - 5.0f, 10.0f, m_sizeSliderBg.h + 10.0f};
    SDL_SetRenderDrawColor(m_renderer.get(), 200, 140, 100, 255);
    SDL_RenderFillRect(m_renderer.get(), &nThumb);
    drawText(std::format("n={}", m_arraySize), m_sizeSliderBg.x, m_sizeSliderBg.y - 25.0f, {200, 200, 200, 255}, m_fontLarge.get());
}

void Visualizer::draw()
{
    SDL_SetRenderDrawColor(m_renderer.get(), 22, 22, 32, 255);
    SDL_RenderClear(m_renderer.get());

    if (m_appState == AppState::MainMenu) {
        drawMainMenu();
    } else if (m_appState == AppState::Settings) {
        drawSettings();
    } else if (m_appState == AppState::Visualizer) {
        SDL_SetRenderDrawColor(m_renderer.get(), 70, 70, 100, 255);
        SDL_RenderLine(m_renderer.get(), 0, WIN_H - static_cast<std::int32_t>(UI_H), WIN_W, WIN_H - static_cast<std::int32_t>(UI_H));

        if (m_viewMode == ViewMode::Bars) {
            drawBarsView();
        } else {
            drawNumbersView();
        }
        drawButtons();
    }
    SDL_RenderPresent(m_renderer.get());
}